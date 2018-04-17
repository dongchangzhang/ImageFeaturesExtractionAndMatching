//
//  utils.h
//  sift_asift_match
//
//  Created by willard on 8/21/15.
//  Copyright (c) 2015 wilard. All rights reserved.
//

#ifndef __sift_asift_match__utils__
#define __sift_asift_match__utils__

#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"

using namespace std;
using namespace cv;

struct MatchedPoint{
    Point2f p1;
    Point2f p2;
    MatchedPoint(Point2f _p1, Point2f _p2) : p1(_p1), p2(_p2) {}
};
void drawMatch(const string &src, const string &obj, vector<Point2f> &srcPoints, vector<Point2f> &dstPoints);
vector<MatchedPoint> findInliers(vector<KeyPoint> &qKeypoints, vector<KeyPoint> &objKeypoints, vector<DMatch> &matches, const string &imgfn, const string &objFileName);
void writeIntoFile(const vector<MatchedPoint> &mps, const string &name);

#endif /* defined(__sift_asift_match__utils__) */
