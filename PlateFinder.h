#ifndef PLATE_FINDER_H
#define PLATE_FINDER_H

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include "Main.h"
#include "Preparation.h"
#include "PlateCandidate.h"
#include "CharCandidate.h"
#include "CharRecogniser.h"

const double PLATE_WIDTH_PADDING = 1.3;
const double PLATE_HEIGHT_PADDING = 1.5;


vector<PlateCandidate> findAllPlates(cv::Mat &inputOriginal);
vector<CharCandidate> getCharCandidates(cv::Mat &imgThresh);
PlateCandidate getPlate(cv::Mat &inputOriginal, vector<CharCandidate> &vectorOfMatchingChars);

# endif

