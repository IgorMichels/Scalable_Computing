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
    // remove(fileName.c_str());
}


void checkCollisions(map<int, map<string, carData>*>& carInfos) {
    // Percorre todas as rodovias
    for (auto& [highway, cars] : carInfos) {
        // Percorre todas as pistas
        for (int lane = 0; lane < 4; lane++) {
            // Cria um vetor com os carros da pista atual
            vector<carData*> carsInLane;
            for (auto& [name, data] : *cars) {
                if (data.lane == lane) {
                    carsInLane.push_back(&data);
                }
            }

            // Verifica as colisões entre os carros da pista atual
            int numCars = carsInLane.size();
            for (int i = 0; i < numCars; i++) {
                for (int j = i + 1; j < numCars; j++) {
                    auto car1 = carsInLane[i];
                    auto car2 = carsInLane[j];

                    if (car1->speed == car2->speed && car1->acceleration == car2->acceleration) {
                        int timeToCollision = (car2->actualPosition - car1->actualPosition) / car1->speed;
                        if (timeToCollision == 0) {
                            cout << "Os carros " << car1->name << " e " << car2->name << " vão colidir na rodovia " << highway << " na pista " << lane << endl;
                        }
                    }
                }
            }
        }
    }
}


void countCarsByHighway(map<int, map<string, carData>*> carInfos) {
    for (const auto& p : carInfos) {
        int key = p.first; // obtém a chave do mapa externo
        map<string, carData>* carMap = p.second; // obtém o mapa interno

        int carCount = 0;
        for (const auto& q : *carMap) {
            carData car = q.second; // obtém o valor do mapa interno
            carCount++;
        }

        cout << "Rodovia " << key << " tem " << carCount << " carros" << endl;
    }
}

void calculateSpeedAndAcceleration(map<int, map<string, carData>*>& carInfos) {
    for (auto& p : carInfos) {
        map<string, carData>* carMap = p.second;
        for (auto& q : *carMap) {
            carData& car = q.second;
            int currentPosition = car.actualPosition;
            int lastPosition = car.lastPosition;
            int penultimatePosition = car.penultimatePosition;
            int timeInterval = 1.0; // por enquanto estamos difinindo o intervalo de tempo como 1 segundo (deve ser ajustado para o tempo entre dois estados)

            // Calcula a velocidade
            double velocity = (currentPosition - lastPosition) / timeInterval;
            car.speed = static_cast<int>(velocity); // converte a velocidade de double para int e atribui ao campo speed da struct carData

            // Calcula a aceleração
            double lastVelocity = (lastPosition - penultimatePosition) / timeInterval;
            double acceleration = (velocity - lastVelocity) / timeInterval;
            car.acceleration = static_cast<int>(acceleration); // converte a aceleração de double para int e atribui ao campo acceleration da struct carData
        }
    }
}



map<string, string> checkCollision(map<string, map<string, int>> carsAcceleration, int maxSpeed, int dt=1){
    /*
        Função para verificar quais carros podem colidir mantendo aceleração e pista constantes. 
    Caso o carro atinja a velocidade máxima ele para de acelerar. Usamos a fórmula do MUV até o momento
    em que o carro para de acelerar e MU dali em seguida.

    * @param carsAcceleration: um dicionário onde a chave corresponde a placa e o valor é o dicionário
        com posição, velocidade e aceleração do carro referente a placa.
    * @param maxSpeed: velocidade máxima que o carro consegue atingir em dada rodovia.
    * @param dt: intervalo de tempo. default=1.
    */

    map<string, string> crashs;
    vector<double> positions;
    vector<string> cars;
    double S = 0;
    map<string, int> _info;
    double t_lim_speed;
    
    for (auto it : carsAcceleration) {
        _info = it.second;
        if(_info["speed"] == 0){
            t_lim_speed = dt;
        }
        else if ((maxSpeed - _info["speed"]) / (double)_info["speed"] > dt){
            t_lim_speed = dt;
        }
        else{
            t_lim_speed = (maxSpeed - _info["speed"]) / (double)_info["speed"];
        }
        S = _info["pos"] + _info["speed"] * dt + _info["acceleration"] * t_lim_speed * t_lim_speed / 2;
        positions.push_back(S);
        cars.push_back(it.first);
    }

    for(int i = 0; i < positions.size() - 1; i++){
        if (positions[i] >= positions[i+1]){
            crashs[cars[i]] = cars[i+1];
        }
    }

    return crashs;
}


void checkCollisionsInAllHighways(map<int, map<string, carData>*> carInfos, map<int, highwayData*> highwayInfos){
    for(auto& highway : highwayInfos){
        map<string, map<string, int>> carsAcceleration;
        int maxSpeed = highway.second->maxSpeed;
        for(auto& car : *(carInfos[highway.first])){
            map<string, int> carInfo = {
                {"pos", car.second.actualPosition},
                {"speed", car.second.speed},
                {"acceleration", car.second.acceleration}
            };
            carsAcceleration[car.first] = carInfo;
        }
        map<string, string> collisions = checkCollision(carsAcceleration, maxSpeed);
        if(collisions.size() > 0){
            cout << "Collisions in highway " << highway.first << ":\n";
            for(auto& collision : collisions){
                cout << collision.first << " and " << collision.second << "\n";
            }
        }
        else{
            cout << "No collisions in highway " << highway.first << "\n";
        }
    }
}

