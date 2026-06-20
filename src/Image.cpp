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
	//imwrite принимает только Mat 
	//UMat ему не подходит так что преобразование обязательно
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

	cv::Mat src = currentImage.getMat(cv::ACCESS_READ);
	cv::Mat temp = src.clone();
	cv::Mat dst = src.clone();

	int radius = kernelSize / 2;
	int channels = src.channels();

	// ядро Гаусса
	std::vector<double> kernel1D(kernelSize);
	double sum = 0.0;
	double sigma2 = 2 * sigma * sigma;

	for (int i = -radius; i <= radius; i++) {
		double value = exp(-(i * i) / sigma2) / sqrt(CV_PI * sigma2);
		kernel1D[i + radius] = value;
		sum += value;
	}

	//Нормализация
	for (int i = 0; i < kernelSize; i++) {
		kernel1D[i] /= sum;
	}

	// Свертка по горизонтали
	for (int y = 0; y < src.rows; y++) {
		for (int x = radius; x < src.cols - radius; x++) {
			for (int c = 0; c < channels; c++) {
				double value = 0.0;
				for (int k = -radius; k <= radius; k++) {
					value += kernel1D[k + radius] *
						src.at<cv::Vec3b>(y, x + k)[c];
				}
				temp.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(value);
			}
		}
	}

	//Свертка по вертикали
	for (int y = radius; y < src.rows - radius; y++) {
		for (int x = 0; x < src.cols; x++) {
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

	cv::Mat src = currentImage.getMat(cv::ACCESS_READ);
	cv::Mat dst = cv::Mat::zeros(src.size(), src.type());

	int rows = src.rows;
	int cols = src.cols;
	int channels = src.channels();

	if (horizontal) {
		// Горизонтальное отражение
		for (int y = 0; y < rows; y++) {
			uchar* srcRow = src.ptr<uchar>(y);
			uchar* dstRow = dst.ptr<uchar>(y);

			for (int x = 0; x < cols; x++) {
				int srcIdx = x * channels;
				int dstIdx = (cols - 1 - x) * channels;

				for (int c = 0; c < channels; c++) {
					dstRow[dstIdx + c] = srcRow[srcIdx + c];
				}
			}
		}
	}
	else {
		// Вертикальное отражение
		for (int y = 0; y < rows; y++) {
			uchar* srcRow = src.ptr<uchar>(y);
			uchar* dstRow = dst.ptr<uchar>(rows - 1 - y);

			for (int x = 0; x < cols; x++) {
				int idx = x * channels;
				for (int c = 0; c < channels; c++) {
					dstRow[idx + c] = srcRow[idx + c];
				}
			}
		}
	}

	dst.copyTo(currentImage);
}

void Image::rotate(bool clockwise) {
	if (currentImage.empty()) { return; }
	pushState();

	cv::Mat src = currentImage.getMat(cv::ACCESS_READ);
	int rows = src.rows;
	int cols = src.cols;
	int channels = src.channels();
	int rowSize = cols * channels;

	// При повороте на 90 градусов размеры меняются местами
	cv::Mat dst = cv::Mat::zeros(cv::Size(rows, cols), src.type());

	if (clockwise) {
		// Поворот по часовой стрелке
		// Каждая строка исходного изображения становится столбцом в новом
		for (int y = 0; y < rows; y++) {
			const uchar* srcRow = src.ptr<uchar>(y);
			for (int x = 0; x < cols; x++) {
				uchar* dstPixel = dst.ptr<uchar>(x) + (rows - 1 - y) * channels;
				const uchar* srcPixel = srcRow + x * channels;
				// Копируем пиксель (все каналы)
				for (int c = 0; c < channels; c++) {
					dstPixel[c] = srcPixel[c];
				}
			}
		}
	}
	else {
		// Поворот против часовой стрелки
		for (int y = 0; y < rows; y++) {
			const uchar* srcRow = src.ptr<uchar>(y);
			for (int x = 0; x < cols; x++) {
				uchar* dstPixel = dst.ptr<uchar>(cols - 1 - x) + y * channels;
				const uchar* srcPixel = srcRow + x * channels;
				for (int c = 0; c < channels; c++) {
					dstPixel[c] = srcPixel[c];
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