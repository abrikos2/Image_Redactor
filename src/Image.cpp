#include "Image.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
#include <thread>
#include <vector>

Image::Image() {
	if (cv::ocl::haveOpenCL()) {
		cv::ocl::setUseOpenCL(true);
	}
}

void Image::setUseGPU(bool use) {
	useGPU = use && cv::ocl::haveOpenCL();
}

void Image::setUseMultithreading(bool use) {
	useMultithreading = use;
}

bool Image::loadFromFile(const std::string& filePath) {
	cv::Mat cpuImg = cv::imread(filePath, cv::IMREAD_COLOR);
	if (cpuImg.empty()) { return false; }
	clearHistory();
	cpuImg.copyTo(currentImage);
	pushState();
	return true;
}

bool Image::SaveToFile(const std::string& filePath) const {
	if (currentImage.empty()) { return false; }
	cv::Mat cpu;
	currentImage.copyTo(cpu);
	return cv::imwrite(filePath, cpu);
}

cv::UMat Image::getCurrent() const {
	return currentImage.clone();
}

void Image::applyGausBlur(int kernelSize, double sigma) {
	if (currentImage.empty()) { return; }
	pushState();

	if (useGPU) {
		// Используем встроенную функцию OpenCV, которая автоматически задействует OpenCL через UMat
		cv::UMat dst;
		cv::GaussianBlur(currentImage, dst, cv::Size(kernelSize, kernelSize), sigma);
		currentImage = dst;
		return;
	}

	cv::Mat cpu;
	currentImage.copyTo(cpu);
	cv::Mat temp = cpu.clone();
	cv::Mat dst = cpu.clone();

	int radius = kernelSize / 2;
	int channels = cpu.channels();

	std::vector<double> kernel1D(kernelSize);
	double sum = 0.0;
	double sigma2 = 2 * sigma * sigma;

	for (int i = -radius; i <= radius; i++) {
		double value = exp(-(i * i) / sigma2) / sqrt(CV_PI * sigma2);
		kernel1D[i + radius] = value;
		sum += value;
	}

	for (int i = 0; i < kernelSize; i++) {
		kernel1D[i] /= sum;
	}

	auto processHorizontal = [&](int startY, int endY) {
		for (int y = startY; y < endY; y++) {
			for (int x = radius; x < cpu.cols - radius; x++) {
				for (int c = 0; c < channels; c++) {
					double value = 0.0;
					for (int k = -radius; k <= radius; k++) {
						value += kernel1D[k + radius] *
							cpu.at<cv::Vec3b>(y, x + k)[c];
					}
					temp.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(value);
				}
			}
		}
	};

	auto processVertical = [&](int startY, int endY) {
		for (int y = startY; y < endY; y++) {
			for (int x = 0; x < cpu.cols; x++) {
				for (int c = 0; c < channels; c++) {
					double value = 0.0;
					for (int k = -radius; k <= radius; k++) {
						value += kernel1D[k + radius] *
							temp.at<cv::Vec3b>(y + k, x)[c];
					}
					dst.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(value);
				}
			}
		}
	};

	if (useMultithreading) {
		int numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) numThreads = 4;
		
		std::vector<std::thread> threads;
		int rowsPerThread = cpu.rows / numThreads;
		
		// Горизонтальный проход
		for (int i = 0; i < numThreads; i++) {
			int startY = i * rowsPerThread;
			int endY = (i == numThreads - 1) ? cpu.rows : (i + 1) * rowsPerThread;
			threads.emplace_back(processHorizontal, startY, endY);
		}
		for (auto& t : threads) t.join();
		threads.clear();

		// Вертикальный проход
		for (int i = 0; i < numThreads; i++) {
			int startY = std::max(radius, i * rowsPerThread);
			int endY = (i == numThreads - 1) ? cpu.rows - radius : std::max(radius, (i + 1) * rowsPerThread);
			if (startY < endY) {
				threads.emplace_back(processVertical, startY, endY);
			}
		}
		for (auto& t : threads) t.join();
	} else {
		processHorizontal(0, cpu.rows);
		processVertical(radius, cpu.rows - radius);
	}

	dst.copyTo(currentImage);
}

