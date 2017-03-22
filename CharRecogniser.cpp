#include "CharRecogniser.h"
using namespace std;

cv::Ptr<cv::ml::KNearest> kNearest = cv::ml::KNearest::create();

bool loadAndLearnKNN(void) {

    cv::Mat classificationsMatrix; // �� �������� �������� �� ��������������(���������) � ���� ���������� ���� ������
    cv::FileStorage classificationsFile("charactergroups.xml", cv::FileStorage::READ);

    if (classificationsFile.isOpened() == false) {
        cout << "greshka: lispvat klasifikaciite ot dataset-a\n\n";
        return false;
    }

	//�������� �������������� � ������������
    classificationsFile["charactergroups"] >> classificationsMatrix;
    classificationsFile.release();
	
    cv::Mat imagesFloatsMatrix; //�� �������� ����������� ������� �� training dataset-a � ���� ���������� ���� ������
    cv::FileStorage imagesFile("images.xml", cv::FileStorage::READ);

    if (imagesFile.isOpened() == false) {
        cout << "greshka: ne moje da otvori snimkite ot traindata set-a.\n\n";
        return false;
    }

	//�������� ��������� � ������������
    imagesFile["images"] >> imagesFloatsMatrix;
    imagesFile.release();

    //������ ���-�������� ����� (�=1 => nearest neighbour alg)
	kNearest->setDefaultK(1);
    kNearest->train(imagesFloatsMatrix, cv::ml::ROW_SAMPLE, classificationsMatrix);

    return true;
}

vector<PlateCandidate> getCharsInPlates(vector<PlateCandidate> &plateCadndidatesVector) {
    int intPlateCounter = 0; // �����
    cv::Mat imgContours;
    vector<vector<cv::Point> > contours;
    cv::RNG rng;

    if (plateCadndidatesVector.empty()) {
        return plateCadndidatesVector;
    }

    for (auto &plateCandidate : plateCadndidatesVector) { 		
        preparation(plateCandidate.plate, plateCandidate.imgGrayscale, plateCandidate.imgThresh); //������������� � grayscale � threshold-����

        //���������� ��������������� � 1.6 ����. = 60%. 
        cv::resize(plateCandidate.imgThresh, plateCandidate.imgThresh, cv::Size(), 1.6, 1.6);
        //�������� threshold-���
        cv::threshold(plateCandidate.imgThresh, plateCandidate.imgThresh, 0.0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);

         //�������� ��������� �� �� �������� ������� � �������� ���� ����, ����� �� �������
        vector<CharCandidate> vectorOfPossibleCharsInPlate = findPossibleCharsInPlate(plateCandidate.imgGrayscale, plateCandidate.imgThresh);
        
        //������ ������ �� ��. �������� �������, ������ ����� �� �������, �� ����� ����� "����������" (match-���)
        vector<vector<CharCandidate> > listOfVectorsOfMatchingCharsInPlate = findListOfMatchingCharsVectors(vectorOfPossibleCharsInPlate);

        if (listOfVectorsOfMatchingCharsInPlate.size() == 0) {
            plateCandidate.strChars = ""; //��������������� ���. ����� ���� �������, ��� ����� ����� "����������" 
            continue;//�������� �� �����. ��������
        }

        for (auto &vectorOfMatchingChars : listOfVectorsOfMatchingCharsInPlate) { //����� ������� �� �������, ��� ����� ����� "����������" , �� ��. �� ���:
            sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), CharCandidate::sortLeftToRight); //��������� ����-> ����� , ���� �� ��������� �� �� ��������� ������ ������ �������� �� ���� �
            vectorOfMatchingChars = removeInnerOverlappingChars(vectorOfMatchingChars); //������ ������������ �� �������
        }

        unsigned int LongestVectorOfCharsSize = 0, LongestVectorOfCharsIndex = 0;

         //�� ��. ������� ��� �������, ��� ����� ����� "����������" , �������  ������� �� ���-�������
        for (unsigned int i = 0; i < listOfVectorsOfMatchingCharsInPlate.size(); i++) {
            if (listOfVectorsOfMatchingCharsInPlate[i].size() > LongestVectorOfCharsSize) {
                LongestVectorOfCharsSize = listOfVectorsOfMatchingCharsInPlate[i].size();
                LongestVectorOfCharsIndex = i;
            }
        }
        vector<CharCandidate> biggestVectorOfMatchingCharsInPlate = listOfVectorsOfMatchingCharsInPlate[LongestVectorOfCharsIndex];

		//���������� ������������ �� ��������� ����� ���-�������� ������ �� �������, ��� ����� ����� "����������"
		plateCandidate.strChars = recogniseCharsInPlate(plateCandidate.imgThresh, biggestVectorOfMatchingCharsInPlate); 
    } //������� ���������

    return plateCadndidatesVector;
}

