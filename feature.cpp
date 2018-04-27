#include "feature.h"

/* api */
std::vector<MatchedPoint> FeatureTool::getMatchedPoints(
        const std::string &srcImage,
        const std::string &objImage)
{
    // for test 
    // cv::Mat Descriptor;
    // std::vector<cv::KeyPoint> Keypoints;
    // extractKAZEFeature(srcImage, Keypoints);
    // std::cout << "KAZE " << Keypoints.size() << std::endl; 
    // drawKeypoints(srcImage, "KAZE", Keypoints);

    // drawKeypoints(srcImage, "SIFT", Keypoints);
    
    // return std::vector<MatchedPoint>();

    // test end
    // extract features

    cv::Mat qDescriptor;
    std::vector<cv::KeyPoint> qKeypoints;
    qDescriptor = extractSIFTFeature(srcImage, qKeypoints);
    std::cout << "image1 keypoints: " << qKeypoints.size() << std::endl; 

    cv::Mat objDescriptor;
    std::vector<cv::KeyPoint> objKeypoints;
    objDescriptor = extractSIFTFeature(objImage, objKeypoints);
    std::cout << "image2 keypoints: " << objKeypoints.size() << std::endl; 
    // match
    auto matches = matchPointsByBF(qDescriptor, objDescriptor, FeatureTool::SIFT);

    std::cout << "matches: " << matches.size() << std::endl; 

    // filter
    auto goodMatches = getGoodMatches(matches);

    std::cout << "good matches: " << goodMatches.size() << std::endl; 

    drawMatch(srcImage, objImage, matches2MatchedPoints(qKeypoints, objKeypoints, goodMatches), "Origin");

    std::cout << " begin ransac " << std::endl;
    auto points = findInliersByRANSAC(qKeypoints, objKeypoints, goodMatches);

    std::cout << "ransac: " << points.size() << std::endl; 

    drawMatch(srcImage, objImage, points, "After");

    // result
    sortPointsByDistance(points);

    return points;
}

// sift
cv::Mat FeatureTool::extractSIFTFeature(
    const std::string &srcImage, 
    std::vector<cv::KeyPoint> &points)
{
    points.clear();
    cv::Mat descriptors;
    auto img = cv::imread(srcImage, INPUT_COLOR);

    cv::Ptr<cv::Feature2D> sift = cv::xfeatures2d::SIFT::create();
    sift->detectAndCompute(img, cv::noArray(), points, descriptors);
    return descriptors;
}

// 2. kaze
cv::Mat FeatureTool::extractKAZEFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{

    points.clear();
    cv::Mat descriptors;
    auto img = cv::imread(srcImage, INPUT_COLOR);

    cv::Ptr<cv::KAZE> kaze = cv::KAZE::create();
    kaze->detectAndCompute(img, cv::noArray(), points, descriptors);
    return descriptors;
}
// 3. akaze
cv::Mat FeatureTool::extractAKAZEFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{

    points.clear();
    cv::Mat descriptors;
    auto img = cv::imread(srcImage, INPUT_COLOR);

    cv::Ptr<cv::AKAZE> akaze = cv::AKAZE::create();
    akaze->detectAndCompute(img, cv::noArray(), points, descriptors);
    return descriptors;
}

// 4. surf
cv::Mat FeatureTool::extractSURFFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{

    points.clear();
    cv::Mat descriptors;
    auto img = cv::imread(srcImage, INPUT_COLOR);

    cv::Ptr<cv::Feature2D> surf = cv::xfeatures2d::SIFT::create();
    surf->detectAndCompute(img, cv::noArray(), points, descriptors);
    return descriptors;
}

// 5. harris
cv::Mat FeatureTool::extractHarrisFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points,
    int threshod)
{
    cv::Mat dst, normDst;
    cv::Mat img = cv::imread(srcImage, INPUT_COLOR);
    dst = cv::Mat::zeros(img.size(), CV_32FC1);
    cv::cornerHarris(img, dst, 2, 3, 0.04);
    cv::normalize(dst, dst, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());

    return cv::Mat();
}

// match

std::vector<std::vector<cv::DMatch>> FeatureTool::matchPointsByFlann(
    const cv::Mat &qDescriptor,
    const cv::Mat &objDescriptor,
    const FeatureTool::FeatureType type)
{
    cv::FlannBasedMatcher matcher;
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(qDescriptor, objDescriptor, matches, 2);
    return matches;
}

std::vector<std::vector<cv::DMatch>> FeatureTool::matchPointsByBF(
    const cv::Mat &qDescriptor,
    const cv::Mat &objDescriptor,
    const FeatureTool::FeatureType type)
{
    auto distanceType = cv::NORM_L2;
    if (type == AKAZE)
        distanceType = cv::NORM_HAMMING;
    cv::BFMatcher matcher(distanceType);

    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(qDescriptor, objDescriptor, matches, 2);
    return matches;
}

// filter

std::vector<cv::DMatch> FeatureTool::getGoodMatches(const std::vector<std::vector<cv::DMatch>> &matches)
{
    // Lowe's raw feature
    std::vector<cv::DMatch> goodMatches;
    for (size_t i = 0; i < matches.size(); ++i) {
        if (matches[i][0].distance < NN_MATCH_RATIO * matches[i][1].distance){
            goodMatches.push_back(matches[i][0]);
        }
    }
    return goodMatches;
}

