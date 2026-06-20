#include "Image.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>

Image::Image() {
	if (cv::ocl::haveOpenCL()) {
		cv::ocl::setUseOpenCL(true);
	}
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

	for (int y = 0; y < cpu.rows; y++) {
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

	for (int y = radius; y < cpu.rows - radius; y++) {
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
	int rowSize = cols * channels;

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

void Image::drawOnImage(const cv::Point& from, const cv::Point& to,
						const cv::Scalar& color, int thickness) {
	if (currentImage.empty()) { return; }
	pushState();
	cv::Mat cpu;
	currentImage.copyTo(cpu);
	cv::line(cpu, from, to, color, thickness);
	cpu.copyTo(currentImage);
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