vector<CharCandidate> findPossibleCharsInPlate(cv::Mat &imgGrayscale, cv::Mat &imgThresh) {
    vector<CharCandidate> vectorOfPossibleChars;
    cv::Mat imgThreshCopy;
    vector<vector<cv::Point> > contours;
	imgThreshCopy = imgThresh.clone();

    cv::findContours(imgThreshCopy, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE); //�������� ������ ������� � ���. �����

    for (auto &contour : contours) {
        CharCandidate CharCandidate(contour);

        if (isPossibleChar(CharCandidate)) {
            vectorOfPossibleChars.push_back(CharCandidate); //��� � ������������ ������, �������� �� ��� ������� � ������������� �������
        }
    }

    return vectorOfPossibleChars;
}

bool isPossibleChar(CharCandidate &charCandidate) {
    //����������� ���� �������� � �������� �� ������ ������
	//���������� ��������� �� ������� �� ��������������, ����� �������� ��������������� �������, ����� � ����� ������ ��������� �� �������
    if (charCandidate.boundingRect.area() > MIN_PIXEL_AREA &&
        charCandidate.boundingRect.width > MIN_PIXEL_WIDTH && charCandidate.boundingRect.height > MIN_PIXEL_HEIGHT &&
        MIN_ASPECT_RATIO < charCandidate.aspectRatio && charCandidate.aspectRatio < MAX_ASPECT_RATIO) {
        return(true);
    } else {
        return(false);
    }
}

vector<vector<CharCandidate> > findListOfMatchingCharsVectors(const vector<CharCandidate> &vectorOfPossibleChars) {
	//������� ������ ���������� ������� � ���� ����� ������. ������������� ���� �� �� ����� ������ �� ������� �� �������, �� ����� ����� "����������"
    vector<vector<CharCandidate> > listOfVectorsOfMatchingChars;

    for (auto &charCandidate : vectorOfPossibleChars) {

         //�������� ������ ������� �� ������� ������, ����� ���� "����������" � ������� ������
        vector<CharCandidate> vectorOfMatchingChars = findVectorOfMatchingChars(charCandidate, vectorOfPossibleChars);

        vectorOfMatchingChars.push_back(charCandidate);//�������� ������ �� �������� ��� ������������ ������ ��� �������, �� ����� ��� "����������"
         
        if (vectorOfMatchingChars.size() < MIN_NUMBER_OF_MATCHING_CHARS) {
            continue; //��� �������� ������ �� �������, �� ����� ��� "����������" � ������ ����� �� �� ������� ���. �����, �� ��� �� �� ����� ������
        }
       
        listOfVectorsOfMatchingChars.push_back(vectorOfMatchingChars);

        vector<CharCandidate> vectorOfPossibleCharsWithCurrentMatchesRemoved;

        for (auto &possChar : vectorOfPossibleChars) {
            if (find(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), possChar) == vectorOfMatchingChars.end()) {
                vectorOfPossibleCharsWithCurrentMatchesRemoved.push_back(possChar);
            }
        }
        
        vector<vector<CharCandidate> > recursivelistOfVectorsOfMatchingChars;

        recursivelistOfVectorsOfMatchingChars = findListOfMatchingCharsVectors(vectorOfPossibleCharsWithCurrentMatchesRemoved);	//������� ��������

        for (auto &recursiveVectorOfMatchingChars : recursivelistOfVectorsOfMatchingChars) { 
            listOfVectorsOfMatchingChars.push_back(recursiveVectorOfMatchingChars); 
        }

        break;
    }

    return listOfVectorsOfMatchingChars;
}

