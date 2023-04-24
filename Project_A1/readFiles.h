#include <sys/time.h>
#include <algorithm>
#include <dirent.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <tuple>
#include <ctime>
#include <map>

#include "API.h"

using namespace std;
mutex iomutex;

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
    if (pos == -1) return make_tuple(row, 0, 0);
    string plate = row.substr(0, pos);
    string position = row.substr(pos + 2);
    pos = position.find(',');
    int actualLane = stoi(position.substr(1, pos - 1));
    int lanePosition = stoi(position.substr(pos + 2, position.find(')') - pos - 2));
    
    return make_tuple(plate, actualLane, lanePosition);
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
            if ((*carInfos)[plate].extraInfos == false) (*carInfos)[plate].extraInfos = API.query_vehicle(plate);
        }
        else {
            (*highwayInfos).infoTime = plate;
        }
    }
}

void readFile(string fileName, map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
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
            if (pos == -1) {
                i--;
                continue;
            }
            row = text.substr(0, pos);
            if (i == 0) {
                try {
                    highway = getInfo(row);
                }
                catch (...) {
                    cout << row << " deu pau " << pos << " " << text << " essa foi a string" << (pos == -1) << " " << (text == "") << endl;
                    abort;
                }
                if ((*carInfos).find(highway) == (*carInfos).end()) (*carInfos)[highway] = new map<string, carData>;
                if ((*highwayInfos).find(highway) == (*highwayInfos).end()) (*highwayInfos)[highway] = new highwayData;
            }
            else {
                (*(*highwayInfos)[highway]).highwayDataBlocker.lock(); // barrar leitura aqui
                if (i == 1) (*(*highwayInfos)[highway]).maxSpeed = getInfo(row);
                else (*(*highwayInfos)[highway]).carMaxSpeed = getInfo(row);
                (*(*highwayInfos)[highway]).highwayDataBlocker.unlock(); // barrar leitura aqui
            }
            text.erase(0, pos + 1);
        }
        
        // nada pode ser lido enquanto estamos atualizando esse dicionário
        (*(*highwayInfos)[highway]).highwayDataBlocker.lock(); // barrar leitura aqui
        if ((*(*highwayInfos)[highway]).infoTime != "") {
            for (auto item : (*(*carInfos)[highway])) (*(*carInfos)[highway])[item.first].isInHighway = false;
        }

        updateData((*carInfos)[highway], &(*(*highwayInfos)[highway]), text);
        
        vector<string> remove;
        if ((*(*highwayInfos)[highway]).infoTime != "") {
            for (auto item : (*(*carInfos)[highway])) {
                if (item.second.isInHighway == false) remove.push_back(item.first);
            }
        }

        for (auto plate : remove) (*(*carInfos)[highway]).erase(plate);
        (*(*highwayInfos)[highway]).highwayDataBlocker.unlock(); // liberar leitura
    }
    remove(fileName.c_str());
}

void readFiles(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, bool always) {
    if (always) {
        while (true) {
            vector<string> files = getFiles();
            if (files.size() == 0) {
                active = false;
                cout << "Aguardando novos arquivos" << endl;
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            else {
                for (auto file : files) readFile(file, &(*carInfos), &(*highwayInfos));
                active = true;
                this_thread::sleep_for(chrono::milliseconds(10));
            }
            // break;
        }
    }
    else {
        vector<string> files = getFiles();
        for (auto file : files) readFile(file, &(*carInfos), &(*highwayInfos));
    }
}
