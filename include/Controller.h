#pragma once
#include "Image.h"
#include <memory>

class Controller {
public :
	Controller();
	~Controller() = default;

	bool openImage(const std::string& filePath);
	bool SaveImage(const std::string& filePath) const;

	cv::UMat getCurrent() const;

	void setUseGPU(bool use);
	void setUseMultithreading(bool use);
	bool isUsingGPU() const;
	bool isUsingMultithreading() const;

	void BlurImage(int kernelSize, double sigma);
	void BilateralFilter(int d, double sigmaColor, double sigmaSpace);
	void MirrorImage(bool horizontal);
	void rotateImage(bool clockwise);

	void undo();
	void redo();
	bool canUndo() const;
	bool canRedo() const;

	void processFolder(const std::string& inputDir, const std::string& outputDir, 
					   int blurKernelSize, double blurSigma,
					   int bilateralD, double bilateralSigmaColor, double bilateralSigmaSpace,
					   bool useBlur, bool useBilateral);

private:
	std::unique_ptr<Image> ImageModel;
};