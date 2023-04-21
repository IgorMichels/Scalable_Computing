#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <tuple>
#include <ctime>
#include <mutex>
#include <map>

using namespace std;

struct highwayData {
    int maxSpeed = 0;
    int carMaxSpeed = 0;
    string infoTime = "";
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

vector<string> getFiles() {
    DIR *dr;
    struct dirent *en;
    vector<string> files;
    const string folder = "files/";
    dr = opendir("files/");
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            if (strcmp(en->d_name, ".") == 0 || strcmp(en->d_name, "..") == 0) continue;
            files.push_back(folder + en->d_name);
        }
        closedir(dr);
    }
    sort(files.begin(), files.end());
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

vector<string> getBlocks(string text, int numBlocks) {
    vector<string> dataBlocks;
    size_t pos;
    string aux;
    string rows;
    int blockSize = text.length() / numBlocks;
    while (text != "") {
        if (blockSize > text.length()) {
            dataBlocks.push_back(text);
            break;
        }
        aux = text.substr(blockSize);
        pos = blockSize + aux.find('\n');
        rows = text.substr(0, pos);
        text.erase(0, pos + 1);
        dataBlocks.push_back(rows);
    }

    return dataBlocks;
}

void updateDataMultiThread(map<string, carData> *carInfos, highwayData *highwayInfos, string text) {
    int pos;
    string row;
    string plate;
    int actualLane;
    int lanePosition;
    while (text.length() > 0) {
        pos = text.find('\n');
        if (pos == -1) pos = text.length();
        row = text.substr(0, pos);
        text.erase(0, pos + 1);
        tie(plate, actualLane, lanePosition) = parseRow(row);
            if (plate.length() == 7) {
            (*highwayInfos).highwayDataBlocker.lock(); // barrar leitura aqui
            (*carInfos)[plate].lane = actualLane;
            (*carInfos)[plate].penultimatePosition = (*carInfos)[plate].lastPosition;
            (*carInfos)[plate].lastPosition = (*carInfos)[plate].actualPosition;
            (*carInfos)[plate].actualPosition = lanePosition;
            (*carInfos)[plate].isInHighway = true;
            (*highwayInfos).highwayDataBlocker.unlock(); // liberar leitura
        }
        else {
            (*highwayInfos).highwayDataBlocker.lock(); // barrar leitura aqui
            (*highwayInfos).infoTime = plate;
            (*highwayInfos).highwayDataBlocker.unlock(); // liberar leitura
        }
    }
}

void updateData(map<string, carData> *carInfos, highwayData *highwayInfos, string text) {
    int pos;
    string row;
    string plate;
    int actualLane;
    int lanePosition;
    while (text.length() > 0) {
        pos = text.find('\n');
        if (pos == -1) pos = text.length();
        row = text.substr(0, pos);
        text.erase(0, pos + 1);
        tie(plate, actualLane, lanePosition) = parseRow(row);
            if (plate.length() == 7) {
            (*carInfos)[plate].lane = actualLane;
            (*carInfos)[plate].penultimatePosition = (*carInfos)[plate].lastPosition;
            (*carInfos)[plate].lastPosition = (*carInfos)[plate].actualPosition;
            (*carInfos)[plate].actualPosition = lanePosition;
            (*carInfos)[plate].isInHighway = true;
        }
        else {
            (*highwayInfos).infoTime = plate;
        }
    }
}

void readFiles(string fileName, int maxBlocks, map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    string row;
    size_t pos;
    string text;
    int highway;
    fstream file;
    string plate;
    int actualLane;
    int lanePosition;
    int speedLimitCar;
    int speedLimitHighway;
    vector<thread *> threads;
    vector<string> dataBlocks; 
    file.open(fileName, ios::in);
    if (file.is_open()) {
        file.seekg(0, file.end);
        long int fileLength = file.tellg();
        file.seekg(0, file.beg);

        vector<char> buffer(fileLength);
        if (!file.read(buffer.data(), fileLength)) {
            perror("[ERROR] Failed to read the input file\n");
            exit(EXIT_FAILURE);
        }

        file.close();

        text = string(&buffer[0], fileLength);
        for (int i = 0; i < 3; i++) {
            pos = text.find('\n');
            row = text.substr(0, pos);
            text.erase(0, pos + 1);
            if (i == 0) {
                highway = getInfo(row);
                if ((*carInfos).find(highway) == (*carInfos).end()) (*carInfos)[highway] = new map<string, carData>;
                if ((*highwayInfos).find(highway) == (*highwayInfos).end()) (*highwayInfos)[highway] = new highwayData;
            }
            else if (i == 1) (*(*highwayInfos)[highway]).maxSpeed = getInfo(row);
            else (*(*highwayInfos)[highway]).carMaxSpeed = getInfo(row);
        }
        
        if ((*(*highwayInfos)[highway]).infoTime != "") {
            for (auto item : (*(*carInfos)[highway])) (*(*carInfos)[highway])[item.first].isInHighway = false;
        }

        int numBlocks;
        if (text.length() / 200 + 1 < maxBlocks) numBlocks = text.length() / 200 + 1;
        else numBlocks = maxBlocks;

        if (numBlocks > 1) {
            vector<string> dataBlocks = getBlocks(text, numBlocks);
            numBlocks = dataBlocks.size();
            for (int i = 0; i < numBlocks; i++) {
                thread *ti = new thread(&updateDataMultiThread, (*carInfos)[highway], &(*(*highwayInfos)[highway]), dataBlocks[i]);
                threads.push_back(ti);
            }
            
            for (auto th : threads) th -> join();
        }
        else {
            // nada pode ser lido enquanto estamos atualizando esse dicionário
            (*(*highwayInfos)[highway]).highwayDataBlocker.lock(); // barrar leitura aqui
            updateData((*carInfos)[highway], &(*(*highwayInfos)[highway]), text);
            (*(*highwayInfos)[highway]).highwayDataBlocker.unlock(); // liberar leitura
        }
        
        vector<string> remove;
        if ((*(*highwayInfos)[highway]).infoTime != "") {
            for (auto item : (*(*carInfos)[highway])) {
                if (item.second.isInHighway == false) remove.push_back(item.first);
            }
        }
        for (auto plate : remove) (*(*carInfos)[highway]).erase(plate);
    }
    remove(fileName.c_str());
}

void readFiles(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, bool always) {
    if (always) {
        while (true) {
            vector<string> files = getFiles();
            for (auto file : files) readFiles(file, 1, &(*carInfos), &(*highwayInfos));
        }
    }
    else {
        vector<string> files = getFiles();
        for (auto file : files) readFiles(file, 1, &(*carInfos), &(*highwayInfos));
    }
}
