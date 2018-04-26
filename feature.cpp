#include "feature.h"

/* api */
std::vector<MatchedPoint> FeatureTool::getMatchedPoints(
        const std::string &srcImage,
        const std::string &objImage)
{
    cv::Mat qDescriptor;
    std::vector<cv::KeyPoint> qKeypoints;
    qDescriptor = extractSIFTFeature(srcImage, qKeypoints);
    
    cv::Mat objDescriptor;
    std::vector<cv::KeyPoint> objKeypoints;
    objDescriptor = extractSIFTFeature(objImage, objKeypoints);

    // auto points = matchPointsByRawFeature(qDescriptor, qKeypoints, srcImage, objDescriptor, objKeypoints, objImage);
    auto matches = matchPointsByFlann(qDescriptor, qKeypoints, srcImage, objDescriptor, objKeypoints, objImage);

    auto goodMatches = getGoodMatches(matches);
    drawMatch(srcImage, objImage, matches2MatchedPoints(qKeypoints, objKeypoints, goodMatches), "Origin");

    auto points = findInliersByRANSAC(qKeypoints, objKeypoints, goodMatches);

    drawMatch(srcImage, objImage, points, "After");
    cv::waitKey(6000);

    sortPointsByDistance(points);

    return points;
}

// sift
cv::Mat FeatureTool::extractSIFTFeature(
    const std::string &srcImage, 
    std::vector<cv::KeyPoint> &points)
{
    auto img = cv::imread(srcImage, INPUT_COLOR);
    cv::Ptr<cv::Feature2D> f2d = cv::xfeatures2d::SIFT::create();
    std::vector<cv::KeyPoint> keypoints;
    f2d->detect(img, keypoints);
    cv::Mat descriptors;
    if (!keypoints.size()) {
        return cv::Mat();
    }
    // extract sift feature
    f2d->compute(img, keypoints, descriptors);
    //points.resize(keypoints.size());
    for (int i = 0; i < keypoints.size(); i++){
        points.push_back(keypoints[i]);
    }
    return descriptors;
}

// 2. kaze
cv::Mat extractKAZEFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{
    return cv::Mat();
}
// 3. akaze
cv::Mat extractAKAZEFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{
    return cv::Mat();
}
// 4. surf
cv::Mat extractSURFFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{
    return cv::Mat();
}
// 5. harris
cv::Mat extractHarrisFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points)
{
    return cv::Mat();
}

// match

std::vector<std::vector<cv::DMatch>> FeatureTool::matchPointsByFlann(
    const cv::Mat &qDescriptor,
    const std::vector<cv::KeyPoint> &qKeypoints,
    const std::string &srcImage,
    const cv::Mat &objDescriptor,
    const std::vector<cv::KeyPoint> &objKeypoints,
    const std::string &objImage)
{
    cv::FlannBasedMatcher matcher;
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(qDescriptor, objDescriptor, matches, 2);
    return matches;
}

std::vector<std::vector<cv::DMatch>> FeatureTool::matchPointsByBF(
    const cv::Mat &qDescriptor,
    const std::vector<cv::KeyPoint> &qKeypoints,
    const std::string &srcImage,
    const cv::Mat &objDescriptor,
    const std::vector<cv::KeyPoint> &objKeypoints,
    const std::string &objImage)
{
    cv::BFMatcher matcher;
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(qDescriptor, objDescriptor, matches, 2);
    return matches;
}

// filter

std::vector<cv::DMatch> FeatureTool::getGoodMatches(const std::vector<std::vector<cv::DMatch>> &matches)
{
    // Lowe's raw feature
    std::vector<cv::DMatch> goodMatches;
    for (size_t i = 0; i < matches.size(); i++){
        if (matches[i][0].distance < 0.8 * matches[i][1].distance){
            goodMatches.push_back(matches[i][0]);
        }
    }
    return goodMatches;
}

// get the matched points
// paras: keyPoints-of-image1 keyPoints-of-image2 matches-info name-of-image1 name-of-image2
// return: matched points <x1, y1, x2, y2>
std::vector<MatchedPoint> FeatureTool::findInliersByRANSAC(
        const std::vector<cv::KeyPoint> &qKeypoints,
        const std::vector<cv::KeyPoint> &objKeypoints,
        const std::vector<cv::DMatch> &matches)
{
    // get the location of keyPoints
    std::vector<float> distances;
    std::vector<cv::Point2f> queryCoord;
    std::vector<cv::Point2f> objectCoord;

    for( int i = 0; i < matches.size(); i++){
        distances.push_back(matches[i].distance);
        queryCoord.push_back((qKeypoints[matches[i].queryIdx]).pt);
        objectCoord.push_back((objKeypoints[matches[i].trainIdx]).pt);
    }
    
    // homography matrix
    cv::Mat mask;
    std::vector<float> distanceInliers;
    std::vector<cv::Point2f> queryInliers;
    std::vector<cv::Point2f> sceneInliers;
    cv::Mat H = findFundamentalMat(queryCoord, objectCoord, mask, cv::FM_RANSAC);
    //cv::Mat H = findHomography( queryCoord, objectCoord, CV_RANSAC, 10, mask);
    int inliers_cnt = 0, outliers_cnt = 0;
    for (int j = 0; j < mask.rows; j++){
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
    std::cout << "the Numsber of Matched Points is: " << mps.size() << std::endl;

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
    cv::imwrite("/home/z/"+windowName+".jpg", matchingImage);
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