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
    
    connect(blurSlider, &QSlider::sliderReleased, this, &GUI::onBlurSliderReleased);
    connect(bilateralSlider, &QSlider::sliderReleased, this, &GUI::onBilateralSliderReleased);

    connect(&btnMirrorH_, &QPushButton::clicked, this, &GUI::applyMirrorHorizontal);
    connect(&btnMirrorV_, &QPushButton::clicked, this, &GUI::applyMirrorVertical);
    connect(&btnRotateCW_, &QPushButton::clicked, this, &GUI::rotateClockwise);
    connect(&btnRotateCCW_, &QPushButton::clicked, this, &GUI::rotateCounterClockwise);
    connect(&btnDrawLine_, &QPushButton::clicked, this, &GUI::drawLine);
    
    connect(&cbUseGPU_, &QCheckBox::stateChanged, this, &GUI::toggleGPU);
    connect(&cbUseMultithreading_, &QCheckBox::stateChanged, this, &GUI::toggleMultithreading);
    updateUndoRedoButtons();
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
    originalLabel_.setMinimumSize(400, 400);
    originalLabel_.setMaximumSize(400, 400);
    originalLabel_.setScaledContents(true);
    
    editedLabel_.setText("Edited Image");
    editedLabel_.setAlignment(Qt::AlignCenter);
    editedLabel_.setMinimumSize(400, 400);
    editedLabel_.setStyleSheet("border: 1px solid black;");
    editedLabel_.setMinimumSize(400, 400);
    editedLabel_.setMaximumSize(400, 400);
    editedLabel_.setScaledContents(true);
    
    imageLayout->addWidget(&originalLabel_);
    imageLayout->addWidget(&editedLabel_);
    mainLayout->addLayout(imageLayout);
    
    // Filters
    QHBoxLayout* filtersLayout = new QHBoxLayout();
    
    // Gaussian Blur
    QGroupBox* blurGroup = new QGroupBox("Gaussian Blur");
    QVBoxLayout* blurLayout = new QVBoxLayout();
    blurSlider = new QSlider(Qt::Horizontal);
    blurSlider->setRange(1, 100);
    blurSlider->setValue(30);
    blurSlider->setTickInterval(10);
    blurSlider->setTickPosition(QSlider::TicksBelow);
    blurValueLabel = new QLabel("30");
    blurValueLabel->setAlignment(Qt::AlignCenter);
    blurValueLabel->setMinimumWidth(30);
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(new QLabel("Strength:"));
    sliderLayout->addWidget(blurSlider);
    sliderLayout->addWidget(blurValueLabel);
    blurLayout->addLayout(sliderLayout);
    blurGroup->setLayout(blurLayout);
    
    // Bilateral Filter
    QGroupBox* bilateralGroup = new QGroupBox("Bilateral Filter");
    QVBoxLayout* bilateralLayout = new QVBoxLayout();
    bilateralSlider = new QSlider(Qt::Horizontal);
    bilateralSlider->setRange(1, 100);
    bilateralSlider->setValue(50);
    bilateralSlider->setTickInterval(10);
    bilateralSlider->setTickPosition(QSlider::TicksBelow);
    bilateralValueLabel = new QLabel("50");
    bilateralValueLabel->setAlignment(Qt::AlignCenter);
    bilateralValueLabel->setMinimumWidth(30);
    QHBoxLayout* bilatSliderLayout = new QHBoxLayout();
    QLabel* bilatStrengthLabel = new QLabel("Strength:");
    bilatSliderLayout->addWidget(bilatStrengthLabel);
    bilatSliderLayout->addWidget(bilateralSlider);
    bilatSliderLayout->addWidget(bilateralValueLabel);
    bilatSliderLayout->setStretchFactor(bilatStrengthLabel, 0);
    bilatSliderLayout->setStretchFactor(bilateralSlider, 1);
    bilatSliderLayout->setStretchFactor(bilateralValueLabel, 0);
    bilateralLayout->addLayout(bilatSliderLayout);
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

    filtersLayout->setStretchFactor(blurGroup, 1);
    filtersLayout->setStretchFactor(bilateralGroup, 1);
    filtersLayout->setStretchFactor(transformGroup, 1);
    
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
            originalImage_ = getCurrentMat();
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
    
    int strength = blurSlider->value();
    int kernelSize = 3 + (strength * 28) / 100;
    if (kernelSize % 2 == 0) kernelSize++;
    double sigma = 0.5 + (strength * 9.5) / 100;

    int bilatStrength = bilateralSlider->value();
    int d = 1 + (bilatStrength * 30) / 100;
    if (d % 2 == 0) d++;
    double sigmaColor = 1.0 + (bilatStrength * 150.0) / 100;
    double sigmaSpace = 1.0 + (bilatStrength * 150.0) / 100;

    controller_.processFolder(
        inputDir.toStdString(),
        outputDir.toStdString(),
        kernelSize, sigma,
        d, sigmaColor, sigmaSpace,
        true, true 
    );

    
    progressBar_.setValue(100);
    QMessageBox::information(this, "Success", "Batch processing completed!");
    progressBar_.setValue(0);


}


void GUI::applyGaussianBlur() {
    if (!hasImage_) return;
    int strength = blurSlider->value();     
    int kernelSize = 3 + (strength * 28) / 100;
    if (kernelSize % 2 == 0) kernelSize++;     
    double sigma = 0.5 + (strength * 9.5) / 100;
    blurValueLabel->setText(QString::number(strength));
    controller_.BlurImage(kernelSize, sigma);
    updateDisplays();
}

void GUI::onBlurSliderReleased() {
    blurValueLabel->setText(QString::number(blurSlider->value()));
    applyGaussianBlur();
}

void GUI::onBilateralSliderReleased() {
    bilateralValueLabel->setText(QString::number(bilateralSlider->value()));
    applyBilateralFilter();
}

void GUI::applyBilateralFilter() {
    if (!hasImage_) return;

    int strength = bilateralSlider->value(); 

    int d = 1 + (strength * 30) / 100;       
    if (d % 2 == 0) d++;                      

    double sigmaColor = 1.0 + (strength * 150.0) / 100;  
    double sigmaSpace = 1.0 + (strength * 150.0) / 100; 

    bilateralValueLabel->setText(QString::number(strength));

    controller_.BilateralFilter(d, sigmaColor, sigmaSpace);
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

void GUI::updateUndoRedoButtons() {
    btnUndo_.setEnabled(controller_.canUndo());
    btnRedo_.setEnabled(controller_.canRedo());
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

    QImage qorig = matToImage(originalImage_);
    originalLabel_.setPixmap(QPixmap::fromImage(
        scaledImage(qorig, originalLabel_.width(), originalLabel_.height())
    ));

    cv::Mat current = getCurrentMat();
    QImage qedit = matToImage(current);
    editedLabel_.setPixmap(QPixmap::fromImage(
        scaledImage(qedit, editedLabel_.width(), editedLabel_.height())
    ));
    updateUndoRedoButtons();
}


