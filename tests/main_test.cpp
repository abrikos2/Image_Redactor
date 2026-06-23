#include <gtest/gtest.h>
#include "Image.h"
#include <opencv2/opencv.hpp>
#include <iostream>

// Вспомогательная функция для создания тестового изображения
cv::Mat createTestImage(int width, int height, cv::Scalar color) {
    return cv::Mat(height, width, CV_8UC3, color);
}

// Вспомогательная функция для сохранения тестового изображения во временный файл
std::string saveTempImage(const cv::Mat& img, const std::string& filename) {
    std::string path = filename;
    cv::imwrite(path, img);
    return path;
}

// Тест загрузки и сохранения
TEST(ImageTest, LoadAndSave) {
    Image img;
    cv::Mat testMat = createTestImage(100, 100, cv::Scalar(0, 0, 255)); // Красное изображение
    std::string tempFile = saveTempImage(testMat, "test_load.png");

    EXPECT_TRUE(img.loadFromFile(tempFile));
    
    cv::UMat current = img.getCurrent();
    EXPECT_FALSE(current.empty());
    EXPECT_EQ(current.cols, 100);
    EXPECT_EQ(current.rows, 100);

    EXPECT_TRUE(img.SaveToFile("test_save.png"));
    
    // Проверяем, что сохраненный файл существует и читается
    cv::Mat savedMat = cv::imread("test_save.png");
    EXPECT_FALSE(savedMat.empty());
    EXPECT_EQ(savedMat.cols, 100);
    EXPECT_EQ(savedMat.rows, 100);

    // Очистка
    std::remove("test_load.png");
    std::remove("test_save.png");
}

// Тест отзеркаливания (горизонтальное)
TEST(ImageTest, MirrorHorizontal) {
    Image img;
    // Создаем изображение: левая половина черная, правая белая
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    testMat(cv::Rect(50, 0, 50, 100)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_mirror_h.png");

    img.loadFromFile(tempFile);
    img.applyMirror(true); // Горизонтальное отзеркаливание

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    // После отзеркаливания левая половина должна стать белой, правая - черной
    cv::Vec3b leftPixel = result.at<cv::Vec3b>(50, 25);
    cv::Vec3b rightPixel = result.at<cv::Vec3b>(50, 75);

    EXPECT_EQ(leftPixel, cv::Vec3b(255, 255, 255));
    EXPECT_EQ(rightPixel, cv::Vec3b(0, 0, 0));

    std::remove("test_mirror_h.png");
}

// Тест отзеркаливания (вертикальное)
TEST(ImageTest, MirrorVertical) {
    Image img;
    // Создаем изображение: верхняя половина черная, нижняя белая
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    testMat(cv::Rect(0, 50, 100, 50)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_mirror_v.png");

    img.loadFromFile(tempFile);
    img.applyMirror(false); // Вертикальное отзеркаливание

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    // После отзеркаливания верхняя половина должна стать белой, нижняя - черной
    cv::Vec3b topPixel = result.at<cv::Vec3b>(25, 50);
    cv::Vec3b bottomPixel = result.at<cv::Vec3b>(75, 50);

    EXPECT_EQ(topPixel, cv::Vec3b(255, 255, 255));
    EXPECT_EQ(bottomPixel, cv::Vec3b(0, 0, 0));

    std::remove("test_mirror_v.png");
}

// Тест поворота (по часовой)
TEST(ImageTest, RotateClockwise) {
    Image img;
    // Создаем прямоугольное изображение 100x50
    cv::Mat testMat = cv::Mat::zeros(50, 100, CV_8UC3);
    // Закрашиваем верхнюю левую четверть (50x25) белым
    testMat(cv::Rect(0, 0, 50, 25)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_rotate_cw.png");

    img.loadFromFile(tempFile);
    img.rotate(true); // По часовой

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    // Размеры должны поменяться местами
    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.rows, 100);

    // Белый прямоугольник должен переместиться в верхний правый угол
    cv::Vec3b topRightPixel = result.at<cv::Vec3b>(25, 25); // Внутри белой области
    cv::Vec3b bottomLeftPixel = result.at<cv::Vec3b>(75, 25); // Внутри черной области

    EXPECT_EQ(topRightPixel, cv::Vec3b(255, 255, 255));
    EXPECT_EQ(bottomLeftPixel, cv::Vec3b(0, 0, 0));

    std::remove("test_rotate_cw.png");
}

// Тест размытия по Гауссу
TEST(ImageTest, GaussianBlur) {
    Image img;
    // Создаем изображение с резким переходом
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    testMat(cv::Rect(50, 0, 50, 100)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_blur.png");

    img.loadFromFile(tempFile);
    
    // Проверяем резкий переход до размытия
    cv::UMat beforeU = img.getCurrent();
    cv::Mat before;
    beforeU.copyTo(before);
    EXPECT_EQ(before.at<cv::Vec3b>(50, 49), cv::Vec3b(0, 0, 0));
    EXPECT_EQ(before.at<cv::Vec3b>(50, 50), cv::Vec3b(255, 255, 255));

    img.applyGausBlur(15, 5.0);

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    // После размытия переход должен стать плавным (значения между 0 и 255)
    cv::Vec3b pixelLeft = result.at<cv::Vec3b>(50, 49);
    cv::Vec3b pixelRight = result.at<cv::Vec3b>(50, 50);

    EXPECT_GT(pixelLeft[0], 0); // Должно стать светлее черного
    EXPECT_LT(pixelRight[0], 255); // Должно стать темнее белого

    std::remove("test_blur.png");
}

// Тест рисования
TEST(ImageTest, DrawOnImage) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    std::string tempFile = saveTempImage(testMat, "test_draw.png");

    img.loadFromFile(tempFile);
    
    cv::Point p1(10, 10);
    cv::Point p2(90, 10);
    cv::Scalar color(0, 255, 0); // Зеленая линия
    
    img.drawOnImage(p1, p2, color, 1);

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    // Проверяем пиксель на линии
    EXPECT_EQ(result.at<cv::Vec3b>(10, 50), cv::Vec3b(0, 255, 0));
    // Проверяем пиксель вне линии
    EXPECT_EQ(result.at<cv::Vec3b>(20, 50), cv::Vec3b(0, 0, 0));

    std::remove("test_draw.png");
}

// Тест Undo/Redo
TEST(ImageTest, UndoRedo) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    std::string tempFile = saveTempImage(testMat, "test_undo.png");

    img.loadFromFile(tempFile);
    
    // После загрузки файла состояние сохраняется в undoStack
    EXPECT_TRUE(img.canUndo());
    EXPECT_FALSE(img.canRedo());

    // Действие 1: Рисуем линию
    img.drawOnImage(cv::Point(0,0), cv::Point(10,10), cv::Scalar(255,0,0), 1);
    EXPECT_TRUE(img.canUndo());
    EXPECT_FALSE(img.canRedo());

    // Действие 2: Отзеркаливаем
    img.applyMirror(true);
    
    // Отменяем отзеркаливание
    img.undo();
    EXPECT_TRUE(img.canRedo());
    
    // Отменяем рисование
    img.undo();
    EXPECT_TRUE(img.canUndo()); // Все еще можно отменить загрузку файла
    EXPECT_TRUE(img.canRedo());

    // Повторяем рисование
    img.redo();
    EXPECT_TRUE(img.canUndo());
    EXPECT_TRUE(img.canRedo());

    std::remove("test_undo.png");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
