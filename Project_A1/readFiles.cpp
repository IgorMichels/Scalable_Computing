#include <sys/time.h>
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <tuple>
#include <ctime>
#include <mutex>
#include <map>

#include "API_query.h"

using namespace std;

struct highwayData {
    int maxSpeed = 0;
    int carMaxSpeed = 0;
    string infoTime = "";
    mutex carDataBlocker;
    mutex highwayDataBlocker;
};

struct carData {
    int lane = 0;
    int actualPosition = 0;
    int lastPosition = 0;
    int penultimatePosition = 0; // sim, essa palavra existe, só não é muito usada (https://www.italki.com/en/post/question-92435)
    bool isInHighway = true;
    string name = "";
    string model = "";
    int year = 1900;
    int speed = 0;
    int acceleration = 0;
    bool canCrash = false;
};

map<int, map<string, carData>*> carInfos;
map<int, highwayData*> highwayInfos;

vector<string> getFiles() {
    DIR *dr;
    struct dirent *en;
    vector<string> files;
    const string folder = "files/";
    dr = opendir("files/");
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            if (strcmp(en->d_name, ".") == 0 | strcmp(en->d_name, "..") == 0) continue;
            files.push_back(folder + en->d_name);
        }
        closedir(dr);
    }
    sort(files.begin(), files.end());
    // reverse(files.begin(), files.end());
    return files;
}

int getInfo(string row) {
    return stoi(row.substr(row.find(' ') + 1));
}

tuple<string, int, int> parseRow(string row)
{
    int pos = row.find(',');
    string plate = row.substr(0, pos);
    string position = row.substr(pos + 2);
    pos = position.find(',');
    int actualLane = stoi(position.substr(1, pos - 1));
    int lanePosition = stoi(position.substr(pos + 2, position.find(')') - pos - 2));
    
    return make_tuple(plate, actualLane, lanePosition);
}

void updateData(map<string, carData> *carInfos, highwayData *hwData, string row) {
    string plate;
    int actualLane;
    int lanePosition;
    tie(plate, actualLane, lanePosition) = parseRow(row);
    if (plate.length() > 7) {
        (*hwData).highwayDataBlocker.lock(); // barrar leitura aqui
        (*hwData).infoTime = plate;
        (*hwData).highwayDataBlocker.unlock(); // liberar leitura
    }
    else {
        (*hwData).carDataBlocker.lock(); // barrar leitura aqui
        (*carInfos)[plate].lane = actualLane;
        (*carInfos)[plate].penultimatePosition = (*carInfos)[plate].lastPosition;
        (*carInfos)[plate].lastPosition = (*carInfos)[plate].actualPosition;
        (*carInfos)[plate].actualPosition = lanePosition;
        (*hwData).carDataBlocker.unlock(); // liberar leitura
    }
}

void readFiles(string fileName, map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    fstream file;
    file.open(fileName, ios::in);
    if (file.is_open()) {
        string row;
        string plate;
        int highway;
        int actualLane;
        int lanePosition;
        int speedLimitCar;
        int speedLimitHighway;
        int lineController = 0;
        while(getline(file, row)) {
            lineController++;
            if (lineController == 1) {
                highway = getInfo(row);
                if ((*carInfos).find(highway) == (*carInfos).end()) (*carInfos)[highway] = new map<string, carData>;
                if ((*highwayInfos).find(highway) == (*highwayInfos).end()) (*highwayInfos)[highway] = new highwayData;
            }
            else if (lineController == 2) {
                (*(*highwayInfos)[highway]).maxSpeed = getInfo(row);
            }
            else if (lineController == 3) {
                (*(*highwayInfos)[highway]).carMaxSpeed = getInfo(row);
            }
            else {
                updateData((*carInfos)[highway], &(*(*highwayInfos)[highway]), row);
            }
        }
        file.close();
    }
    // remove(fileName.c_str());
}

int main() {
    auto start = chrono::steady_clock::now();
    vector<string> files = getFiles();
    for (auto file : files) readFiles(file, &carInfos, &highwayInfos);
    auto end = chrono::steady_clock::now();

    cout << "DBL9H83" << endl;
    cout << (*(carInfos[101]))["DBL9H83"].lane << endl;
    cout << (*(carInfos[101]))["DBL9H83"].actualPosition << endl;
    cout << (*(carInfos[101]))["DBL9H83"].lastPosition << endl;
    cout << (*(carInfos[101]))["DBL9H83"].penultimatePosition << endl;

    cout << endl;
    cout << (*(highwayInfos[101])).infoTime << endl;
    cout << (*(highwayInfos[101])).maxSpeed << endl;
    cout << (*(highwayInfos[101])).carMaxSpeed << endl;

    // cout << "\nTestando mock de dados externos" << endl;
    // MyClass API;
    // string plate = "DBL9H83";
    // API.search(plate);
    // cout << API.get_name() << endl;
    // cout << API.get_model() << endl;
    // cout << API.get_year() << endl;

    // (*(carInfos[101]))[plate].name = API.get_name();
    // (*(carInfos[101]))[plate].model = API.get_model();
    // (*(carInfos[101]))[plate].year = API.get_year();

    // cout << (*(carInfos[101]))[plate].name << ' ' << API.get_name() << endl;
    // cout << (*(carInfos[101]))[plate].model << ' ' << API.get_model() << "batata" << endl;
    // cout << (*(carInfos[101]))[plate].year << ' ' << API.get_year() << endl;

    cout << endl;
    chrono::duration<double> totalTime = end - start;
    cout << totalTime.count() << endl;
    return 0;
}