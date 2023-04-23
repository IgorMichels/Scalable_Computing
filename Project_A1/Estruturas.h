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


map<int, int> countCarsByHighway(map<int, map<string, carData>*> carInfos,
                                 map<int, highwayData*> * highwayInfos) {

    map<int, int> cars_per_highway;

    for (const auto& p : carInfos) {
        (*(*highwayInfos)[p.first]).highwayDataBlocker.lock(); // bloqueio o acesso à rodovia
        
        int key = p.first; 
        map<string, carData>* carMap = p.second; 
        int carCount = 0;
        for (const auto& q : *carMap) {
            carData car = q.second;
            carCount++;
        }
        cars_per_highway[key] = carCount;
        (*(*highwayInfos)[p.first]).highwayDataBlocker.unlock(); // libero o acesso à rodovia
    }
    return cars_per_highway;
}

map<int, int> countCarsOverLim(map<int, map<string, carData>*> carInfos, map<int, highwayData*> highwayInfos) {

    map<int, int> cars_over_lim;

    for (const auto& p : carInfos) {
        (*highwayInfos[p.first]).highwayDataBlocker.lock(); // bloqueio o acesso à rodovia

        int key = p.first;
        map<string, carData>* carMap = p.second; 
        int maxSpeed = highwayInfos[key]->maxSpeed;
        int carCount = 0;
        for (const auto& q : *carMap) {
            carData car = q.second; 
            if(maxSpeed < car.speed){
                carCount++;
            }
        }
        cars_over_lim[key] = carCount;
        (*highwayInfos[p.first]).highwayDataBlocker.unlock(); // libero o acesso à rodovia
    }
    return cars_over_lim;
}


void calculateSpeedAndAcceleration(map<int, map<string, carData>*>& carInfos,
                                        map<int, highwayData*>* highwayInfos) {
    
    for (auto& p : carInfos) {
        (*(*highwayInfos)[p.first]).highwayDataBlocker.lock(); // bloqueio o acesso à rodovia
        map<string, carData>* carMap = p.second;
        for (auto& q : *carMap) {
            carData& car = q.second;
            int currentPosition = car.actualPosition;
            int lastPosition = car.lastPosition;
            int penultimatePosition = car.penultimatePosition;
            double timeInterval = 1.0; // por enquanto estamos difinindo o intervalo de tempo como 1 segundo (deve ser ajustado para o tempo entre dois estados)

            // Calcula a velocidade
            double velocity = (currentPosition - lastPosition) / timeInterval;
            car.speed = static_cast<int>(velocity); 

            // Calcula a aceleração
            double lastVelocity = (lastPosition - penultimatePosition) / timeInterval;
            double acceleration = (velocity - lastVelocity) / timeInterval;
            car.acceleration = static_cast<int>(acceleration);
        }
        (*(*highwayInfos)[p.first]).highwayDataBlocker.unlock(); // libero o acesso à rodovia
    }
}

// função para ordenar vetor de tuplas
bool compare_second(const tuple<string, int, int>& a, const tuple<string, int, int>& b) {
    return get<1>(a) < get<1>(b);
}

map<string, string> checkCollision(map<int, map<string, carData>*>& carInfos,
                                map<int, highwayData*>* highwayInfos, int dt=1){

    /*  Função para verificar quais carros podem colidir mantendo aceleração e pista constantes. 
    Caso o carro atinja a velocidade máxima ele para de acelerar. Usamos a fórmula do MRUV até o momento
    em que o carro para de acelerar e MRU dali em seguida.

    * @param carInfos: um dicionário onde a chave corresponde a rodovia e o valor é um dicionário com carros;
    * @param highwayInfos: informações das rodovias;
    * @param dt: intervalo de tempo. default=1. */


    map<string, string> crashs;
    carData _info;
    double t_lim_speed;
    double S = 0;
    int maxSpeed;
    
    for (auto& p : carInfos) {
        (*(*highwayInfos)[p.first]).highwayDataBlocker.lock(); // bloqueia o acesso à rodovia

        maxSpeed = (*highwayInfos)[p.first]->carMaxSpeed;
        map<int, vector<tuple<string, int, int>>> cars_lane; //[plate, curPos, futPos]
        map<string, carData>* carMap = p.second;
        for (auto it : *carMap) {
            _info = it.second;
            if(_info.speed == 0){
                t_lim_speed = dt;
            }
            else if ((maxSpeed - _info.speed) / (double)_info.speed > dt){
                t_lim_speed = dt;
            }
            else{
                t_lim_speed = (maxSpeed - _info.speed) / (double)_info.speed;
            }
            S = _info.actualPosition + _info.speed * dt + _info.acceleration * t_lim_speed * t_lim_speed / 2;
            tuple<string, int, int> _cur = {it.first, _info.actualPosition, S};
            cars_lane[_info.lane].push_back(_cur);
        }

        for(auto lane : cars_lane){
            vector<tuple<string, int, int>> e = lane.second;
            sort(e.begin(), e.end(), compare_second);
            for(int i = 0; i < e.size() - 1; i++){
                if (get<2>(e[i]) >= get<2>(e[i+1])){
                    crashs[get<0>(e[i])] = get<0>(e[i+1]);
                    (*carMap)[get<0>(e[i])].canCrash = true;
                    (*carMap)[get<0>(e[i+1])].canCrash = true;
                    cout << get<0>(e[i]) << " pode bater no " << get<0>(e[i+1]) << endl;
                }
            }
        }
        (*(*highwayInfos)[p.first]).highwayDataBlocker.unlock(); // libera o acesso à rodovia
    }
    return crashs;
}


/*
void updateMaps(map<int, map<string, carData>*>& carInfos, map<int, highwayData*>* highwayInfos,
                externalAPI * API){

    for(auto& p : carInfos){
        // (*(*highwayInfos)[p.first]).highwayDataBlocker.lock(); // bloqueia o acesso à rodovia

        map<string, carData>* carMap = p.second;
        for(auto it : *carMap){
            (*API).query_vehicle(it.first);

            pair<string, string> model = (*API).get_model();
            (*carMap)[model.first].model = model.second;

            pair<string, string> name = (*API).get_name();
            (*carMap)[name.first].name = name.second;

            pair<string, int> year = (*API).get_year();
            (*carMap)[year.first].year = year.second;
        }
        // (*(*highwayInfos)[p.first]).highwayDataBlocker.unlock(); // libera o acesso à rodovia
    }
}
*/


void analise(map<int, map<string, carData>*> carInfos, map<int, highwayData*> highwayInfos,
            map<int, int>* cars_per_highway, map<int, int>* cars_over_lim, int* total_cars){
    
    *cars_per_highway = countCarsByHighway(carInfos, &highwayInfos);
    *cars_over_lim = countCarsOverLim(carInfos, highwayInfos);
    *total_cars = 0;
    for(auto e : *cars_per_highway){
        *total_cars += e.second;
    }

}