#include "feature.h"

vector<MatchedPoint> getMatchedPoints(
        const string &srcImage,
        const string &objImage)
{
    Mat qDescriptor;
    vector<KeyPoint> qKeypoints;
    qDescriptor = extractSIFTFeature(srcImage, qKeypoints);
    
    Mat objDescriptor;
    vector<KeyPoint> objKeypoints;
    objDescriptor = extractSIFTFeature(objImage, objKeypoints);

#ifdef FLANN
    auto points = matchePointsByFlann(qDescriptor, qKeypoints, srcImage, objDescriptor, objKeypoints, objImage);
#elif BF
    auto points = matchePointsByBF(qDescriptor, qKeypoints, srcImage, objDescriptor, objKeypoints, objImage);
#else 
    auto points = matchPointsByRawFeature(qDescriptor, qKeypoints, srcImage, objDescriptor, objKeypoints, objImage);
#endif

    sortPointsByDistance(points);
    return points;
}

// exttract sift features
Mat extractSIFTFeature(const string &imgfn, vector<KeyPoint> &points)  {
    auto img = imread(imgfn, INPUT_COLOR);
    cv::Ptr<cv::Feature2D> f2d = cv::xfeatures2d::SIFT::create();
    vector<KeyPoint> keypoints;
    f2d->detect(img, keypoints);
    Mat descriptors;
    if (!keypoints.size()) {
        return Mat();
    }
    // extract sift feature
    f2d->compute(img, keypoints, descriptors);
    //points.resize(keypoints.size());
    for (int i = 0; i < keypoints.size(); i++){
        points.push_back(keypoints[i]);
    }
    return descriptors;
}


// get the matched points
// paras: keyPoints-of-image1 keyPoints-of-image2 matches-info name-of-image1 name-of-image2
// return: matched points <x1, y1, x2, y2>; see details in the sift.h
vector<MatchedPoint> findInliers(
        vector<KeyPoint> &qKeypoints,
        vector<KeyPoint> &objKeypoints,
        vector<DMatch> &matches,
        const string &imgfn,
        const string &objFileName){
    // get the location of keyPoints
    vector<float> distances;
    vector<Point2f> queryCoord;
    vector<Point2f> objectCoord;
    vector<MatchedPoint> result;

    for( int i = 0; i < matches.size(); i++){
        queryCoord.push_back((qKeypoints[matches[i].queryIdx]).pt);
        objectCoord.push_back((objKeypoints[matches[i].trainIdx]).pt);
        distances.push_back(matches[i].distance);
    }
#ifdef _SHOW_IMAGE_
    // show matched image
    drawMatch(imgfn, objFileName, queryCoord, objectCoord, "Origin");
#endif
    // cal homography matrix
    Mat mask;
    vector<float> distanceInliers;
    vector<Point2f> queryInliers;
    vector<Point2f> sceneInliers;
    Mat H = findFundamentalMat(queryCoord, objectCoord, mask, CV_FM_RANSAC);
    //Mat H = findHomography( queryCoord, objectCoord, CV_RANSAC, 10, mask);
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
    // result for return
    for (int i = 0; i < queryInliers.size(); ++i) {
        result.push_back(MatchedPoint(queryInliers[i], sceneInliers[i], distanceInliers[i]));
    }
#ifdef _SHOW_IMAGE_
    drawMatch(imgfn, objFileName, queryInliers, sceneInliers, "AfterDeleteIncorrectPoints");
#endif
    return result;
}

