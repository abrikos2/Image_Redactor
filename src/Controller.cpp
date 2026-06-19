#include "Controller.h"

Controller::Controller() : ImageModel(std::make_unique<Image>()) {}

bool Controller::openImage(const std::string& filePath) {
	return ImageModel->loadFromFile(filePath);
}

bool Controller::SaveImage(const std::string& filePath) const {
	return ImageModel->SaveToFile(filePath);
}

cv::UMat Controller::getCurrent() const {
	return ImageModel->getCurrent();
}

void Controller::BlurImage(int kernelSize, double sigma) {
	ImageModel->applyGausBlur(kernelSize, sigma);
}

void Controller::MirrorImage(bool horizontal) {
	ImageModel->applyMirror(horizontal);
}

void Controller::rotateImage(bool clockwise) {
	ImageModel->rotate(clockwise);
}

void Controller::drawLine(int x1, int y1, int x2, int y2,
	                      int r, int g, int b, int thickness) {
	cv::Point from(x1, y1), to(x2, y2);
	cv::Scalar color(b, g, r);
	ImageModel->drawOnImage(from, to,color, thickness);
}

void Controller::undo() {
	ImageModel->undo();
}

void Controller::redo() {
	ImageModel->redo();
}

bool Controller::canUndo() const {
	return ImageModel->canUndo();
}

bool Controller::canRedo() const {
	return ImageModel->canRedo();
}
