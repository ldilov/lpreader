#include "CharCandidate.h"
using namespace std;

CharCandidate::CharCandidate(vector<cv::Point> InputContour) {
    contour = InputContour;

    boundingRect = cv::boundingRect(contour);

    charCenterX = (boundingRect.x + boundingRect.x + boundingRect.width) / 2;
    charCenterY = (boundingRect.y + boundingRect.y + boundingRect.height) / 2;

    diagSize = sqrt(pow(boundingRect.width, 2) + pow(boundingRect.height, 2)); //за правоъгълник със страни а и b, диагноала е d = sqrt(a^2 + b^2) от питагор. т-ма
    aspectRatio = (float)boundingRect.width / (float)boundingRect.height; //отношениято ширина : височина
}

bool CharCandidate::operator == (const CharCandidate& otherCandidate) const {
	if (this->contour == otherCandidate.contour) return true;
	else return false;
}

bool CharCandidate::operator != (const CharCandidate& otherCandidate) const {
	if (this->contour != otherCandidate.contour) return true;
	else return false;
}