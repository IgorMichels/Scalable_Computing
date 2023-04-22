#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <map>
#include <mutex>
#include <iostream>
#include <tuple>
#include <algorithm>
#include "Estruturas.h"

using namespace std;


int main() {

    externalAPI API(10,"extraInfoCars.txt");

    int maxBlocks = 5;
    int iters = 5;

    vector<string> _ = getFiles();
    int init = 0;
    int n_files = 4;
    vector<string> files;
    for(int i = 0; i < 5 * (n_files-1) + 1; i += 5){
        files.push_back(_[i]);
    }
    for(string e : files){cout << e << endl;}


    auto start = chrono::steady_clock::now();
    map<int, map<string, carData>*> carInfos;
    map<int, highwayData*> highwayInfos;

    for (string file : files) {
        readFile(file, 1, &carInfos, &highwayInfos);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;
    cout << "\n\nTotal time (sem contagem): " << totalTime.count() << "\n";

    // updateMaps(carInfos, &highwayInfos, &API);
    calculateSpeedAndAcceleration(carInfos, &highwayInfos);

    map<string, string> crashs = checkCollision(carInfos, &highwayInfos);

    map<int, int> cars_per_highway = countCarsByHighway(carInfos, &highwayInfos);
    for(auto e : cars_per_highway){
        cout << "Rodovia " << e.first << ": " << e.second << " carros \n";
    }

    for (const auto& p : carInfos) {
        int key = p.first; // obtém a chave do mapa externo
        map<string, carData>* carMap = p.second; // obtém o mapa interno

        for (const auto& q : *carMap) {
            string carName = q.first; // obtém a chave do mapa interno
            carData car = q.second; // obtém o valor do mapa interno

            // imprime o valor de carData
            cout << "Rodovia: " << key << ", Nome do carro: " << carName << endl;
            cout << "Pista: " << car.lane << ", Posição atual: " << car.actualPosition << endl;
            cout << "Última posição: " << car.lastPosition << ", Penúltima posição: " << car.penultimatePosition << endl;
            cout << "Está na rodovia? " << (car.isInHighway ? "Sim" : "Não") << endl;
            cout << "Nome: " << car.name << ", Modelo: " << car.model << ", Ano: " << car.year << endl;
            cout << "Velocidade: " << car.speed << ", Aceleração: " << car.acceleration << ", Pode colidir? " << (car.canCrash ? "Sim" : "Não") << endl;
            cout << "-----------------------" << endl;
        }
    }

    for(auto e : crashs){
        cout << e.first << " irá colidir com " << e.second << "\n";
    }


}