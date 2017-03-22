#ifndef PREPARATION_H
#define PREPARATION_H

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

// Константи
const int ADAPTIVE_THRESH_BLOCK_SIZE = 19;
const int ADAPTIVE_THRESH_WEIGHT = 9;

void preparation(cv::Mat &inputOriginal, cv::Mat &inputGrayscale, cv::Mat &inputThresh);
cv::Mat getLightnessVal(cv::Mat &inputOriginal);
cv::Mat enhanceContrast(cv::Mat &inputGrayscale);
#endif

