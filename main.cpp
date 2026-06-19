#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    Mat image = imread("photo.jpg", IMREAD_COLOR);

    if (image.empty()) {
        cout << "ERROR" << endl;
        return -1;
    }



    int x = 10, y = 20;
    Vec3b pixel = image.at<Vec3b>(y, x);
    cout << "B: " << (int)pixel[0] << endl;  
    cout << "G: " << (int)pixel[1] << endl;
    cout << "R: " << (int)pixel[2] << endl;

    image.at<Vec3b>(y, x) = Vec3b(0, 0, 255);

    imwrite("output.jpg", image);
    return 0;
}