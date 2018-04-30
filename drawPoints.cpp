#include "feature.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace cv;

int main(int argc, const char *argv[])
{
    cv::Mat im1 = imread(argv[1]);
    cv::Mat im2 = imread(argv[2]);
    vector<MatchedPoint> mps;
    float x1, y1, x2, y2, distance;
    ifstream in(argv[3]);
    while (in >> x1 >> y1 >> x2 >> y2 >> distance) {
        mps.push_back(MatchedPoint(cv::Point2f(x1, y1), cv::Point2f(x2, y2), distance));
    }
    in.close();
    FeatureTool ft;
    ft.drawMatch(argv[1], argv[2], mps, "Lines", 1);
    return 0;
}
