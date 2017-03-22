#ifndef CHAR_CANDIDATE_H
#define CHAR_CANDIDATE_H

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace std;

class CharCandidate {
public:
    vector<cv::Point> contour;
    cv::Rect boundingRect;
    int charCenterX;
    int charCenterY;
    double diagSize;
    double aspectRatio;
	CharCandidate(vector<cv::Point> _contour);
	bool operator == (const CharCandidate& otherCandidate) const;
	bool operator != (const CharCandidate& otherCandidate) const;
    static bool sortLeftToRight(const CharCandidate &left, const CharCandidate & right) {
        return(left.charCenterX < right.charCenterX);
    }
};
#endif