// ransac
std::vector<MatchedPoint> FeatureTool::findInliersByRANSAC(
        const std::vector<cv::KeyPoint> &qKeypoints,
        const std::vector<cv::KeyPoint> &objKeypoints,
        const std::vector<cv::DMatch> &matches)
{
    // get the location of keyPoints
    std::vector<float> distances;
    std::vector<cv::Point2f> queryCoord;
    std::vector<cv::Point2f> objectCoord;

    for (auto match : matches) {
        distances.push_back(match.distance);
        queryCoord.push_back(qKeypoints[match.queryIdx].pt);
        objectCoord.push_back(objKeypoints[match.trainIdx].pt);
    }
    
    // get homography / fundamental matrix
    cv::Mat mask;
    std::vector<float> distanceInliers;
    std::vector<cv::Point2f> queryInliers;
    std::vector<cv::Point2f> sceneInliers;
    cv::Mat H = findFundamentalMat(queryCoord, objectCoord, mask, cv::FM_RANSAC);
    // cv::Mat H = findHomography( queryCoord, objectCoord, cv::RANSAC, 10, mask);
    int inliers_cnt = 0, outliers_cnt = 0;
    for (size_t j = 0; j < mask.rows; ++j){
        if (mask.at<uchar>(j) == 1){
            queryInliers.push_back(queryCoord[j]);
            sceneInliers.push_back(objectCoord[j]);
            distanceInliers.push_back(distances[j]);
            inliers_cnt++;
        }else {
            outliers_cnt++;
        }
    }
    
    return matches2MatchedPoints(queryInliers, sceneInliers, distances);
}

void FeatureTool::writeMatchedPointsIntoFile(const std::vector<MatchedPoint> &mps, const std::string &name)
{
    if (name.size() == 0)
        std::cerr << "Error to save result!" << std::endl;
    std::ofstream of(name, std::iostream::out);
    for (auto mp: mps) {
        of << mp.p1.x << " " << mp.p1.y << " " << mp.p2.x << " " << mp.p2.y << " " << mp.distance << std::endl;
    }
    of.close();
}
// draw matched points
// paras: image1 image2 keyPoints-of-image1 keyPoints-of-image2
void FeatureTool::drawMatch(
        const std::string &src,
        const std::string &obj,
        const std::vector<MatchedPoint> &mps,
        const std::string &windowName){
    cv::Mat srcColorImage = cv::imread(src);
    cv::Mat dstColorImage = cv::imread(obj);
    // Create a image for displaying mathing keypoints
    cv::Size sz = cv::Size(srcColorImage.size().width + dstColorImage.size().width,
            srcColorImage.size().height + dstColorImage.size().height);
    cv::Mat matchingImage = cv::Mat::zeros(sz, CV_8UC3);
    
    // Draw camera frame
    cv::Mat roi1 = cv::Mat(matchingImage, cv::Rect(0, 0, srcColorImage.size().width, srcColorImage.size().height));
    srcColorImage.copyTo(roi1);
    // Draw original image
    cv::Mat roi2 = cv::Mat(matchingImage, cv::Rect(srcColorImage.size().width, srcColorImage.size().height,
                dstColorImage.size().width, dstColorImage.size().height));
    dstColorImage.copyTo(roi2);
    
    // Draw line between nearest neighbor pairs
    std::cout << "Drawing Points: the Numsber of Matched Points is: " << mps.size() << std::endl;

    cv::RNG rng(time(0));
    for (auto mp : mps) {
        cv::Point2f pt1 = mp.p1;
        cv::Point2f pt2 = mp.p2;
        cv::Point2f from = pt1;
        cv::Point2f to   = cv::Point(srcColorImage.size().width + pt2.x, srcColorImage.size().height + pt2.y);
        cv::line(matchingImage, from, to, cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255)));
    }
    // Display mathing image
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::imshow(windowName, matchingImage);
    cv::waitKey(0);
}

void FeatureTool::drawKeypoints(
    const std::string &image,
    const std::string &name,
    const std::vector<cv::KeyPoint> &kps)
{
    cv::Mat copy = cv::imread(image);
    cv::RNG rng(time(0));
    srand((unsigned)time(NULL));
    for (auto kp : kps) {
        auto c1 = rng.uniform(0,255);
        auto c2 = rng.uniform(0,255);
        auto c3 = rng.uniform(0,255);
        int r = rand() % 10 + 8;
        cv::circle(copy, kp.pt, r, cv::Scalar(c1, c2, c3), 2);
    }
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::imshow(name, copy);
    cv::waitKey(0);
}
void FeatureTool::sortPointsByDistance(std::vector<MatchedPoint> &points)
{
    sort(points.begin(), points.end(), 
    [](MatchedPoint p1, MatchedPoint p2) { return p1.distance < p2.distance; });
}

std::vector<MatchedPoint> FeatureTool::matches2MatchedPoints(
    const std::vector<cv::KeyPoint> &qKeypoints,
    const std::vector<cv::KeyPoint> &objKeypoints,
    const std::vector<cv::DMatch> &matches)
{
    std::vector<MatchedPoint> result;
    for (auto match : matches) {
        result.push_back(MatchedPoint(qKeypoints[match.queryIdx].pt, objKeypoints[match.trainIdx].pt, match.distance));
    }
    return result;
}

std::vector<MatchedPoint> FeatureTool::matches2MatchedPoints(
    const std::vector<cv::Point2f> &queryInliers,
    const std::vector<cv::Point2f> &sceneInliers,
    const std::vector<float> &distances)
{
    std::vector<MatchedPoint> result;
    for (int i = 0; i < queryInliers.size(); ++i) {
        result.push_back(MatchedPoint(queryInliers[i], sceneInliers[i], distances[i]));
    }
    return result;
}