vector<CharCandidate> findVectorOfMatchingChars(const CharCandidate &charCandidate, const vector<CharCandidate> &vectorOfChars) {
	vector<CharCandidate> vectorOfMatchingChars;

	for (auto &possibleMatchingChar : vectorOfChars) {

		//��� ������ ���������� �� ������ ��� ���� ��, �� �� ���������� (����� ������ �� ���)
		if (possibleMatchingChar == charCandidate) {
			continue;
		}
		// ���������� ��������� ���� �� "��������" �� ����������
		double distanceBetweenChars = charDistance(charCandidate, possibleMatchingChar);
		double angleBetweenChars = charsAngle(charCandidate, possibleMatchingChar);
		double changeInArea = (double)abs(possibleMatchingChar.boundingRect.area() - charCandidate.boundingRect.area()) / (double)charCandidate.boundingRect.area();
		double changeInWidth = (double)abs(possibleMatchingChar.boundingRect.width - charCandidate.boundingRect.width) / (double)charCandidate.boundingRect.width;
		double changeInHeight = (double)abs(possibleMatchingChar.boundingRect.height - charCandidate.boundingRect.height) / (double)charCandidate.boundingRect.height;

		// check if chars match
		if (distanceBetweenChars < (charCandidate.diagSize * MAX_DIAG_SIZE_MULTIPLE_AWAY) &&
			angleBetweenChars < MAX_ANGLE_BETWEEN_CHARS &&
			changeInArea < MAX_CHANGE_IN_AREA &&
			changeInWidth < MAX_CHANGE_IN_WIDTH &&
			changeInHeight < MAX_CHANGE_IN_HEIGHT) {
			vectorOfMatchingChars.push_back(possibleMatchingChar); //��� ���� ������ �������� �� ������������, �� �� ����������/��������� � ������� ��� �������
		}
	}

	return vectorOfMatchingChars;
}
//���������� ���������� ������� � ������������� ������� �� �� ������� ������������ ����� ����� ��������� �����
double charDistance(const CharCandidate &firstChar, const CharCandidate &secondChar) {
    int intX = abs(firstChar.charCenterX - secondChar.charCenterX);
    int intY = abs(firstChar.charCenterY - secondChar.charCenterY);

    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

double charsAngle(const CharCandidate &firstChar, const CharCandidate &secondChar) {
    double prilejasht = abs(firstChar.charCenterX - secondChar.charCenterX);
    double sreshtulejasht = abs(firstChar.charCenterY - secondChar.charCenterY);

    double radianAngle = atan(sreshtulejasht / prilejasht);
    double degreeAngle = radianAngle * (180.0 / CV_PI);

    return degreeAngle;
}

//��� ���������� �� ������� (�������� 1 ������ ������� ���� ��� �������, ��� ���� �� ���������� ��� �������, ������ ������
vector<CharCandidate> removeInnerOverlappingChars(vector<CharCandidate> &vectorOfMatchingChars) {
    vector<CharCandidate> vectorOfMatchingCharsWithInnerCharRemoved(vectorOfMatchingChars);

    for (auto &currentChar : vectorOfMatchingChars) {
        for (auto &otherChar : vectorOfMatchingChars) {
            if (currentChar != otherChar) {
                //��� ����� ������� ���� ����� ���� � ���� ��������, ����� ����� ������������ �� �������
                if (charDistance(currentChar, otherChar) < (currentChar.diagSize * MIN_DIAG_SIZE_MULTIPLE_AWAY)) {
                    // ��� ������� � ��-����� �� ������
                    if (currentChar.boundingRect.area() < otherChar.boundingRect.area()) {
                        vector<CharCandidate>::iterator currentCharIterator = find(vectorOfMatchingCharsWithInnerCharRemoved.begin(), vectorOfMatchingCharsWithInnerCharRemoved.end(), currentChar);
                         //��� ��������� �� ������ ���� ����� �� ��� �������� ����. ����� �������� ��� �� ��� �������
                        if (currentCharIterator != vectorOfMatchingCharsWithInnerCharRemoved.end()) {
                            vectorOfMatchingCharsWithInnerCharRemoved.erase(currentCharIterator);//������
                        }
                    } else {
						//������� � ��--�����
                        vector<CharCandidate>::iterator otherCharIterator = find(vectorOfMatchingCharsWithInnerCharRemoved.begin(), vectorOfMatchingCharsWithInnerCharRemoved.end(), otherChar);
                        if (otherCharIterator != vectorOfMatchingCharsWithInnerCharRemoved.end()) {
                            vectorOfMatchingCharsWithInnerCharRemoved.erase(otherCharIterator);
                        }
                    }
                }
            }
        }
    }

    return vectorOfMatchingCharsWithInnerCharRemoved;
}

//������������ �� ���������
string recogniseCharsInPlate(cv::Mat &imgThresh, vector<CharCandidate> &vectorOfMatchingChars) {
    string strChars;
    cv::Mat imgThreshColor;
	//��������� �� � , ���� �� ��-������� �-��� �� �� ��-�����
    sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), CharCandidate::sortLeftToRight);

    cv::cvtColor(imgThresh, imgThreshColor, CV_GRAY2BGR);
    for (auto &currentChar : vectorOfMatchingChars) {
        cv::rectangle(imgThreshColor, currentChar.boundingRect, SCALAR_GREEN, 2);
        cv::Mat imgROItoBeCloned = imgThresh(currentChar.boundingRect);
        cv::Mat imgROI = imgROItoBeCloned.clone();
        cv::Mat imgROIResized;
        cv::resize(imgROI, imgROIResized, cv::Size(RESIZED_CHAR_IMAGE_WIDTH, RESIZED_CHAR_IMAGE_HEIGHT));
        cv::Mat matROIFloat;
        imgROIResized.convertTo(matROIFloat, CV_32FC1);//������������ ��� float � �� �������� �� ���������� ������ �������������� �� openCV

        cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);//������� � ���� ���
        cv::Mat matCurrentChar(0, 0, CV_32F); // �� ������� ������ �� ������ ������� ������ �������������� � OpenCV

        kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);

        float currentChar = (float)matCurrentChar.at<float>(0, 0); //���������� �� ��� float

        strChars = strChars + char(int(currentChar));//�������� �������
    }

    return(strChars); 
}




