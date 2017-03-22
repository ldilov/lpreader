#include "Preparation.h"
using namespace std;

void preparation(cv::Mat &inputOriginal, cv::Mat &inputGrayscale, cv::Mat &imgThresh) {
	cv::Mat blurredImage;
	
	inputGrayscale = getLightnessVal(inputOriginal);
    cv::Mat imgMaxContrastGrayscale = enhanceContrast(inputGrayscale);
    cv::GaussianBlur(imgMaxContrastGrayscale, blurredImage, cv::Size(5, 5), 0);
    cv::adaptiveThreshold(blurredImage, imgThresh, 255.0, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, ADAPTIVE_THRESH_BLOCK_SIZE, ADAPTIVE_THRESH_WEIGHT);
}

cv::Mat getLightnessVal(cv::Mat &inputOriginal) {
    cv::Mat imgHSV;
    vector<cv::Mat> hsvImagesVector;
    cv::Mat lightnessVal;

    cv::cvtColor(inputOriginal, imgHSV, CV_BGR2HSV);
    cv::split(imgHSV, hsvImagesVector);
    lightnessVal = hsvImagesVector[2];

    return lightnessVal;
}

cv::Mat enhanceContrast(cv::Mat &inputGrayscale) {
    cv::Mat topHat;
    cv::Mat blackHat;
    cv::Mat imgGrayscaleTopHat;
    cv::Mat imgGrayscaleResult;

    cv::Mat structuringElement = cv::getStructuringElement(CV_SHAPE_RECT, cv::Size(3, 3));
    cv::morphologyEx(inputGrayscale, topHat, CV_MOP_TOPHAT, structuringElement);
    cv::morphologyEx(inputGrayscale, blackHat, CV_MOP_BLACKHAT, structuringElement);

    imgGrayscaleTopHat = inputGrayscale + topHat;
    imgGrayscaleResult = imgGrayscaleTopHat - blackHat;

    return imgGrayscaleResult;
}



