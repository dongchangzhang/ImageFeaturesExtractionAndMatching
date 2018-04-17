#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

int main()
{
    Mat im1 = imread("9.JPG");
    Mat im2 = imread("10.JPG");

    float x1, y1, x2, y2;
    ifstream in("output.txt");
    cout << " - " << endl;
    int i = 0;
    while (in >> x1 >> y1 >> x2 >> y2) {
        if (i >= 20) i = 0;
        cout << x1 << " - " << y1 << " - " << x2 << " - " << y2 << endl;
        circle(im1, Point2f(x1, y1), 5 + i, Scalar(i * 10, i * 5, 255 - i * 10), 2);
        circle(im2, Point2f(x2, y2), 5 + i, Scalar(i * 10, i * 5, 255 - i * 10), 2);
        i++;
    }
    in.close();
    cout << " - " << endl;
    namedWindow("2", WINDOW_NORMAL);
    namedWindow("3", WINDOW_NORMAL);
    imshow("2", im1);
    imshow("3", im2);
    waitKey(0);
    return 0;
}
