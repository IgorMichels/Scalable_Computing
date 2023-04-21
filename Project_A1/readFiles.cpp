#include <sys/time.h>
#include <algorithm>
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
#include <cstring>

#include "API.h"

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

void updateDataBK(map<string, carData> *carInfos, highwayData *highwayInfos, string text) {
    int pos;
    string row;
    string plate;
    int actualLane;
    int lanePosition;
    cout << text << endl << "deu" << endl << endl << endl;
    (*highwayInfos).carDataBlocker.lock(); // barrar leitura aqui
    while ((pos = text.find('\n')) != string::npos) {
        row = text.substr(0, pos);
        cout << row << endl;
        text.erase(0, pos + 1);
        tie(plate, actualLane, lanePosition) = parseRow(row);
        // (*highwayInfos).carDataBlocker.lock(); // barrar leitura aqui
        (*carInfos)[plate].lane = actualLane;
        (*carInfos)[plate].penultimatePosition = (*carInfos)[plate].lastPosition;
        (*carInfos)[plate].lastPosition = (*carInfos)[plate].actualPosition;
        (*carInfos)[plate].actualPosition = lanePosition;
        (*carInfos)[plate].isInHighway = true;
        // (*highwayInfos).carDataBlocker.unlock();
    }

    // cout << endl << text << endl << text.find('\n') << endl;
    tie(plate, actualLane, lanePosition) = parseRow(text);
    if (plate.length() == 7) {
        // (*highwayInfos).carDataBlocker.lock(); // barrar leitura aqui
        (*carInfos)[plate].lane = actualLane;
        (*carInfos)[plate].penultimatePosition = (*carInfos)[plate].lastPosition;
        (*carInfos)[plate].lastPosition = (*carInfos)[plate].actualPosition;
        (*carInfos)[plate].actualPosition = lanePosition;
        (*carInfos)[plate].isInHighway = true;
        // (*highwayInfos).carDataBlocker.unlock(); // liberar leitura
    }
    else {
        (*highwayInfos).highwayDataBlocker.lock(); // barrar leitura aqui
        (*highwayInfos).infoTime = plate;
        (*highwayInfos).highwayDataBlocker.unlock(); // liberar leitura
    }
    (*highwayInfos).carDataBlocker.unlock();
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
            (*highwayInfos).carDataBlocker.lock(); // barrar leitura aqui
            (*carInfos)[plate].lane = actualLane;
            (*carInfos)[plate].penultimatePosition = (*carInfos)[plate].lastPosition;
            (*carInfos)[plate].lastPosition = (*carInfos)[plate].actualPosition;
            (*carInfos)[plate].actualPosition = lanePosition;
            (*carInfos)[plate].isInHighway = true;
            (*highwayInfos).carDataBlocker.unlock(); // liberar leitura
        }
        else {
            (*highwayInfos).highwayDataBlocker.lock(); // barrar leitura aqui
            (*highwayInfos).infoTime = plate;
            (*highwayInfos).highwayDataBlocker.unlock(); // liberar leitura
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

        vector<string> dataBlocks = getBlocks(text, numBlocks);
        numBlocks = dataBlocks.size();
        for (int i = 0; i < numBlocks; i++) {
            thread *ti = new thread(&updateData, (*carInfos)[highway], &(*(*highwayInfos)[highway]), dataBlocks[i]);
            threads.push_back(ti);
        }
        
        for (auto th : threads) th -> join();
        
        vector<string> remove;
        if ((*(*highwayInfos)[highway]).infoTime != "") {
            for (auto item : (*(*carInfos)[highway])) {
                // cout << item.first << " está sendo testado para apagar: " << item.second.isInHighway << endl;
                if (item.second.isInHighway == false) remove.push_back(item.first);
            }
        }
        for (auto plate : remove) (*(*carInfos)[highway]).erase(plate);
    }
    remove(fileName.c_str());
}

int main() {
    int maxBlocks = 10;
    int iters = 5;

    
    for (int t = 1; t <= maxBlocks; t++) {
        auto start = chrono::steady_clock::now();
        for (int i = 0; i < iters; i++) {
            map<int, map<string, carData>*> carInfos;
            map<int, highwayData*> highwayInfos;
            vector<string> files = getFiles();
            for (auto file : files) readFiles(file, t, &carInfos, &highwayInfos);
        }
        auto end = chrono::steady_clock::now();
        chrono::duration<double> totalTime = end - start;
        cout << "threads: " << t << "  tempo total: " << totalTime.count() / iters << endl;
    }
    

    auto start = chrono::steady_clock::now();
    map<int, map<string, carData>*> carInfos;
    map<int, highwayData*> highwayInfos;
    vector<string> files = getFiles();
    for (auto file : files) {
        readFiles(file, 1, &carInfos, &highwayInfos);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;
    cout << "Total time: " << totalTime.count() << endl;

    // cout << "IFZ2Z79" << endl;
    // cout << (*(carInfos[103]))["IFZ2Z79"].lane << endl;
    // cout << (*(carInfos[103]))["IFZ2Z79"].actualPosition << endl;
    // cout << (*(carInfos[103]))["IFZ2Z79"].lastPosition << endl;
    // cout << (*(carInfos[103]))["IFZ2Z79"].penultimatePosition << endl;

    // for(auto it = (*(carInfos[103])).cbegin(); it != (*(carInfos[103])).cend(); ++it) {
    //     cout << it->first << endl;
    // }

    // cout << endl;
    // cout << (*(highwayInfos[101])).infoTime << endl;
    // cout << (*(highwayInfos[101])).maxSpeed << endl;
    // cout << (*(highwayInfos[101])).carMaxSpeed << endl;

    // cout << "\nTestando mock de dados externos" << endl;
    // MyClass API;
    // string plate = "WXY-5678";
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

    // cout << endl;
    return 0;

}
