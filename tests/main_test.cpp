#include <gtest/gtest.h>
#include "Image.h"
#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat createTestImage(int width, int height, cv::Scalar color) {
    return cv::Mat(height, width, CV_8UC3, color);
}

std::string saveTempImage(const cv::Mat& img, const std::string& filename) {
    std::string path = filename;
    cv::imwrite(path, img);
    return path;
}

TEST(ImageTest, LoadAndSave) {
    Image img;
    cv::Mat testMat = createTestImage(100, 100, cv::Scalar(0, 0, 255)); 
    std::string tempFile = saveTempImage(testMat, "test_load.png");

    EXPECT_TRUE(img.loadFromFile(tempFile));
    
    cv::UMat current = img.getCurrent();
    EXPECT_FALSE(current.empty());
    EXPECT_EQ(current.cols, 100);
    EXPECT_EQ(current.rows, 100);

    EXPECT_TRUE(img.SaveToFile("test_save.png"));
    
    cv::Mat savedMat = cv::imread("test_save.png");
    EXPECT_FALSE(savedMat.empty());
    EXPECT_EQ(savedMat.cols, 100);
    EXPECT_EQ(savedMat.rows, 100);

    std::remove("test_load.png");
    std::remove("test_save.png");
}

TEST(ImageTest, MirrorHorizontal) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    testMat(cv::Rect(50, 0, 50, 100)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_mirror_h.png");

    img.loadFromFile(tempFile);
    img.applyMirror(true); 

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    cv::Vec3b leftPixel = result.at<cv::Vec3b>(50, 25);
    cv::Vec3b rightPixel = result.at<cv::Vec3b>(50, 75);

    EXPECT_EQ(leftPixel, cv::Vec3b(255, 255, 255));
    EXPECT_EQ(rightPixel, cv::Vec3b(0, 0, 0));

    std::remove("test_mirror_h.png");
}

TEST(ImageTest, MirrorVertical) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    testMat(cv::Rect(0, 50, 100, 50)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_mirror_v.png");

    img.loadFromFile(tempFile);
    img.applyMirror(false);

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    cv::Vec3b topPixel = result.at<cv::Vec3b>(25, 50);
    cv::Vec3b bottomPixel = result.at<cv::Vec3b>(75, 50);

    EXPECT_EQ(topPixel, cv::Vec3b(255, 255, 255));
    EXPECT_EQ(bottomPixel, cv::Vec3b(0, 0, 0));

    std::remove("test_mirror_v.png");
}

TEST(ImageTest, RotateClockwise) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(50, 100, CV_8UC3);
    testMat(cv::Rect(0, 0, 50, 25)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_rotate_cw.png");

    img.loadFromFile(tempFile);
    img.rotate(true); 

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    EXPECT_EQ(result.cols, 50);
    EXPECT_EQ(result.rows, 100);

    cv::Vec3b topRightPixel = result.at<cv::Vec3b>(25, 25); 
    cv::Vec3b bottomLeftPixel = result.at<cv::Vec3b>(75, 25); 

    EXPECT_EQ(topRightPixel, cv::Vec3b(255, 255, 255));
    EXPECT_EQ(bottomLeftPixel, cv::Vec3b(0, 0, 0));

    std::remove("test_rotate_cw.png");
}

TEST(ImageTest, GaussianBlur) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    testMat(cv::Rect(50, 0, 50, 100)).setTo(cv::Scalar(255, 255, 255));
    std::string tempFile = saveTempImage(testMat, "test_blur.png");

    img.loadFromFile(tempFile);
    
    cv::UMat beforeU = img.getCurrent();
    cv::Mat before;
    beforeU.copyTo(before);
    EXPECT_EQ(before.at<cv::Vec3b>(50, 49), cv::Vec3b(0, 0, 0));
    EXPECT_EQ(before.at<cv::Vec3b>(50, 50), cv::Vec3b(255, 255, 255));

    img.applyGausBlur(15, 5.0);

    cv::UMat resultU = img.getCurrent();
    cv::Mat result;
    resultU.copyTo(result);

    cv::Vec3b pixelLeft = result.at<cv::Vec3b>(50, 49);
    cv::Vec3b pixelRight = result.at<cv::Vec3b>(50, 50);

    EXPECT_GT(pixelLeft[0], 0); 
    EXPECT_LT(pixelRight[0], 255); 

    std::remove("test_blur.png");
}

TEST(ImageTest, UndoRedo) {
    Image img;
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    std::string tempFile = saveTempImage(testMat, "test_undo.png");

    img.loadFromFile(tempFile);

    EXPECT_TRUE(img.canUndo());
    EXPECT_FALSE(img.canRedo());

    img.applyMirror(true);
    EXPECT_TRUE(img.canUndo());  
    EXPECT_FALSE(img.canRedo()); 

    img.undo();
    EXPECT_TRUE(img.canUndo()); 
    EXPECT_TRUE(img.canRedo()); 

    img.redo();
    EXPECT_TRUE(img.canUndo());  
    EXPECT_FALSE(img.canRedo()); 

    img.applyMirror(false);
    EXPECT_TRUE(img.canUndo()); 
    EXPECT_FALSE(img.canRedo()); 

    img.undo();
    img.undo();
    EXPECT_TRUE(img.canUndo()); 
    EXPECT_TRUE(img.canRedo());

    std::remove("test_undo.png");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
