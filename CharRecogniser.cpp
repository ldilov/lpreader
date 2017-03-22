#include "CharRecogniser.h"
using namespace std;

cv::Ptr<cv::ml::KNearest> kNearest = cv::ml::KNearest::create();

bool loadAndLearnKNN(void) {

    cv::Mat classificationsMatrix; // ще прочетем номерата на класификациите(класовете) в тази променлива като вектор
    cv::FileStorage classificationsFile("charactergroups.xml", cv::FileStorage::READ);

    if (classificationsFile.isOpened() == false) {
        cout << "greshka: lispvat klasifikaciite ot dataset-a\n\n";
        return false;
    }

	//вкарваме класификациите в променливата
    classificationsFile["charactergroups"] >> classificationsMatrix;
    classificationsFile.release();
	
    cv::Mat imagesFloatsMatrix; //ще прочетем множеството картини от training dataset-a в тази променлива като вектор
    cv::FileStorage imagesFile("images.xml", cv::FileStorage::READ);

    if (imagesFile.isOpened() == false) {
        cout << "greshka: ne moje da otvori snimkite ot traindata set-a.\n\n";
        return false;
    }

	//вкарваме картините в променливата
    imagesFile["images"] >> imagesFloatsMatrix;
    imagesFile.release();

    //Търсим най-близкият съсед (к=1 => nearest neighbour alg)
	kNearest->setDefaultK(1);
    kNearest->train(imagesFloatsMatrix, cv::ml::ROW_SAMPLE, classificationsMatrix);

    return true;
}

vector<PlateCandidate> getCharsInPlates(vector<PlateCandidate> &plateCadndidatesVector) {
    int intPlateCounter = 0; // брояч
    cv::Mat imgContours;
    vector<vector<cv::Point> > contours;
    cv::RNG rng;

    if (plateCadndidatesVector.empty()) {
        return plateCadndidatesVector;
    }

    for (auto &plateCandidate : plateCadndidatesVector) { 		
        preparation(plateCandidate.plate, plateCandidate.imgGrayscale, plateCandidate.imgThresh); //Преобразуваме в grayscale и threshold-ваме

        //извършваме преоразмеряване с 1.6 коеф. = 60%. 
        cv::resize(plateCandidate.imgThresh, plateCandidate.imgThresh, cv::Size(), 1.6, 1.6);
        //Повторен threshold-инг
        cv::threshold(plateCandidate.imgThresh, plateCandidate.imgThresh, 0.0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);

         //намираме контурите на вс възможни символи и вкарваме само тези, които са символи
        vector<CharCandidate> vectorOfPossibleCharsInPlate = findPossibleCharsInPlate(plateCandidate.imgGrayscale, plateCandidate.imgThresh);
        
        //имайки вектор от вс. възможни символи, търсим групи от символи, за които имаме "обвързване" (match-ват)
        vector<vector<CharCandidate> > listOfVectorsOfMatchingCharsInPlate = findListOfMatchingCharsVectors(vectorOfPossibleCharsInPlate);

        if (listOfVectorsOfMatchingCharsInPlate.size() == 0) {
            plateCandidate.strChars = ""; //предполагаемият рег. номер няма символи, при които имаме "обвързване" 
            continue;//минаваме на следв. итерация
        }

        for (auto &vectorOfMatchingChars : listOfVectorsOfMatchingCharsInPlate) { //имаме вектори от символи, при които имаме "обвързване" , за вс. от тях:
            sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), CharCandidate::sortLeftToRight); //сортираме ляво-> дясно , така че символите да са подредени спрямо своите центрове по оста Х
            vectorOfMatchingChars = removeInnerOverlappingChars(vectorOfMatchingChars); //махаме препокриващи се символи
        }

        unsigned int LongestVectorOfCharsSize = 0, LongestVectorOfCharsIndex = 0;

         //от вс. вектори със символи, при които имаме "обвързване" , взимаме  индекса на най-големия
        for (unsigned int i = 0; i < listOfVectorsOfMatchingCharsInPlate.size(); i++) {
            if (listOfVectorsOfMatchingCharsInPlate[i].size() > LongestVectorOfCharsSize) {
                LongestVectorOfCharsSize = listOfVectorsOfMatchingCharsInPlate[i].size();
                LongestVectorOfCharsIndex = i;
            }
        }
        vector<CharCandidate> biggestVectorOfMatchingCharsInPlate = listOfVectorsOfMatchingCharsInPlate[LongestVectorOfCharsIndex];

		//извършваме разпознаване на символите върху най-големият вектор от символи, при които имаме "обвързване"
		plateCandidate.strChars = recogniseCharsInPlate(plateCandidate.imgThresh, biggestVectorOfMatchingCharsInPlate); 
    } //цикълът приключва

    return plateCadndidatesVector;
}

vector<CharCandidate> findPossibleCharsInPlate(cv::Mat &imgGrayscale, cv::Mat &imgThresh) {
    vector<CharCandidate> vectorOfPossibleChars;
    cv::Mat imgThreshCopy;
    vector<vector<cv::Point> > contours;
	imgThreshCopy = imgThresh.clone();

    cv::findContours(imgThreshCopy, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE); //намираме всички контури в рег. номер

    for (auto &contour : contours) {
        CharCandidate CharCandidate(contour);

        if (isPossibleChar(CharCandidate)) {
            vectorOfPossibleChars.push_back(CharCandidate); //ако е предполагаем символ, добавяме го към списъка с предполагаеми символи
        }
    }

    return vectorOfPossibleChars;
}

