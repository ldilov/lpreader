#ifndef PLATE_CANDIDATE_H
#define PLATE_CANDIDATE_H

#include <string>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace std;

class PlateCandidate {
public:
    cv::Mat plate;
    cv::Mat imgGrayscale;
    cv::Mat imgThresh;
	string strChars;
    cv::RotatedRect plateLocation;

    static bool sortDescendingByNumberOfChars(const PlateCandidate &left, const PlateCandidate &right) {
        return(left.strChars.length() > right.strChars.length());
    }

};

#endif

