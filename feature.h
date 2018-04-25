
#ifndef __feature__h__
#define __feature_h__

// show matched image ?
#define _SHOW_IMAGE_
// use gray image to detect keypoints -- 0
#define INPUT_COLOR 0
// function to match points
// #define FLANN
// #define BF
#define RF

#include <algorithm>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <vector>

using namespace std;
using namespace cv;

struct MatchedPoint{
    Point2f p1;
    Point2f p2;
    float distance;
    MatchedPoint(Point2f _p1, Point2f _p2, float _d)
        : p1(_p1), p2(_p2), distance(_d) { }
};

// api for feature match
vector<MatchedPoint> getMatchedPoints(
        const string &image1,
        const string &image2);


Mat extractSIFTFeature(
        const string &imgfn,
        vector<KeyPoint> &points);

// filter for matched points
vector<MatchedPoint> findInliers(
        vector<KeyPoint> &qKeypoints,
        vector<KeyPoint> &objKeypoints,
        vector<DMatch> &matches,
        const string &imgfn,
        const string &objFileName);

// tools
void drawMatch(
        const string &src,
        const string &obj,
        vector<Point2f> &srcPoints,
        vector<Point2f> &dstPoints,
        const string &windowName);


void writeIntoFile(
        const vector<MatchedPoint> &mps,
        const string &name);

vector<MatchedPoint> matchPointsByFlann(
    Mat &qDescriptor,
    vector<KeyPoint> &qKeypoints,
    const string &srcImage,
    Mat &objDescriptor,
    vector<KeyPoint> &objKeypoints,
    const string &objImage);

vector<MatchedPoint> matchPointsByBF(
    Mat &qDescriptor,
    vector<KeyPoint> &qKeypoints,
    const string &srcImage,
    Mat &objDescriptor,
    vector<KeyPoint> &objKeypoints,
    const string &objImage);

vector<MatchedPoint> matchPointsByRawFeature(
    Mat &qDescriptor,
    vector<KeyPoint> &qKeypoints,
    const string &srcImage,
    Mat &objDescriptor,
    vector<KeyPoint> &objKeypoints,
    const string &objImage);

void sortPointsByDistance(vector<MatchedPoint> &points);

#endif /* defined(__sift_h__) */
