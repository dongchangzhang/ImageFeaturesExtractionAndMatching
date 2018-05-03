#include "feature.h"

/* api */
std::vector<MatchedPoint> FeatureTool::getMatchedPoints(
        const std::string &srcImage,
        const std::string &objImage,
        FeatureTool::FeatureType type,
        FeatureTool::Direction direction)
{
    cv::Mat firstDesc;
    cv::Mat secondDesc;
    std::vector<cv::KeyPoint> firstKps;
    std::vector<cv::KeyPoint> secondKps;

    cv::Ptr<cv::Feature2D> fp;

    int ok0 = init(fp, type);
    int ok1 = extractFeature(srcImage, fp, firstKps, firstDesc);
    int ok2 = extractFeature(objImage, fp, secondKps, secondDesc);

    if (ok0 == -1 || ok1 == -1 || ok2 == -1) {
        return std::vector<MatchedPoint>();
    }
    // match
    auto matches = matchPointsByBF(firstDesc, secondDesc, type);

    // good matches
    auto goodMatches = getGoodMatches(matches);

    // ransac
    auto matchedPoints = findInliersByRANSAC(firstKps, secondKps, goodMatches);

#ifdef _SHOW_IMAGE_AND_LOG_
    std::cout << "the number of feature(image1): " << firstKps.size() << std::endl;
    std::cout << "the number of feature(image2): " << secondKps.size() << std::endl;
    std::cout << "the number of good matches(by distance): " << goodMatches.size() << std::endl;
    drawMatch(srcImage, objImage, matches2MatchedPoints(firstKps, secondKps, goodMatches), "by distance");
    std::cout << "the number of good matches(by dist + ransac): " << matchedPoints.size() << std::endl;
    drawMatch(srcImage, objImage, matchedPoints, "by distance + ransac");
#endif
    // result  1
    sortPointsByDistance(matchedPoints);
    // both direction ?
    if (direction == BOTH) {
        auto rmatches = matchPointsByBF(secondDesc, firstDesc, type);
        auto rgoodMatches = getGoodMatches(rmatches);
        auto rmatchedPoints = findInliersByRANSAC(secondKps, firstKps, rgoodMatches);
        // result  2
        sortPointsByDistance(rmatchedPoints);
        // filter
        matchSymmetric(matchedPoints, rmatchedPoints, matchedPoints);
#ifdef _SHOW_IMAGE_AND_LOG_
    std::cout << "the number of good matches(by dist + ransac + both direction): " << matchedPoints.size() << std::endl;
    drawMatch(srcImage, objImage, matchedPoints, "by distance + ransac + both direction");
#endif
    }
    // result
    return matchedPoints;
}

int FeatureTool::init(cv::Ptr<cv::Feature2D> &fp, FeatureTool::FeatureType type)
{
    switch (type) {
        case SIFT:
            fp = cv::xfeatures2d::SIFT::create();
            break;
        case SURF:
            fp = cv::xfeatures2d::SURF::create();
            break;
        case KAZE:
            fp = cv::KAZE::create();
            break;
        case AKAZE:
            fp = cv::AKAZE::create();
            break;
        default:
            std::cerr << "Error: Can not init cv::Ptr" << std::endl;
            return -1;
    }
    return 0;
}
int FeatureTool::extractFeature(
    const std::string &image,
    const cv::Ptr<cv::Feature2D> &fp,
    std::vector<cv::KeyPoint> &kps,
    cv::Mat &desc)
{
    auto img = cv::imread(image, INPUT_COLOR);
    if (img.empty()) {
        std::cerr << "Error: open image failed: " << image << std::endl;
        return -1;
    }
    fp->detectAndCompute(img, cv::noArray(), kps, desc);
    return 0;
}
// 5. harris
cv::Mat FeatureTool::extractHarrisFeature(
    const std::string &srcImage,
    std::vector<cv::KeyPoint> &points,
    int thresh)
{
    // cv::Mat dst, normDst, scaleDst;
    cv::Mat descriptors;
    cv::Mat img = cv::imread(srcImage, INPUT_COLOR);
    cv::Mat show = cv::imread(srcImage);
    // dst = cv::Mat::zeros(img.size(), CV_32FC1);
    std::vector<cv::Point2f> corners;

    // cv::cornerHarris(img, dst, 2, 3, 0.04);
    // cv::normalize(dst, normDst, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
    // cv::convertScaleAbs(normDst, scaleDst);
    // for (int j = 0; j < normDst.rows; ++j) {
    //     for (int i = 0; i < normDst.cols; ++i) {
    //         if ((int)normDst.at<float>(j, i) > thresh + 90) {
    //             std::cout << "harris: [" << i << ", " << j << "]" << std::endl;
    //             corners.push_back(cv::Point2f(i, j));
    //             // cv::circle(show, cv::Point(i, j), 5, cv::Scalar(0, 0, 255), 2, 8, 0);
    //         }
    //     }
    // }

    // paras
    double qualityLevel = 0.01;
    double minDistance = 10;
    int blockSize = 3;
    double k = 0.04;
    cv::goodFeaturesToTrack(img, corners, 10000, qualityLevel, minDistance, cv::Mat(), blockSize, true, k);

    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 40, 0.001);
    cv::cornerSubPix(img, corners, cv::Size(5, 5), cv::Size(-1, -1), criteria);
    // for (auto corner : corners) {
    //     std::cout << "harris: [" << corner.x << ", " << corner.y << "]" << std::endl;
    //     cv::circle(show, corner, 5, cv::Scalar(0, 0, 255), 2, 8, 0);
    // }
    cv::KeyPoint::convert(corners, points, 1, 1, 0, -1);
    cv::Ptr<cv::Feature2D> sift = cv::xfeatures2d::SIFT::create();
    sift->compute(img, points, descriptors);
    return descriptors;
}

