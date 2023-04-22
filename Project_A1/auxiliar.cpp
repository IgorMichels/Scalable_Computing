#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <map>
#include <mutex>
#include "Estruturas.h"

using namespace std;



int main() {

    Api& API = Api::getInstance(3,"car.txt");

    int maxBlocks = 5;
    int iters = 5;


    vector<string> _ = getFiles();
    int init = 0;
    int n_files = 10;
    vector<string> files;
    for(int i = 0; i < 5 * (n_files-1) + 1; i += 5){
        files.push_back(_[i]);
    }
    // vector<string> files = {_.begin() + init, _.begin() + init + n_files};
    for(string e : files){cout << e << endl;}


    auto start = chrono::steady_clock::now();
    map<int, map<string, carData>*> carInfos;
    map<int, highwayData*> highwayInfos;

    for (auto file : files) {
        readFiles(file, 1, &carInfos, &highwayInfos);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;
    cout << "Total time: " << totalTime.count() << "\n\n";


    calculateSpeedAndAcceleration(carInfos);

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
    cout << "\n\n";

    countCarsByHighway(carInfos);

    cout << "\n\n";

    // checkCollisions(carInfos);

    for (auto& p : highwayInfos){
        int hw_number = p.first;
        highwayData * HW = p.second;

        cout << "Rodovia: " << hw_number << endl;
        cout << "Vel. Max.: " << HW->maxSpeed << " | Carro V.M.: " << HW->carMaxSpeed << " Time: " << HW->infoTime << "\n\n";
    }

   

    return 0;
}
 
 