
#ifndef __feature__h__
#define __feature_h__

// if define, then you will see the image with matched result
#define _SHOW_IMAGE_AND_LOG_
// use gray image to detect keypoints -- 0
#define INPUT_COLOR 0

#define NN_MATCH_RATIO 0.8

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

// opencv3
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

// struct fro save matched result
struct MatchedPoint{
    cv::Point2f p1;
    cv::Point2f p2;
    float distance;
    MatchedPoint() {}
    MatchedPoint(cv::Point2f _p1, cv::Point2f _p2, float _d)
        : p1(_p1), p2(_p2), distance(_d) { }
};

// hash functions for unordered_map
struct hashFunc
{
    inline size_t operator()(const cv::Point &point) const
    {
         return (point.x + point.y) % 499;
    }
};

struct cmpFun
{
    inline bool operator()(const cv::Point &p1, const cv::Point &p2) const
    {
         return p1.x == p2.x && p1.y == p2.y;
    }
};

// extract features and match them
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
    enum Direction {
        SINGLE,
        BOTH,
    };
public:

    FeatureTool() {}

    ~FeatureTool() {}

    // api for feature match
    std::vector<MatchedPoint> getMatchedPoints(
        const std::string &srcImage,
        const std::string &objImage,
        FeatureType type=SIFT,
        Direction direction=SINGLE);

    // init cv::Ptr by feature type
    int init(cv::Ptr<cv::Feature2D> &fp, FeatureType type);

    int extractFeature(
        const std::string &image,
        const cv::Ptr<cv::Feature2D> &fp,
        std::vector<cv::KeyPoint> &kps,
        cv::Mat &desc);

    //  harris
    cv::Mat extractHarrisFeature(
        const std::string &srcImage,
        std::vector<cv::KeyPoint> &points,
        int thresh);

    /* match features and get the match result: 'matches' */

    std::vector<std::vector<cv::DMatch>> matchPointsByFlann(
        const cv::Mat &qDesc,
        const cv::Mat &objDesc,
        const FeatureTool::FeatureType type=FeatureTool::NONE);

    std::vector<std::vector<cv::DMatch>> matchPointsByBF(
        const cv::Mat &qDesc,
        const cv::Mat &objDesc,
        const FeatureTool::FeatureType type=FeatureTool::NONE);

    /* filter: input 'matches' and then delete the outliers then return the final result*/

    // 1. distance-best / distance-second-best > ratio
    std::vector<cv::DMatch> getGoodMatches(
        const std::vector<std::vector<cv::DMatch>> &matches);

    // 2. ransac via fundamental or essential matrix
    std::vector<MatchedPoint> findInliersByRANSAC(
        const std::vector<cv::KeyPoint> &qKeypoints,
        const std::vector<cv::KeyPoint> &objKeypoints,
        const std::vector<cv::DMatch> &matches);
    
    // 3. both direction
    size_t matchSymmetric(
        const std::vector<MatchedPoint> &matchedPoint,
        const std::vector<MatchedPoint> &rmatchedPoint,
        std::vector<MatchedPoint> &result);

    /* tools */

    // write matched result into file
    void writeMatchedPointsIntoFile(
        const std::vector<MatchedPoint> &mps,
        const std::string &name);

    // draw match result in images
    void drawMatch(
        const std::string &src,
        const std::string &obj,
        const std::vector<MatchedPoint> &mps,
        const std::string &windowName,
        const int type=0);
    
    void drawKeypoints(
        const std::string &image,
        const std::string &name,
        const std::vector<cv::KeyPoint> &kps);

private:
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