// match
std::vector<std::vector<cv::DMatch>> FeatureTool::matchPointsByFlann(
    const cv::Mat &qDesc,
    const cv::Mat &objDesc,
    const FeatureTool::FeatureType type)
{
    cv::FlannBasedMatcher matcher;
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(qDesc, objDesc, matches, 2);
    return matches;
}

std::vector<std::vector<cv::DMatch>> FeatureTool::matchPointsByBF(
    const cv::Mat &qDesc,
    const cv::Mat &objDesc,
    const FeatureTool::FeatureType type)
{
    auto distanceType = cv::NORM_L2;
    if (type == AKAZE)
        distanceType = cv::NORM_HAMMING;
    cv::BFMatcher matcher(distanceType);

    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(qDesc, objDesc, matches, 2);
    return matches;
}

// filter
//
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
    if (matches.size() == 0) return std::vector<MatchedPoint>();
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
    for (size_t j = 0; j < mask.rows; ++j){
        if (mask.at<uchar>(j) == 1){
            queryInliers.push_back(queryCoord[j]);
            sceneInliers.push_back(objectCoord[j]);
            distanceInliers.push_back(distances[j]);
        }
    }
    return matches2MatchedPoints(queryInliers, sceneInliers, distances);
}

size_t FeatureTool::matchSymmetric(
    const std::vector<MatchedPoint> &matchedPoint,
    const std::vector<MatchedPoint> &rmatchedPoint,
    std::vector<MatchedPoint> &result)
{
    std::vector<MatchedPoint> r;
    std::unordered_map<cv::Point, std::vector<cv::Point>, hashFunc, cmpFun> record;
    for (auto p : rmatchedPoint) {
        cv::Point from((int)p.p2.x, (int)p.p2.y);
        cv::Point to((int)p.p1.x, (int) p.p1.y);
        if (record.find(from) != record.end()) {
            record[from].push_back(to);
        } else {
            std::vector<cv::Point> tmp{to};
            record[from] = tmp;
        }
    }
    for (auto p : matchedPoint) {
        cv::Point from((int)p.p1.x, (int) p.p1.y);
        cv::Point to((int)p.p2.x, (int) p.p2.y);
        if (record.find(from) != record.end()) {
            for (auto tmp : record[from]) {
                if (to == tmp) {
                    r.push_back(p);
                    break;
                }
            }
        } 
    }
    result = r;
    return result.size();
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
        const std::string &windowName,
        const int type) 
{
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

    srand((unsigned)time(NULL));
    cv::RNG rng(time(0));
    for (auto mp : mps) {
        cv::Point2f pt1 = mp.p1;
        cv::Point2f pt2 = mp.p2;
        cv::Point2f from = pt1;
        cv::Point2f to   = cv::Point(srcColorImage.size().width + pt2.x, srcColorImage.size().height + pt2.y);
        auto c1 = rng.uniform(10,255);
        auto c2 = rng.uniform(10,255);
        auto c3 = rng.uniform(10,255);
        if (type == 0)
            cv::line(matchingImage, from, to, cv::Scalar(0, 0, 255));
        else {
            int r = rand() % 10 + 8;
            cv::circle(matchingImage, from, r, cv::Scalar(c1, c2, c3), 2);
            cv::circle(matchingImage, to, r, cv::Scalar(c1, c2, c3), 2);
        }
    }
    // Display mathing image
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::imshow(windowName, matchingImage);
    cv::waitKey(0);
    cv::destroyAllWindows();
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
