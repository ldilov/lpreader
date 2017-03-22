#include "Main.h"
#include <string>
#include <fstream>
using namespace std;

int main(void) {

	cout << "  --------------------------------------------------------------------------------------------" << endl;
	cout << "	 ___      _______  _______  _______  ______      ______   ___   ___      _______  __   __  " << endl;
	cout << "	|   |    |   _   ||       ||   _   ||    _ |    |      | |   | |   |    |       ||  | |  | " << endl;
	cout << "	|   |    |  |_|  ||____   ||  |_|  ||   | ||    |  _    ||   | |   |    |   _   ||  |_|  | " << endl;
	cout << "	|   |    |       | ____|  ||       ||   |_||_   | | |   ||   | |   |    |  | |  ||       | " << endl;
	cout << "	|   |___ |       || ______||       ||    __  |  | |_|   ||   | |   |___ |  |_|  ||       | " << endl;
	cout << "	|       ||   _   || |_____ |   _   ||   |  | |  |       ||   | |       ||       | |     |  " << endl;
	cout << "	|_______||__| |__||_______||__| |__||___|  |_|  |______| |___| |_______||_______|  |___|   " << endl;
	cout << " ------------------------ Razpoznavane nomer na MPS ------------------------------------------" << endl;
	cout << " ------------------------ Credits: OpenCV 3.2 ------------------------------------------------" << endl;
	cout << " \n \n \n";

	bool trainingStatus = loadAndLearnKNN();

    if (trainingStatus == false) { 
        cout << endl << endl << "greshka: KNN ne moje da izvurshi obuchavaneto uspeshno" << endl << endl;
        return(0);
    }

    cv::Mat imgOriginalScene;
	string fileName;
	cout << "Vuvedi ime na fail:";
	cin >> fileName;
    imgOriginalScene = cv::imread(fileName);

    if (imgOriginalScene.empty()) { 
        cout << "greshka: snimkata ne moje da se zaredi ot faila\n\n";
        return(0);
    }

    vector<PlateCandidate> vectorOfPossiblePlates = findAllPlates(imgOriginalScene);

    vectorOfPossiblePlates = getCharsInPlates(vectorOfPossiblePlates);

    cv::imshow("imgOriginalScene", imgOriginalScene);

    if (vectorOfPossiblePlates.empty()) {    
        cout << endl << "nqma zasecheni registracionni nomera" << endl;
    } else {  
        sort(vectorOfPossiblePlates.begin(), vectorOfPossiblePlates.end(), PlateCandidate::sortDescendingByNumberOfChars);
        PlateCandidate licPlate = vectorOfPossiblePlates.front();

        cv::imshow("plate", licPlate.plate); 
        cv::imshow("imgThresh", licPlate.imgThresh);

        if (licPlate.strChars.length() == 0) {
            cout << endl << "nqma zasecheni simvoli" << endl << endl;
            return(0);
        }

        drawGreenRectangleAroundPlate(imgOriginalScene, licPlate);

        cout << endl << "Registracionen nomer na MPS = " << licPlate.strChars << endl;
        cv::imshow("originalImage", imgOriginalScene);
        cv::imwrite("originalImage.png", imgOriginalScene);
		ofstream myfile;
		char answer;
		myfile.open("reg-nomer.txt", ios::out | ios::app);
		myfile << licPlate.strChars << endl;
		myfile.close();
		cout << "Registracionniqt nomer e zapisan vuv faila \"reg-nomer.txt\"" << endl;
    }

    cv::waitKey(0);
    return(0);
}

void drawGreenRectangleAroundPlate(cv::Mat &imgOriginalScene, PlateCandidate &licPlate) {
    cv::Point2f p2fRectPoints[4];

    licPlate.plateLocation.points(p2fRectPoints);

    for (int i = 0; i < 4; i++) {
        cv::line(imgOriginalScene, p2fRectPoints[i], p2fRectPoints[(i + 1) % 4], SCALAR_GREEN, 2);
    }
}