// draw matched points
// paras: image1 image2 keyPoints-of-image1 keyPoints-of-image2
void drawMatch(
        const string &src,
        const string &obj,
        vector<Point2f> &srcPoints,
        vector<Point2f> &dstPoints,
        const string &windowName){
    // https://gist.github.com/thorikawa/3398619
    Mat srcColorImage = imread(src);
    Mat dstColorImage = imread(obj);
    
    // Create a image for displaying mathing keypoints
    Size sz = Size(srcColorImage.size().width + dstColorImage.size().width,
            srcColorImage.size().height + dstColorImage.size().height);
    Mat matchingImage = Mat::zeros(sz, CV_8UC3);
    
    // Draw camera frame
    Mat roi1 = Mat(matchingImage, Rect(0, 0, srcColorImage.size().width, srcColorImage.size().height));
    srcColorImage.copyTo(roi1);
    // Draw original image
    Mat roi2 = Mat(matchingImage, Rect(srcColorImage.size().width, srcColorImage.size().height,
                dstColorImage.size().width, dstColorImage.size().height));
    dstColorImage.copyTo(roi2);
    
    // Draw line between nearest neighbor pairs
    cout << "the Numsber of Matched Points is: " << srcPoints.size() << endl;

    cv::RNG rng(time(0));
    for (int i = 0; i < (int)srcPoints.size(); ++i) {
        Point2f pt1 = srcPoints[i];
        Point2f pt2 = dstPoints[i];
        Point2f from = pt1;
        Point2f to   = Point(srcColorImage.size().width + pt2.x, srcColorImage.size().height + pt2.y);
        line(matchingImage, from, to, Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255)));
    }
    // Display mathing image
    namedWindow(windowName, WINDOW_NORMAL);
    imshow(windowName, matchingImage);
    cvWaitKey(0);
}

void writeIntoFile(const vector<MatchedPoint> &mps, const string &name)
{
    ofstream of(name, iostream::out);
    for (auto mp: mps) {
        of << mp.p1.x << " " << mp.p1.y << " " << mp.p2.x << " " << mp.p2.y << " " << mp.distance << endl;
    }
    of.close();
}


vector<MatchedPoint> matchPointsByFlann(
    Mat &qDescriptor,
    vector<KeyPoint> &qKeypoints,
    const string &srcImage,
    Mat &objDescriptor,
    vector<KeyPoint> &objKeypoints,
    const string &objImage)
{
    FlannBasedMatcher matcher;
    vector< DMatch > matches;
    matcher.match(qDescriptor, objDescriptor, matches);
    return findInliers(qKeypoints, objKeypoints, matches, srcImage, objImage);
}

vector<MatchedPoint> matchPointsByBF(
    Mat &qDescriptor,
    vector<KeyPoint> &qKeypoints,
    const string &srcImage,
    Mat &objDescriptor,
    vector<KeyPoint> &objKeypoints,
    const string &objImage)
{
    BFMatcher matcher;
    vector<cv::DMatch> matches;
    matcher.match(qDescriptor, objDescriptor, matches);
    return findInliers(qKeypoints, objKeypoints, matches, srcImage, objImage);
}

vector<MatchedPoint> matchPointsByRawFeature(
    Mat &qDescriptor,
    vector<KeyPoint> &qKeypoints,
    const string &srcImage,
    Mat &objDescriptor,
    vector<KeyPoint> &objKeypoints,
    const string &objImage)
{
    // 使用Lowe的raw feature方法匹配
    FlannBasedMatcher matcher;
    vector<Point2f> qeK, obK;
    vector<vector<DMatch>> matches;
    vector<DMatch> goodMatches;
    matcher.knnMatch(qDescriptor, objDescriptor, matches, 2);
    for (size_t i = 0; i < matches.size(); i++){
        if (matches[i][0].distance < 0.8 * matches[i][1].distance){
            goodMatches.push_back(matches[i][0]);
            qeK.push_back(qKeypoints[matches[i][0].queryIdx].pt);
            obK.push_back(objKeypoints[matches[i][1].trainIdx].pt);
        }
    }
    return findInliers(qKeypoints, objKeypoints, goodMatches, srcImage, objImage); // 通过Lowe匹配的点对
}


void sortPointsByDistance(vector<MatchedPoint> &points)
{
    sort(points.begin(), points.end(), 
    [](MatchedPoint p1, MatchedPoint p2) { return p1.distance < p2.distance; });
}