void Image::applyBilateralFilter(int d, double sigmaColor, double sigmaSpace) {
	if (currentImage.empty()) { return; }
	pushState();

	if (useGPU) {
		cv::UMat dst;
		cv::bilateralFilter(currentImage, dst, d, sigmaColor, sigmaSpace);
		currentImage = dst;
		return;
	}

	// Ручная реализация двустороннего фильтра на CPU
	cv::Mat cpu;
	currentImage.copyTo(cpu);
	cv::Mat dst = cpu.clone();

	int radius = d / 2;
	int channels = cpu.channels();

	auto processBilateral = [&](int startY, int endY) {
		for (int y = startY; y < endY; y++) {
			for (int x = radius; x < cpu.cols - radius; x++) {
				cv::Vec3b centerPixel = cpu.at<cv::Vec3b>(y, x);
				
				for (int c = 0; c < channels; c++) {
					double sumWeight = 0.0;
					double sumValue = 0.0;

					for (int ky = -radius; ky <= radius; ky++) {
						for (int kx = -radius; kx <= radius; kx++) {
							cv::Vec3b neighborPixel = cpu.at<cv::Vec3b>(y + ky, x + kx);
							
							// Пространственное расстояние
							double spaceDist2 = ky * ky + kx * kx;
							double spaceWeight = exp(-spaceDist2 / (2 * sigmaSpace * sigmaSpace));
							
							// Цветовое расстояние
							double colorDist = std::abs(neighborPixel[c] - centerPixel[c]);
							double colorWeight = exp(-(colorDist * colorDist) / (2 * sigmaColor * sigmaColor));
							
							double weight = spaceWeight * colorWeight;
							sumWeight += weight;
							sumValue += weight * neighborPixel[c];
						}
					}
					dst.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(sumValue / sumWeight);
				}
			}
		}
	};

	if (useMultithreading) {
		int numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) numThreads = 4;
		
		std::vector<std::thread> threads;
		int rowsPerThread = (cpu.rows - 2 * radius) / numThreads;
		
		for (int i = 0; i < numThreads; i++) {
			int startY = radius + i * rowsPerThread;
			int endY = (i == numThreads - 1) ? cpu.rows - radius : radius + (i + 1) * rowsPerThread;
			if (startY < endY) {
				threads.emplace_back(processBilateral, startY, endY);
			}
		}
		for (auto& t : threads) t.join();
	} else {
		processBilateral(radius, cpu.rows - radius);
	}

	dst.copyTo(currentImage);
}

void Image::applyMirror(bool horizontal) {
	if (currentImage.empty()) { return; }
	pushState();

	cv::Mat cpu;
	currentImage.copyTo(cpu);
	cv::Mat dst = cv::Mat::zeros(cpu.size(), cpu.type());

	int rows = cpu.rows;
	int cols = cpu.cols;
	int channels = cpu.channels();

	if (horizontal) {
		for (int y = 0; y < rows; y++) {
			uchar* cpuRow = cpu.ptr<uchar>(y);
			uchar* dstRow = dst.ptr<uchar>(y);

			for (int x = 0; x < cols; x++) {
				int cpuIdx = x * channels;
				int dstIdx = (cols - 1 - x) * channels;

				for (int c = 0; c < channels; c++) {
					dstRow[dstIdx + c] = cpuRow[cpuIdx + c];
				}
			}
		}
	}
	else {
		for (int y = 0; y < rows; y++) {
			uchar* cpuRow = cpu.ptr<uchar>(y);
			uchar* dstRow = dst.ptr<uchar>(rows - 1 - y);

			for (int x = 0; x < cols; x++) {
				int idx = x * channels;
				for (int c = 0; c < channels; c++) {
					dstRow[idx + c] = cpuRow[idx + c];
				}
			}
		}
	}

	dst.copyTo(currentImage);
}

void Image::rotate(bool clockwise) {
	if (currentImage.empty()) { return; }
	pushState();

	cv::Mat cpu;
	currentImage.copyTo(cpu);
	int rows = cpu.rows;
	int cols = cpu.cols;
	int channels = cpu.channels();

	cv::Mat dst = cv::Mat::zeros(cv::Size(rows, cols), cpu.type());

	if (clockwise) {
		for (int y = 0; y < rows; y++) {
			const uchar* cpuRow = cpu.ptr<uchar>(y);
			for (int x = 0; x < cols; x++) {
				uchar* dstPixel = dst.ptr<uchar>(x) + (rows - 1 - y) * channels;
				const uchar* cpuPixel = cpuRow + x * channels;
				for (int c = 0; c < channels; c++) {
					dstPixel[c] = cpuPixel[c];
				}
			}
		}
	}
	else {
		for (int y = 0; y < rows; y++) {
			const uchar* cpuRow = cpu.ptr<uchar>(y);
			for (int x = 0; x < cols; x++) {
				uchar* dstPixel = dst.ptr<uchar>(cols - 1 - x) + y * channels;
				const uchar* cpuPixel = cpuRow + x * channels;
				for (int c = 0; c < channels; c++) {
					dstPixel[c] = cpuPixel[c];
				}
			}
		}
	}

	dst.copyTo(currentImage);
}


void Image::undo() {
	if (undoStack.empty()) { return; }
	cv::Mat currentCPU;
	currentImage.copyTo(currentCPU);
	redoStack.push_back(currentCPU);
	restoreState(undoStack.back());
	undoStack.pop_back();
}

void Image::redo() {
	if (redoStack.empty()) { return; }
	cv::Mat currentCpu;
	currentImage.copyTo(currentCpu);
	undoStack.push_back(currentCpu);
	restoreState(redoStack.back());
	redoStack.pop_back();
}

bool Image::canUndo() const { return !undoStack.empty(); }
bool Image::canRedo() const { return !redoStack.empty(); }

void Image::clearHistory() {
	undoStack.clear();
	redoStack.clear();
}

void Image::pushState() {
	if (currentImage.empty()) { return; }
	cv::Mat state;
	currentImage.copyTo(state);
	undoStack.push_back(state);
	if (undoStack.size() > masxHistorySize) {
		undoStack.erase(undoStack.begin());
	}
	redoStack.clear();
}

void Image::restoreState(const cv::Mat& state) {
	state.copyTo(currentImage);
}