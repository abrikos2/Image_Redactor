#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

class Image {
public:
	Image();
	~Image() = default;
	bool loadFromFile(const std::string& filePath);
	bool SaveToFile(const std::string& filePath) const;

	cv::UMat getCurrent() const;

	void setUseGPU(bool use);
	void setUseMultithreading(bool use);
	bool isUsingGPU() const { return useGPU; }
	bool isUsingMultithreading() const { return useMultithreading; }

	void applyGausBlur(int kernelSize, double sigma);
	void applyBilateralFilter(int d, double sigmaColor, double sigmaSpace);
	void applyMirror(bool horizontal);
	void rotate(bool clockwise);
	void drawOnImage(const cv::Point& from, const cv::Point& to,
		             const cv::Scalar& color, int thickness);

	void undo();
	void redo();
	bool canUndo() const;
	bool canRedo() const;
	void clearHistory();

private:
	cv::UMat currentImage;
	std::vector<cv::Mat> undoStack;
	std::vector<cv::Mat> redoStack;
	size_t masxHistorySize = 20;

	bool useGPU = false;
	bool useMultithreading = false;

	void pushState();
	void restoreState(const cv::Mat& state);
};