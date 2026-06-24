#include "Controller.h"
#include <filesystem>
#include <thread>
#include <vector>
#include <iostream>
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

void Controller::setUseGPU(bool use) {
	ImageModel->setUseGPU(use);
}

void Controller::setUseMultithreading(bool use) {
	ImageModel->setUseMultithreading(use);
}

bool Controller::isUsingGPU() const {
	return ImageModel->isUsingGPU();
}

bool Controller::isUsingMultithreading() const {
	return ImageModel->isUsingMultithreading();
}

void Controller::BlurImage(int kernelSize, double sigma) {
	ImageModel->applyGausBlur(kernelSize, sigma);
}

void Controller::BilateralFilter(int d, double sigmaColor, double sigmaSpace) {
	ImageModel->applyBilateralFilter(d, sigmaColor, sigmaSpace);
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

void Controller::processFolder(const std::string& inputDir, const std::string& outputDir, 
					   int blurKernelSize, double blurSigma,
					   int bilateralD, double bilateralSigmaColor, double bilateralSigmaSpace,
					   bool useBlur, bool useBilateral) {
	namespace fs = std::filesystem;
	
	if (!fs::exists(inputDir)) return;
	if (!fs::exists(outputDir)) {
		fs::create_directories(outputDir);
	}

	std::vector<std::string> files;
	for (const auto& entry : fs::directory_iterator(inputDir)) {
		if (entry.is_regular_file()) {
			std::string ext = entry.path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp") {
				files.push_back(entry.path().string());
			}
		}
	}

	auto processImage = [&](const std::string& filePath) {
		Image img;
		img.setUseGPU(isUsingGPU());
		img.setUseMultithreading(false); // Внутри потока не используем многопоточность для одного изображения
		
		if (img.loadFromFile(filePath)) {
			if (useBlur) {
				img.applyGausBlur(blurKernelSize, blurSigma);
			}
			if (useBilateral) {
				img.applyBilateralFilter(bilateralD, bilateralSigmaColor, bilateralSigmaSpace);
			}
			
			fs::path p(filePath);
			std::string outPath = (fs::path(outputDir) / p.filename()).string();
			img.SaveToFile(outPath);
		}
	};

	if (isUsingMultithreading()) {
		int numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) numThreads = 4;
		
		std::vector<std::thread> threads;
		for (size_t i = 0; i < files.size(); ++i) {
			threads.emplace_back(processImage, files[i]);
			if (threads.size() >= static_cast<size_t>(numThreads)) {
				for (auto& t : threads) t.join();
				threads.clear();
			}
		}
		for (auto& t : threads) t.join();
	} else {
		for (const auto& file : files) {
			processImage(file);
		}
	}
}
