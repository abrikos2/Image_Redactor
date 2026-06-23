#pragma once
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QSpinBox>
#include <Controller.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <string>

// Convert cv::Mat (BGR) to QImage (RGB)
static inline QImage matToImage(const cv::Mat& mat) {
    if (mat.empty()) return QImage();
    cv::Mat rgb;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 1) {
        rgb = mat;
    } else {
        return QImage();
    }
    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step,
                  QImage::Format_RGB888).copy();
}

class GUI : public QWidget {
    Q_OBJECT

public:
    explicit GUI(QWidget* parent = nullptr);
    ~GUI() override = default;

private slots:
    // File operations
    void openImage();
    void saveImage();
    void processFolder();
    void processSingleImage(const std::string& inputPath, const std::string& outputPath);

    // Edit operations
    void applyGaussianBlur();
    void applyBilateralFilter();
    void applyMirrorHorizontal();
    void applyMirrorVertical();
    void rotateClockwise();
    void rotateCounterClockwise();
    void drawLine();

    // Toggles
    void toggleGPU(int state);
    void toggleMultithreading(int state);

    // Undo/Redo
    void undo();
    void redo();

private:
    void buildUI();
    void updateDisplays();
    cv::Mat getCurrentMat() const;
    QImage scaledImage(const QImage& img, int maxW, int maxH) const;

    // Controller
    Controller controller_;
    bool hasImage_ = false;
    std::string currentFilePath_;

    // Labels for displaying images (original vs edited)
    QLabel originalLabel_;
    QLabel editedLabel_;

    // Controls
    QPushButton btnOpen_;
    QPushButton btnSave_;
    QPushButton btnProcessFolder_;
    QPushButton btnUndo_;
    QPushButton btnRedo_;

    QSpinBox blurKernelSize_;
    QDoubleSpinBox blurSigma_;
    QPushButton btnBlur_;

    QSpinBox bilateralD_;
    QDoubleSpinBox bilateralSigmaColor_;
    QDoubleSpinBox bilateralSigmaSpace_;
    QPushButton btnBilateral_;

    QCheckBox cbUseGPU_;
    QCheckBox cbUseMultithreading_;

    QPushButton btnMirrorH_;
    QPushButton btnMirrorV_;

    QPushButton btnRotateCW_;
    QPushButton btnRotateCCW_;

    QPushButton btnDrawLine_;
    QLineEdit drawX1Edit_, drawY1Edit_, drawX2Edit_, drawY2Edit_, drawColorEdit_, drawThicknessEdit_;

    // Progress bar for batch processing
    QProgressBar progressBar_;
};