bool isPossibleChar(CharCandidate &charCandidate) {
    //проверяваме дали контурът е възможно да описва символ
	//сравняваме отношение на размери на правоъгълникът, който обгражда предполагаемите символи, както и всяко негово измерение по отделно
    if (charCandidate.boundingRect.area() > MIN_PIXEL_AREA &&
        charCandidate.boundingRect.width > MIN_PIXEL_WIDTH && charCandidate.boundingRect.height > MIN_PIXEL_HEIGHT &&
        MIN_ASPECT_RATIO < charCandidate.aspectRatio && charCandidate.aspectRatio < MAX_ASPECT_RATIO) {
        return(true);
    } else {
        return(false);
    }
}

vector<vector<CharCandidate> > findListOfMatchingCharsVectors(const vector<CharCandidate> &vectorOfPossibleChars) {
	//взимаме всички евентуални символи в един голям вектор. Преподреждаме така че да имаме вектор от вектори от символи, за които имаме "обвързване"
    vector<vector<CharCandidate> > listOfVectorsOfMatchingChars;

    for (auto &charCandidate : vectorOfPossibleChars) {

         //намираме всички символи от големия вектор, които имат "обвързване" с текущия символ
        vector<CharCandidate> vectorOfMatchingChars = findVectorOfMatchingChars(charCandidate, vectorOfPossibleChars);

        vectorOfMatchingChars.push_back(charCandidate);//текущият символ го добавяме към евентуалният вектор със символи, за които има "обвързване"
         
        if (vectorOfMatchingChars.size() < MIN_NUMBER_OF_MATCHING_CHARS) {
            continue; //ако текущият вектор от символи, за които има "обвързване" е твърде малък за да съставя рег. номер, то той не ни върши работа
        }
       
        listOfVectorsOfMatchingChars.push_back(vectorOfMatchingChars);

        vector<CharCandidate> vectorOfPossibleCharsWithCurrentMatchesRemoved;

        for (auto &possChar : vectorOfPossibleChars) {
            if (find(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), possChar) == vectorOfMatchingChars.end()) {
                vectorOfPossibleCharsWithCurrentMatchesRemoved.push_back(possChar);
            }
        }
        
        vector<vector<CharCandidate> > recursivelistOfVectorsOfMatchingChars;

        recursivelistOfVectorsOfMatchingChars = findListOfMatchingCharsVectors(vectorOfPossibleCharsWithCurrentMatchesRemoved);	//скапана рекурсия

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

		//ако търсим обвързване на символ със себе си, то го пропускаме (такъв момент ще има)
		if (possibleMatchingChar == charCandidate) {
			continue;
		}
		// сравняваме символите дали са "достойни" за обвързване
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
			vectorOfMatchingChars.push_back(possibleMatchingChar); //ако този символ отговаря на изискванията, то го обвързваме/групираме с другите във вектора
		}
	}

	return vectorOfMatchingChars;
}
//Използваме питагорова теорема в координатната система за да намерим разстоянието между двете централни точки
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

//при съвпадение на символи (примерно 1 символ отчетен като два еднакви, тъй като се отбелязват два контура, махаме малкия
vector<CharCandidate> removeInnerOverlappingChars(vector<CharCandidate> &vectorOfMatchingChars) {
    vector<CharCandidate> vectorOfMatchingCharsWithInnerCharRemoved(vectorOfMatchingChars);

    for (auto &currentChar : vectorOfMatchingChars) {
        for (auto &otherChar : vectorOfMatchingChars) {
            if (currentChar != otherChar) {
                //ако двата символа имат почти едни и същи центрове, тоест имаме препокриващи се символи
                if (charDistance(currentChar, otherChar) < (currentChar.diagSize * MIN_DIAG_SIZE_MULTIPLE_AWAY)) {
                    // ако първият е по-малък от втория
                    if (currentChar.boundingRect.area() < otherChar.boundingRect.area()) {
                        vector<CharCandidate>::iterator currentCharIterator = find(vectorOfMatchingCharsWithInnerCharRemoved.begin(), vectorOfMatchingCharsWithInnerCharRemoved.end(), currentChar);
                         //ако итератора не стигне края значи не сме стигнали края. Тоест намерили сме го във вектора
                        if (currentCharIterator != vectorOfMatchingCharsWithInnerCharRemoved.end()) {
                            vectorOfMatchingCharsWithInnerCharRemoved.erase(currentCharIterator);//махаме
                        }
                    } else {
						//вторият е по--малък
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

//разпознаване на символите
string recogniseCharsInPlate(cv::Mat &imgThresh, vector<CharCandidate> &vectorOfMatchingChars) {
    string strChars;
    cv::Mat imgThreshColor;
	//сортираме по Х , така че по-малките Х-ове да са по-вляво
    sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), CharCandidate::sortLeftToRight);

    cv::cvtColor(imgThresh, imgThreshColor, CV_GRAY2BGR);
    for (auto &currentChar : vectorOfMatchingChars) {
        cv::rectangle(imgThreshColor, currentChar.boundingRect, SCALAR_GREEN, 2);
        cv::Mat imgROItoBeCloned = imgThresh(currentChar.boundingRect);
        cv::Mat imgROI = imgROItoBeCloned.clone();
        cv::Mat imgROIResized;
        cv::resize(imgROI, imgROIResized, cv::Size(RESIZED_CHAR_IMAGE_WIDTH, RESIZED_CHAR_IMAGE_HEIGHT));
        cv::Mat matROIFloat;
        imgROIResized.convertTo(matROIFloat, CV_32FC1);//конвертиране във float е не боходимо за алгоритъма според документацията на openCV

        cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);//правиме я един ред
        cv::Mat matCurrentChar(0, 0, CV_32F); // за текущия символ ни трябва матрица според документацията в OpenCV

        kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);

        float currentChar = (float)matCurrentChar.at<float>(0, 0); //превръщаме го във float

        strChars = strChars + char(int(currentChar));//вкарваме символа
    }

    return(strChars); 
}




