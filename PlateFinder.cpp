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

	//Намираме всички кандидати за символи - извличаме контурите и филтрираме само тези, които наподобяват букви
    vector<CharCandidate> vectorOfCharCandidatesInScene = getCharCandidates(thresholded);

	//Намираме групи от символи и се опитваме да ги разпознаем като номер    
    vector<vector<CharCandidate> > listOfMatchingCharsVectors = findListOfMatchingCharsVectors(vectorOfCharCandidatesInScene);

	//Извличаме номерата за всички символи, за които имаме съвпадение
    for (auto &vectorOfMatchingChars : listOfMatchingCharsVectors) {
        PlateCandidate PlateCandidate = getPlate(inputOriginal, vectorOfMatchingChars);

        if (PlateCandidate.plate.empty() == false) {
			//ако е намерен номер, добавяме вектор от кандидати за номера
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

		//Ако контура е евентуален символ
        if (isPossibleChar(CharCandidate)) {
            intCountOfCharCandidates++;
            vectorOfCharCandidates.push_back(CharCandidate);
        }
    }


    return vectorOfCharCandidates;
}

PlateCandidate getPlate(cv::Mat &inputOriginal, vector<CharCandidate> &vectorOfMatchingChars) {
    PlateCandidate PlateCandidate;

    //сортираме отляво надясно символите
    sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), CharCandidate::sortLeftToRight);

    //намираме центъра на регистрационният номер
	// Х - (х на първия символ + х на последния)/2
    double centerPointX = (double)(vectorOfMatchingChars[0].charCenterX + vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].charCenterX) / 2.0;
	//Y - (y на първия символ + y на последния) / 2
    double centerPointY = (double)(vectorOfMatchingChars[0].charCenterY + vectorOfMatchingChars[vectorOfMatchingChars.size() - 1].charCenterY) / 2.0;
    cv::Point2d centerPoint(centerPointX, centerPointY);

    //Взимаме размерите на рег. номер
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

	//Матрица на завъртане
    cv::Mat rotationMatrix;
    cv::Mat rotated;
    cv::Mat cropped;

	//Взимаме матрицата на завъртане и завъртаме, запазваме в rotated
    rotationMatrix = cv::getRotationMatrix2D(centerPoint, degreeAngle, 1.0);
    cv::warpAffine(inputOriginal, rotated, rotationMatrix, inputOriginal.size());
    //Изрязваме рег. номер и го слагаме в cropped
    cv::getRectSubPix(rotated, PlateCandidate.plateLocation.size, PlateCandidate.plateLocation.center, cropped);

    PlateCandidate.plate = cropped;

    return PlateCandidate;
}





