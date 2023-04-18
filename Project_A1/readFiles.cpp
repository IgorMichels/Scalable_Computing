#include <iostream>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <tuple>
#include <map>

using namespace std;

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

void updateData(map<string, int[4]> *dict, string row) {
    string plate;
    int actualLane;
    int lanePosition;
    tie(plate, actualLane, lanePosition) = parseRow(row);

    // IMPORTANTE: barrar leitura aqui
    (*dict)[plate][0] = actualLane;
    (*dict)[plate][3] = (*dict)[plate][2];
    (*dict)[plate][2] = (*dict)[plate][1];
    (*dict)[plate][1] = lanePosition;
    // liberar leitura
}

void readFiles(string fileName, map<int, map<string, int[4]>*> *carPositions) { //, map<int, int[2]> highwayDetails) {
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
            }
            else if (lineController == 2) {
                speedLimitHighway = getInfo(row);
            }
            else if (lineController == 3) {
                speedLimitCar = getInfo(row);
            }
            else {
                if ((*carPositions).find(highway) == (*carPositions).end()) (*carPositions)[highway] = new map<string, int[4]>;
                updateData((*carPositions)[highway], row);
            }
        }
        file.close();
    }
    remove(fileName.c_str());
}

int main() {
    // dicionário externo: highway para outro dicionário
    // dicionário interno: plate para [lane, pos, pos - 1, pos -2]
    map<int, map<string, int[4]>*> infos;

    auto start = chrono::steady_clock::now();
    vector<string> files = getFiles();
    for (auto file : files) readFiles(file, &infos);
    auto end = chrono::steady_clock::now();

    // cout << (*(infos[101]))["LKT1F56"][0] << endl;
    // cout << (*(infos[101]))["LKT1F56"][1] << endl;
    // cout << (*(infos[101]))["LKT1F56"][2] << endl;
    // cout << (*(infos[101]))["LKT1F56"][3] << endl;

    chrono::duration<double> totalTime = end - start;
    cout << totalTime.count() << endl;
    return 0;
}