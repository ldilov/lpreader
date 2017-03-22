#include "PlateFinder.h"
using namespace std;

vector<PlateCandidate> findAllPlates(cv::Mat &inputOriginal) {
    vector<PlateCandidate> vectorOfPlateCandidates;
	cv::Mat grayscaled;
    cv::Mat thresholded;
    cv::Mat imageOfContours(inputOriginal.size(), CV_8UC3, SCALAR_BLACK);

    cv::RNG rng;

    cv::destroyAllWindows();
	preparation(inputOriginal, grayscaled, thresholded); 

	//�������� ������ ��������� �� ������� - ��������� ��������� � ���������� ���� ����, ����� ����������� �����
    vector<CharCandidate> vectorOfCharCandidatesInScene = getCharCandidates(thresholded);

	//�������� ����� �� ������� � �� �������� �� �� ���������� ���� �����    
    vector<vector<CharCandidate> > listOfMatchingCharsVectors = findListOfMatchingCharsVectors(vectorOfCharCandidatesInScene);

	//��������� �������� �� ������ �������, �� ����� ����� ����������
    for (auto &vectorOfMatchingChars : listOfMatchingCharsVectors) {
        PlateCandidate PlateCandidate = getPlate(inputOriginal, vectorOfMatchingChars);

        if (PlateCandidate.plate.empty() == false) {
			//��� � ������� �����, �������� ������ �� ��������� �� ������
            vectorOfPlateCandidates.push_back(PlateCandidate);
        }
    }

    cout << endl << "Namereni sa " << vectorOfPlateCandidates.size() << " kandidati za registracionni nomera!" << endl;

    return vectorOfPlateCandidates;
}

vector<CharCandidate> getCharCandidates(cv::Mat &thresholdedImage) {
    vector<CharCandidate> vectorOfCharCandidates;

    cv::Mat imageOfContours(thresholdedImage.size(), CV_8UC3, SCALAR_BLACK);
    int intCountOfCharCandidates = 0;

    cv::Mat thresholdedImageCopy = thresholdedImage.clone();

    vector<vector<cv::Point> > contours;

    cv::findContours(thresholdedImageCopy, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

    for (unsigned int i = 0; i < contours.size(); i++) {

		CharCandidate CharCandidate(contours[i]);

		//��� ������� � ���������� ������
        if (isPossibleChar(CharCandidate)) {
            intCountOfCharCandidates++;
            vectorOfCharCandidates.push_back(CharCandidate);
        }
    }


    return vectorOfCharCandidates;
}

PlateCandidate getPlate(cv::Mat &inputOriginal, vector<CharCandidate> &vectorOfMatchingChars) {
    PlateCandidate PlateCandidate;

    //��������� ������ ������� ���������
    sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), CharCandidate::sortLeftToRight);

    //�������� ������� �� ���������������� �����
	// � - (� �� ������ ������ + � �� ���������)/2
    double centerPointX = (double)(vectorOfMatchingChars[0].charCenterX + vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].charCenterX) / 2.0;
	//Y - (y �� ������ ������ + y �� ���������) / 2
    double centerPointY = (double)(vectorOfMatchingChars[0].charCenterY + vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].charCenterY) / 2.0;
    cv::Point2d centerPoint(centerPointX, centerPointY);

    //������� ��������� �� ���. �����
    int pltWidth = (int)((vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].boundingRect.x + vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].boundingRect.width - vectorOfMatchingChars[0].boundingRect.x) * PLATE_WIDTH_PADDING);

    double sumOfCharHeights = 0;
    for (auto &matchingChar : vectorOfMatchingChars) {
        sumOfCharHeights = sumOfCharHeights + matchingChar.boundingRect.height;
    }
    double avgCharHeight = (double)sumOfCharHeights / vectorOfMatchingChars.size();

    int pltHeight = (int)(avgCharHeight * PLATE_HEIGHT_PADDING);

    //vzimame ugula na zavurtane
    double srehtulejashtKatet = fabs(vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].charCenterY - vectorOfMatchingChars[0].charCenterY); //
    double hipotenuza = charDistance(vectorOfMatchingChars[0], vectorOfMatchingChars[vectorOfMatchingChars.size() - 1]);
    double radianAngle = asin(srehtulejashtKatet / hipotenuza); //sin(x) = r , r = hyp/katet, r : [-1;1] => arcsin(r) = x, x-ugul, asin() vrushta x v radiani
    double degreeAngle = radianAngle * (180.0 / CV_PI); //1rad = 180/P
    PlateCandidate.plateLocation = cv::RotatedRect(centerPoint, cv::Size2f((float)pltWidth, (float)pltHeight), (float)degreeAngle); //slagame rotate-natiq pravougulnik

	//������� �� ���������
    cv::Mat rotationMatrix;
    cv::Mat rotated;
    cv::Mat cropped;

	//������� ��������� �� ��������� � ���������, ��������� � rotated
    rotationMatrix = cv::getRotationMatrix2D(centerPoint, degreeAngle, 1.0);
    cv::warpAffine(inputOriginal, rotated, rotationMatrix, inputOriginal.size());
    //��������� ���. ����� � �� ������� � cropped
    cv::getRectSubPix(rotated, PlateCandidate.plateLocation.size, PlateCandidate.plateLocation.center, cropped);

    PlateCandidate.plate = cropped;

    return PlateCandidate;
}





