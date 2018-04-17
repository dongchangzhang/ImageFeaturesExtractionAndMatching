#!/bin/python

import cv2

im1 = cv2.imread("2.JPG", 0)
im2 = cv2.imread("3.JPG", 0)

with open("output.txt", "r") as f:
    for line in f:
        info = line.split(" ")
        x1, y1, x2, y2 = set(info)
        cv2.circle(im1, (int(x1), int(y1)), 5, (0, 0, 255), -1)
        cv2.circle(im2, (int(x2), int(y2)), 5, (0, 0, 255), -1)

cv2.namedWindow("2", WINDOW_NORMAL)
cv2.namedWindow("3", WINDOW_NORMAL)
cv2.imshow("2", im1)
cv2.imshow("3", im2)


