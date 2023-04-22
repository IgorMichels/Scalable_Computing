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
#include "readFiles.h"

using namespace std;

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

map<int, int> countCarsByHighway(map<int, map<string, carData>*> carInfos) {

    map<int, int> cars_per_highway;

    for (const auto& p : carInfos) {
        int key = p.first; // obtém a chave do mapa externo
        map<string, carData>* carMap = p.second; // obtém o mapa interno

        int carCount = 0;
        for (const auto& q : *carMap) {
            carData car = q.second; // obtém o valor do mapa interno
            carCount++;
        }
        cars_per_highway[key] = carCount;
        // cout << "Rodovia " << key << " tem " << carCount << " carros" << endl;
    }
    return cars_per_highway;
}

map<int, int> countCarsOverLim(map<int, map<string, carData>*> carInfos, map<int, highwayData*> highwayInfos) {

    map<int, int> cars_over_lim;

    for (const auto& p : carInfos) {
        int key = p.first; // obtém a chave do mapa externo
        map<string, carData>* carMap = p.second; // obtém o mapa interno

        int maxSpeed = highwayInfos[key]->maxSpeed;
        int carCount = 0;
        for (const auto& q : *carMap) {
            carData car = q.second; // obtém o valor do mapa interno
            if(maxSpeed < car.speed){
                carCount++;
            }
        }
        cars_over_lim[key] = carCount;
    }
    return cars_over_lim;
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

void analise(map<int, map<string, carData>*> carInfos, map<int, highwayData*> highwayInfos,
            map<int, int>* cars_per_highway, map<int, int>* cars_over_lim, int* total_cars){
    
    *cars_per_highway = countCarsByHighway(carInfos);
    *cars_over_lim = countCarsOverLim(carInfos, highwayInfos);
    *total_cars = 0;
    for(auto e : *cars_per_highway){
        *total_cars += e.second;
    }

}