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

bool Image::saveToFile(const std::string& filePath) const {
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
	//Заменить на ручную реализацию
	cv::UMat blurred;
	cv::GaussianBlur(currentImage, blurred, cv::Size(kernelSize, kernelSize), sigma);
	currentImage = blurred;
}

void Image::applyMirror(bool horizontal) {
	if (currentImage.empty()) { return; }
	pushState();
	// Заменить на ручную реализацию
	cv::UMat flipped;
	cv::flip(currentImage, flipped, horizontal ? 1 : 0);
	currentImage = flipped;
}

void Image::rotate(bool clockwise) {
	if (currentImage.empty()) { return; }
	pushState();
	// Заменить на ручную реализацию
	cv::UMat rotated;
	cv::rotate(currentImage, rotated, clockwise ? cv::ROTATE_90_CLOCKWISE : cv::ROTATE_90_COUNTERCLOCKWISE);
	currentImage = rotated;
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