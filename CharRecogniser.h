#ifndef CHAR_RECOGNISER_H
#define CHAR_RECOGNISER_H

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/ml/ml.hpp>

#include "Main.h"
#include "CharCandidate.h"
#include "PlateCandidate.h"
#include "Preparation.h"

const int MIN_PIXEL_WIDTH = 2;
const int MIN_PIXEL_HEIGHT = 8;
const double MIN_ASPECT_RATIO = 0.25;
const double MAX_ASPECT_RATIO = 1.0;
const int MIN_PIXEL_AREA = 80;
const double MIN_DIAG_SIZE_MULTIPLE_AWAY = 0.3;
const double MAX_DIAG_SIZE_MULTIPLE_AWAY = 5.0;
const double MAX_CHANGE_IN_AREA = 0.5;
const double MAX_CHANGE_IN_WIDTH = 0.8;
const double MAX_CHANGE_IN_HEIGHT = 0.2;
const double MAX_ANGLE_BETWEEN_CHARS = 12.0;
const int MIN_NUMBER_OF_MATCHING_CHARS = 3;
const int RESIZED_CHAR_IMAGE_WIDTH = 20;
const int RESIZED_CHAR_IMAGE_HEIGHT = 30;
const int MIN_CONTOUR_AREA = 100;

extern const bool blnShowSteps;
extern cv::Ptr<cv::ml::KNearest>  kNearest;


bool loadAndLearnKNN(void);

vector<PlateCandidate> getCharsInPlates(vector<PlateCandidate> &vectorOfPossiblePlates);
vector<CharCandidate> findPossibleCharsInPlate(cv::Mat &grayScaleImage, cv::Mat &thresholdedImage);

bool isPossibleChar(CharCandidate &charCandidate);

vector<vector<CharCandidate> > findListOfMatchingCharsVectors(const vector<CharCandidate> &vectorOfPossibleChars);
vector<CharCandidate> findVectorOfMatchingChars(const CharCandidate &charCandidate, const vector<CharCandidate> &vectorOfChars);

double charDistance(const CharCandidate &firstChar, const CharCandidate &secondChar);
double charsAngle(const CharCandidate &firstChar, const CharCandidate &secondChar);

vector<CharCandidate> removeInnerOverlappingChars(vector<CharCandidate> &vectorOfMatchingChars);

string recogniseCharsInPlate(cv::Mat &imgThresh, vector<CharCandidate> &vectorOfMatchingChars);


#endif
