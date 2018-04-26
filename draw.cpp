#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

int main(int argc, const char *argv[])
{
    cv::Mat im1 = imread(argv[1]);
    cv::Mat im2 = imread(argv[2]);

    float x1, y1, x2, y2, distance;
    ifstream in(argv[3]);
    cout << " - " << endl;
    cv::RNG rng(time(0));
    srand((unsigned)time(NULL));
    while (in >> x1 >> y1 >> x2 >> y2 >> distance) {
        cout << "[" << x1 << ", " << y1 << "] - [" << x2 << ", " << y2 << "] - distance: " << distance << endl;
        auto c1 = rng.uniform(0,255);
        auto c2 = rng.uniform(0,255);
        auto c3 = rng.uniform(0,255);
        int r = rand() % 10 + 8;
        circle(im1, Point2f(x1, y1), r, Scalar(c1, c2, c3), 2);
        circle(im2, Point2f(x2, y2), r, Scalar(c1, c2, c3), 2);
    }
    in.close();
    namedWindow(argv[1], WINDOW_NORMAL);
    namedWindow(argv[2], WINDOW_NORMAL);
    imshow(argv[1], im1);
    imshow(argv[2], im2);
    waitKey(0);
    return 0;
}
