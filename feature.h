
#ifndef __feature__h__
#define __feature_h__

// show matched image ?
#define _SHOW_IMAGE_
// use gray image to detect keypoints -- 0
#define INPUT_COLOR 0

#define NN_MATCH_RATIO 0.8
// function to match points
// #define FLANN
// #define BF
#define RF

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

// opencv3
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

struct MatchedPoint{
    cv::Point2f p1;
    cv::Point2f p2;
    float distance;
    MatchedPoint() {}
    MatchedPoint(cv::Point2f _p1, cv::Point2f _p2, float _d)
        : p1(_p1), p2(_p2), distance(_d) { }
};

class FeatureTool {
public:
    enum FeatureType {
        SIFT,
        SURF,
        KAZE,
        AKAZE,
        HARRIS,
        NONE,
    };
public:

    FeatureTool() {}

    ~FeatureTool() {}

    // api for feature match
    std::vector<MatchedPoint> getMatchedPoints(
        const std::string &srcImage,
        const std::string &objImage);

    /* get features, saving the keypoints into points and then return the descriptors */

    int extractFeature(
        const std::string &srcImage,
        const FeatureType feature,
        std::vector<cv::KeyPoint> &keyPoints,
        cv::Mat &descriptors);

    // 1. sift
    cv::Mat extractSIFTFeature(
        const std::string &srcImage,
        std::vector<cv::KeyPoint> &points);
    // 2. kaze
    cv::Mat extractKAZEFeature(
        const std::string &srcImage,
        std::vector<cv::KeyPoint> &points);
    // 3. akaze
    cv::Mat extractAKAZEFeature(
        const std::string &srcImage,
        std::vector<cv::KeyPoint> &points);
    // 4. surf
    cv::Mat extractSURFFeature(
        const std::string &srcImage,
        std::vector<cv::KeyPoint> &points);
    // 5. harris
    cv::Mat extractHarrisFeature(
        const std::string &srcImage,
        std::vector<cv::KeyPoint> &points,
        int threshod);

    /* match features and get the match result: 'matches' */

    std::vector<std::vector<cv::DMatch>> matchPointsByFlann(
        const cv::Mat &qDescriptor,
        const cv::Mat &objDescriptor,
        const FeatureTool::FeatureType type=FeatureTool::NONE);

    std::vector<std::vector<cv::DMatch>> matchPointsByBF(
        const cv::Mat &qDescriptor,
        const cv::Mat &objDescriptor,
        const FeatureTool::FeatureType type=FeatureTool::NONE);

    /* filter: input 'matches' and then delete the outliers then return the final result*/

    // 1. distance-best / distance-second-best > ratio
    std::vector<cv::DMatch> getGoodMatches(
        const std::vector<std::vector<cv::DMatch>> &matches);

    // 2. ransac via fundamental or ? matrix
    std::vector<MatchedPoint> findInliersByRANSAC(
        const std::vector<cv::KeyPoint> &qKeypoints,
        const std::vector<cv::KeyPoint> &objKeypoints,
        const std::vector<cv::DMatch> &matches);

    // write matched result into file
    void writeMatchedPointsIntoFile(
        const std::vector<MatchedPoint> &mps,
        const std::string &name);
private:

    /* tools */

    // draw match result in images
    void drawMatch(
        const std::string &src,
        const std::string &obj,
        const std::vector<MatchedPoint> &mps,
        const std::string &windowName);
    
    void drawKeypoints(
        const std::string &image,
        const std::string &name,
        const std::vector<cv::KeyPoint> &kps);

    // sort matched points by the distance
    void sortPointsByDistance(std::vector<MatchedPoint> &points);

    std::vector<MatchedPoint> matches2MatchedPoints(
        const std::vector<cv::KeyPoint> &qKeypoints,
        const std::vector<cv::KeyPoint> &objKeypoints,
        const std::vector<cv::DMatch> &matches);

    std::vector<MatchedPoint> matches2MatchedPoints(
        const std::vector<cv::Point2f> &queryInliers,
        const std::vector<cv::Point2f> &sceneInliers,
        const std::vector<float> &distances);
};
#endif /* defined(__feature_h__) */
