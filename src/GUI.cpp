#include "GUI.h"
#include <QDir>
#include <QMessageBox>
#include <QThread>

GUI::GUI(QWidget* parent) : QWidget(parent) {
    buildUI();
    
    // Connect signals
    connect(&btnOpen_, &QPushButton::clicked, this, &GUI::openImage);
    connect(&btnSave_, &QPushButton::clicked, this, &GUI::saveImage);
    connect(&btnProcessFolder_, &QPushButton::clicked, this, &GUI::processFolder);
    connect(&btnUndo_, &QPushButton::clicked, this, &GUI::undo);
    connect(&btnRedo_, &QPushButton::clicked, this, &GUI::redo);
    
    connect(&btnBlur_, &QPushButton::clicked, this, &GUI::applyGaussianBlur);
    connect(&btnBilateral_, &QPushButton::clicked, this, &GUI::applyBilateralFilter);
    
    connect(&btnMirrorH_, &QPushButton::clicked, this, &GUI::applyMirrorHorizontal);
    connect(&btnMirrorV_, &QPushButton::clicked, this, &GUI::applyMirrorVertical);
    connect(&btnRotateCW_, &QPushButton::clicked, this, &GUI::rotateClockwise);
    connect(&btnRotateCCW_, &QPushButton::clicked, this, &GUI::rotateCounterClockwise);
    connect(&btnDrawLine_, &QPushButton::clicked, this, &GUI::drawLine);
    
    connect(&cbUseGPU_, &QCheckBox::stateChanged, this, &GUI::toggleGPU);
    connect(&cbUseMultithreading_, &QCheckBox::stateChanged, this, &GUI::toggleMultithreading);
}

void GUI::buildUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Top controls
    QHBoxLayout* topLayout = new QHBoxLayout();
    btnOpen_.setText("Open Image");
    btnSave_.setText("Save Image");
    btnProcessFolder_.setText("Batch Process Folder");
    btnUndo_.setText("Undo");
    btnRedo_.setText("Redo");
    
    cbUseGPU_.setText("Use GPU (OpenCL)");
    cbUseMultithreading_.setText("Use Multithreading");
    
    topLayout->addWidget(&btnOpen_);
    topLayout->addWidget(&btnSave_);
    topLayout->addWidget(&btnProcessFolder_);
    topLayout->addWidget(&btnUndo_);
    topLayout->addWidget(&btnRedo_);
    topLayout->addWidget(&cbUseGPU_);
    topLayout->addWidget(&cbUseMultithreading_);
    
    mainLayout->addLayout(topLayout);
    
    // Image display
    QHBoxLayout* imageLayout = new QHBoxLayout();
    originalLabel_.setText("Original Image");
    originalLabel_.setAlignment(Qt::AlignCenter);
    originalLabel_.setMinimumSize(400, 400);
    originalLabel_.setStyleSheet("border: 1px solid black;");
    
    editedLabel_.setText("Edited Image");
    editedLabel_.setAlignment(Qt::AlignCenter);
    editedLabel_.setMinimumSize(400, 400);
    editedLabel_.setStyleSheet("border: 1px solid black;");
    
    imageLayout->addWidget(&originalLabel_);
    imageLayout->addWidget(&editedLabel_);
    mainLayout->addLayout(imageLayout);
    
    // Filters
    QHBoxLayout* filtersLayout = new QHBoxLayout();
    
    // Gaussian Blur
    QGroupBox* blurGroup = new QGroupBox("Gaussian Blur");
    QVBoxLayout* blurLayout = new QVBoxLayout();
    blurKernelSize_.setRange(1, 99);
    blurKernelSize_.setSingleStep(2);
    blurKernelSize_.setValue(15);
    blurSigma_.setRange(0.1, 50.0);
    blurSigma_.setValue(5.0);
    btnBlur_.setText("Apply Blur");
    blurLayout->addWidget(new QLabel("Kernel Size:"));
    blurLayout->addWidget(&blurKernelSize_);
    blurLayout->addWidget(new QLabel("Sigma:"));
    blurLayout->addWidget(&blurSigma_);
    blurLayout->addWidget(&btnBlur_);
    blurGroup->setLayout(blurLayout);
    
    // Bilateral Filter
    QGroupBox* bilateralGroup = new QGroupBox("Bilateral Filter");
    QVBoxLayout* bilateralLayout = new QVBoxLayout();
    bilateralD_.setRange(1, 50);
    bilateralD_.setValue(9);
    bilateralSigmaColor_.setRange(1.0, 200.0);
    bilateralSigmaColor_.setValue(75.0);
    bilateralSigmaSpace_.setRange(1.0, 200.0);
    bilateralSigmaSpace_.setValue(75.0);
    btnBilateral_.setText("Apply Bilateral");
    bilateralLayout->addWidget(new QLabel("Diameter:"));
    bilateralLayout->addWidget(&bilateralD_);
    bilateralLayout->addWidget(new QLabel("Sigma Color:"));
    bilateralLayout->addWidget(&bilateralSigmaColor_);
    bilateralLayout->addWidget(new QLabel("Sigma Space:"));
    bilateralLayout->addWidget(&bilateralSigmaSpace_);
    bilateralLayout->addWidget(&btnBilateral_);
    bilateralGroup->setLayout(bilateralLayout);
    
    // Transforms
    QGroupBox* transformGroup = new QGroupBox("Transforms");
    QVBoxLayout* transformLayout = new QVBoxLayout();
    btnMirrorH_.setText("Mirror Horizontal");
    btnMirrorV_.setText("Mirror Vertical");
    btnRotateCW_.setText("Rotate CW");
    btnRotateCCW_.setText("Rotate CCW");
    transformLayout->addWidget(&btnMirrorH_);
    transformLayout->addWidget(&btnMirrorV_);
    transformLayout->addWidget(&btnRotateCW_);
    transformLayout->addWidget(&btnRotateCCW_);
    transformGroup->setLayout(transformLayout);
    
    filtersLayout->addWidget(blurGroup);
    filtersLayout->addWidget(bilateralGroup);
    filtersLayout->addWidget(transformGroup);
    
    mainLayout->addLayout(filtersLayout);
    
    // Progress bar
    progressBar_.setRange(0, 100);
    progressBar_.setValue(0);
    mainLayout->addWidget(&progressBar_);
}

