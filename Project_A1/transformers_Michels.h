#include "readFiles.h"
#include "API.h"

#include <iostream>
#include <sstream>
#include <chrono>

void calculateTime(string time, string function) {
    tm tm{};
    stringstream ss{time};
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto tp = chrono::system_clock::from_time_t(mktime(&tm)) +
              chrono::microseconds{std::stoi(time.substr(20))};

    auto now = chrono::system_clock::now();
    auto duration = chrono::duration_cast<std::chrono::microseconds>(now - tp);
    cout << "Tempo de processamento (" << function << "): " << duration.count() / 1000.0 << " milisegundos" << endl;
}

void analysisStats(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    vector<string> times;
    long int numCars = 0;
    long int numHighways = (*highwayInfos).size();
    for (auto it = (*(carInfos)).cbegin(); it != (*(carInfos)).cend(); ++it) {
        (*(*highwayInfos)[it->first]).highwayDataBlocker.lock();
        times.push_back((*(*highwayInfos)[it->first]).infoTime);
        numCars += (*(*carInfos)[it->first]).size();
        (*(*highwayInfos)[it->first]).highwayDataBlocker.unlock();
    }
    sort(times.begin(), times.end());
    string time = times[0];

    cout << "Estatísticas gerais da simulação:" << endl;
    cout << "Número de rodovias: " << numHighways << endl;
    cout << "Número de carros (total): " << numCars << endl;
    calculateTime(time, "contagem de veículos e rodovias");
}

void updateSpeed(map<int, map<string, carData>*> &carInfos, map<int, highwayData*> *highwayInfos) {
    for (auto &hw : carInfos) {
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock(); // bloqueio o acesso à rodovia
        map<string, carData>* carMap = hw.second;
        for (auto& q : *carMap) {
            carData &car = q.second;
            if (car.lastPosition != 0) car.speed = static_cast<int>(car.actualPosition - car.lastPosition); 
            if (car.penultimatePosition != 0) car.acceleration = static_cast<int>(car.actualPosition + car.penultimatePosition -  2 * car.lastPosition);
        }
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock(); // libero o acesso à rodovia
    }
}

void carsOverLimit(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    int overLimit = 0;
    int maxSpeedHw;
    vector<string> times;
    for (auto hw : *carInfos) {
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
        map<string, carData>* carMap = hw.second;
        maxSpeedHw = (*(*highwayInfos)[hw.first]).maxSpeed;
        times.push_back((*(*highwayInfos)[hw.first]).infoTime);
        for (auto car : *carMap) {
            if (abs(car.second.speed) > maxSpeedHw) overLimit++;
        }
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
    }
    sort(times.begin(), times.end());
    string time = times[0];
    cout << "Número de carros acima da velocidade permitida: " << overLimit << endl;
    calculateTime(time, "contagem de carros acima da velocidade");
}

/*
função não trivial, pensar mais
void getExtraInfo(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, map<string, plateData*> *carsExtraInfos, externalAPI &API) {
    for (auto hw : *carInfos) {
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
        map<string, carData>* carMap = hw.second;
        for (auto car : *carMap) API.query_vehicle(car.first);
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
    }
}
*/

void calculateNextPositions(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, int epochs) {
    int vel;
    int maxSpeedCar;
    for (auto hw : *carInfos) {
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
        map<string, carData>* carMap = hw.second;
        maxSpeedCar = (*(*highwayInfos)[hw.first]).carMaxSpeed;
        for (auto car : *carMap) {
            for (int i = 1; i <= epochs; i++) {
                vel = car.second.speed + i * car.second.acceleration;
                if (vel > maxSpeedCar) vel = maxSpeedCar;
                if (vel < maxSpeedCar) vel = - maxSpeedCar;
                if (i == 1) car.second.nextPositions.push_back(car.second.actualPosition + vel);
                else car.second.nextPositions.push_back(car.second.nextPositions.back() + vel);
            }
        }
        (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
    }
}