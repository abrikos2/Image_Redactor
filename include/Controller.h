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

	void BlurImage(int kernelSize, double sigma);
	void MirrorImage(bool horizontal);
	void rotateImage(bool clockwise);
	void drawLine(int x1, int y1, int x2, int y2,
				  int r, int g, int b, int thickness);

	void undo();
	void redo();
	bool canUndo() const;
	bool canRedo() const;
private:
	std::unique_ptr<Image> ImageModel;
};