void GUI::openImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        currentFilePath_ = fileName.toStdString();
        if (controller_.openImage(currentFilePath_)) {
            hasImage_ = true;
            updateDisplays();
        } else {
            QMessageBox::warning(this, "Error", "Failed to open image.");
        }
    }
}

void GUI::saveImage() {
    if (!hasImage_) return;
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        if (!controller_.SaveImage(fileName.toStdString())) {
            QMessageBox::warning(this, "Error", "Failed to save image.");
        }
    }
}

void GUI::processFolder() {
    QString inputDir = QFileDialog::getExistingDirectory(this, "Select Input Folder");
    if (inputDir.isEmpty()) return;
    
    QString outputDir = QFileDialog::getExistingDirectory(this, "Select Output Folder");
    if (outputDir.isEmpty()) return;
    
    progressBar_.setValue(50); // Simple progress indication
    QApplication::processEvents();
    
    controller_.processFolder(
        inputDir.toStdString(), 
        outputDir.toStdString(),
        blurKernelSize_.value(), blurSigma_.value(),
        bilateralD_.value(), bilateralSigmaColor_.value(), bilateralSigmaSpace_.value(),
        true, true // Apply both for batch processing as an example
    );
    
    progressBar_.setValue(100);
    QMessageBox::information(this, "Success", "Batch processing completed!");
    progressBar_.setValue(0);
}

void GUI::processSingleImage(const std::string& inputPath, const std::string& outputPath) {
    // Not used directly in UI, handled by Controller::processFolder
}

void GUI::applyGaussianBlur() {
    if (!hasImage_) return;
    int k = blurKernelSize_.value();
    if (k % 2 == 0) k++; // Kernel size must be odd
    controller_.BlurImage(k, blurSigma_.value());
    updateDisplays();
}

void GUI::applyBilateralFilter() {
    if (!hasImage_) return;
    controller_.BilateralFilter(bilateralD_.value(), bilateralSigmaColor_.value(), bilateralSigmaSpace_.value());
    updateDisplays();
}

void GUI::applyMirrorHorizontal() {
    if (!hasImage_) return;
    controller_.MirrorImage(true);
    updateDisplays();
}

void GUI::applyMirrorVertical() {
    if (!hasImage_) return;
    controller_.MirrorImage(false);
    updateDisplays();
}

void GUI::rotateClockwise() {
    if (!hasImage_) return;
    controller_.rotateImage(true);
    updateDisplays();
}

void GUI::rotateCounterClockwise() {
    if (!hasImage_) return;
    controller_.rotateImage(false);
    updateDisplays();
}

void GUI::drawLine() {
    // Simplified for now
}

void GUI::undo() {
    if (!hasImage_) return;
    controller_.undo();
    updateDisplays();
}

void GUI::redo() {
    if (!hasImage_) return;
    controller_.redo();
    updateDisplays();
}

void GUI::toggleGPU(int state) {
    controller_.setUseGPU(state == Qt::Checked);
}

void GUI::toggleMultithreading(int state) {
    controller_.setUseMultithreading(state == Qt::Checked);
}

cv::Mat GUI::getCurrentMat() const {
    cv::UMat umat = controller_.getCurrent();
    cv::Mat mat;
    umat.copyTo(mat);
    return mat;
}

QImage GUI::scaledImage(const QImage& img, int maxW, int maxH) const {
    return img.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void GUI::updateDisplays() {
    if (!hasImage_) return;
    
    cv::Mat current = getCurrentMat();
    QImage qimg = matToImage(current);
    
    editedLabel_.setPixmap(QPixmap::fromImage(scaledImage(qimg, editedLabel_.width(), editedLabel_.height())));
    
    // Update original if needed (for simplicity, just showing current in edited)
    // In a real app, you'd keep the original loaded separately.
}
