//
//  utils.cpp
//  sift_asift_match
//
//  Created by willard on 8/21/15.
//  Copyright (c) 2015 wilard. All rights reserved.
//

#include "utils.h"

// 画匹配的特征点对
void drawMatch(const string &src, const string &obj, vector<Point2f> &srcPoints, vector<Point2f> &dstPoints){
    // https://gist.github.com/thorikawa/3398619
    Mat srcColorImage = imread(src);
    Mat dstColorImage = imread(obj);
    
    // Create a image for displaying mathing keypoints
    Size sz = Size(srcColorImage.size().width + dstColorImage.size().width, srcColorImage.size().height + dstColorImage.size().height);
    Mat matchingImage = Mat::zeros(sz, CV_8UC3);
    
    // Draw camera frame
    Mat roi1 = Mat(matchingImage, Rect(0, 0, srcColorImage.size().width, srcColorImage.size().height));
    srcColorImage.copyTo(roi1);
    // Draw original image
    Mat roi2 = Mat(matchingImage, Rect(srcColorImage.size().width, srcColorImage.size().height, dstColorImage.size().width, dstColorImage.size().height));
    dstColorImage.copyTo(roi2);
    
    // Draw line between nearest neighbor pairs
    std::cout << "-----------" << std::endl;
    cout << "Matched points: " << srcPoints.size() << endl;
    for (int i = 0; i < (int)srcPoints.size(); ++i) {
        Point2f pt1 = srcPoints[i];
        Point2f pt2 = dstPoints[i];
        Point2f from = pt1;
        Point2f to   = Point(srcColorImage.size().width + pt2.x, srcColorImage.size().height + pt2.y);
        line(matchingImage, from, to, Scalar(0, 0, 255));
    }
    // Display mathing image
    resize(matchingImage, matchingImage, Size(matchingImage.cols/2, matchingImage.rows/2));
    namedWindow( "Matched Points",WINDOW_NORMAL);
    imshow("Matched Points", matchingImage);
    cvWaitKey(0);
}

vector<MatchedPoint> findInliers(vector<KeyPoint> &qKeypoints, vector<KeyPoint> &objKeypoints, vector<DMatch> &matches, const string &imgfn, const string &objFileName){
    // 获取关键点坐标
    vector<Point2f> queryCoord;
    vector<Point2f> objectCoord;
    vector<MatchedPoint> result;
    for( int i = 0; i < matches.size(); i++){
        queryCoord.push_back((qKeypoints[matches[i].queryIdx]).pt);
        objectCoord.push_back((objKeypoints[matches[i].trainIdx]).pt);
    }
    // 使用自定义的函数显示匹配点对
    drawMatch(imgfn, objFileName, queryCoord, objectCoord);
    
    // 计算homography矩阵
    Mat mask;
    vector<Point2f> queryInliers;
    vector<Point2f> sceneInliers;
    Mat H = findFundamentalMat(queryCoord, objectCoord, mask, CV_FM_RANSAC);
    //Mat H = findHomography( queryCoord, objectCoord, CV_RANSAC, 10, mask);
    int inliers_cnt = 0, outliers_cnt = 0;
    for (int j = 0; j < mask.rows; j++){
        if (mask.at<uchar>(j) == 1){
            queryInliers.push_back(queryCoord[j]);
            sceneInliers.push_back(objectCoord[j]);
            inliers_cnt++;
        }else {
            outliers_cnt++;
        }
    }
    for (int i = 0; i < queryInliers.size(); ++i) {
        result.push_back(MatchedPoint(queryInliers[i], sceneInliers[i]));
    }
    //显示剔除误配点对后的匹配点对
    drawMatch(imgfn, objFileName, queryInliers, sceneInliers);
    return result;
}

void writeIntoFile(const vector<MatchedPoint> &mps, const string &name)
{
    ofstream of(name, iostream::out);
    for (auto mp: mps) {
        of << mp.p1.x << " " << mp.p1.y << " " << mp.p2.x << " " << mp.p2.y << endl;
    }
    of.close();
}
