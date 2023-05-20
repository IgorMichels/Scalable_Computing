#include "readFiles.h"

void calculateTime(string time, string function) {
    tm tm{};
    stringstream ss{time};
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto tp = chrono::system_clock::from_time_t(mktime(&tm)) + chrono::microseconds{std::stoi(time.substr(20))};
    auto now = chrono::system_clock::now();
    auto duration = chrono::duration_cast<std::chrono::microseconds>(now - tp);
    cout << "Tempo de processamento (" << function << "): " << duration.count() / 1000.0 << " milisegundos" << endl;
}

void carsOverLimit(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    int maxSpeedHw;
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
            for (auto hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                (*(*highwayInfos)[hw.first]).timeOverSpeed = (*(*highwayInfos)[hw.first]).infoTime;
                map<string, carData>* carMap = hw.second;
                maxSpeedHw = (*(*highwayInfos)[hw.first]).maxSpeed;
                for (auto car : *carMap) {
                    if (abs(car.second.speed) > maxSpeedHw) car.second.overSpeed = true;
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }
        }
    }
}

bool comparePosition(const tuple<string, vector<int>> &positionsCar1, const tuple<string, vector<int>> &positionsCar2) {
    return get<1>(positionsCar1)[0] < get<1>(positionsCar2)[0];
}

void updateSpeed(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                map<string, carData>* carMap = hw.second;
                for (auto &q : *carMap) {
                    carData &car = q.second;
                    if (car.lastPosition != 0) car.speed = static_cast<int>(car.actualPosition - car.lastPosition); 
                    if (car.penultimatePosition != 0) car.acceleration = static_cast<int>(car.actualPosition + car.penultimatePosition -  2 * car.lastPosition);
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }
        }
    }
}

void calculateCrash(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, int epochs) {
    int vel;
    int maxSpeedCar;
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                (*(*highwayInfos)[hw.first]).timeCrash = (*(*highwayInfos)[hw.first]).infoTime;
                maxSpeedCar = (*(*highwayInfos)[hw.first]).carMaxSpeed;
                map<int, vector<tuple<string, vector<int>>>> lanes;
                map<string, carData>* carMap = hw.second;
                for (auto &q : *carMap) {
                    vector<int> nextPositions;
                    carData &car = q.second;
                    for (int i = 0; i <= epochs; i++) {
                        if (i == 0) {
                            nextPositions.push_back(car.actualPosition);
                            continue;
                        }
                        vel = car.speed + i * car.acceleration;
                        if (vel > maxSpeedCar) vel = maxSpeedCar;
                        if (vel < maxSpeedCar) vel = - maxSpeedCar;
                        nextPositions.push_back(nextPositions.back() + vel);
                    }
                    tuple<string, vector<int>> currentCar = {q.first, nextPositions};
                    lanes[car.lane].push_back(currentCar);
                    nextPositions.clear();
                }
                
                for (auto lane : lanes){
                    vector<tuple<string, vector<int>>> actualLane = lane.second;
                    sort(actualLane.begin(), actualLane.end(), comparePosition);
                    for (int i = 0; i < actualLane.size() - 1; i++){
                        for (int j = 1; j <= epochs; j++) {
                            if (get<1>(actualLane[i])[j] >= get<1>(actualLane[i + 1])[j]){
                                (*carMap)[get<0>(actualLane[i])].canCrash = true;
                                (*carMap)[get<0>(actualLane[i + 1])].canCrash = true;
                                break;
                            }
                        }
                    }
                }

                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }
        }
    }
}

void updateExtraInfos(map<string, plateData> *carsExtraInfos) {
    plateData actualPlate;
    string plate;
    string model;
    string name;
    bool update;
    int year;
    while (true) {
        if (active) {
            API.semaphoreMutex.lock();
            update = (API.queue.size() > 0) || (API.semaphore > 0);
            API.semaphoreMutex.unlock();
            while (update) {
                tie(plate, name) = API.get_name();
                tie(plate, year) = API.get_year();
                tie(plate, model) = API.get_model();

                actualPlate.name = name;
                actualPlate.year = year;
                actualPlate.model = model;
                extraInfos.lock();
                (*carsExtraInfos)[plate] = actualPlate;
                extraInfos.unlock();

                API.semaphoreMutex.lock();
                update = (API.queue.size() > 0) || (API.semaphore > 0);
                API.semaphoreMutex.unlock();
            }
        }
    }
}

void printCarInfos(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, map<string, plateData> *carsExtraInfos) {
    string time;
    string analysisTime;
    vector<string> times;
    map<int, vector<string>> analysisTimesHighways;
    vector<float> analysisTotalTime;
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
            long int numCars = 0;
            long int numHighways = (*highwayInfos).size();
            for (auto it = (*(carInfos)).cbegin(); it != (*(carInfos)).cend(); ++it) {
                (*(*highwayInfos)[it->first]).highwayDataBlocker.lock();
                times.push_back((*(*highwayInfos)[it->first]).infoTime);
                numCars += (*(*carInfos)[it->first]).size();
                (*(*highwayInfos)[it->first]).highwayDataBlocker.unlock();
            }

            sort(times.begin(), times.end());
            time = times[0];
            cout << "Estatísticas gerais da simulação:" << endl;
            cout << "Número de rodovias: " << numHighways << endl;
            cout << "Número de carros (total): " << numCars << endl;
            calculateTime(time, "contagem de veículos e rodovias");
            times.clear();

            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                map<string, carData>* carMap = hw.second;
                times.push_back((*(*highwayInfos)[hw.first]).timeCrash);
                cout << "Carros com risco de colisão na rodovia " << hw.first << ":" << endl << endl;
                for (auto &q : *carMap) {
                    carData &car = q.second;
                    if (car.canCrash) cout << q.first << endl;
                    }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
                }

            sort(times.begin(), times.end());
            time = times[0];
            calculateTime(time, "possíveis colisões");
            times.clear();

            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                map<string, carData>* carMap = hw.second;
                times.push_back((*(*highwayInfos)[hw.first]).timeOverSpeed);
                cout << "Carros acima da velocidade permitida na rodovia " << hw.first << ":" << endl << endl;
                for (auto &q : *carMap) {
                    carData &car = q.second;
                    if (car.overSpeed) cout << q.first << endl;
                    }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
                }

            sort(times.begin(), times.end());
            time = times[0];
            calculateTime(time, "veículos acima da velocidade");
            times.clear();

            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                map<string, carData>* carMap = hw.second;
                cout << "Rodovia: " << hw.first << endl << endl;
                for (auto &q : *carMap) {
                    carData &car = q.second;
                    cout << q.first << ":\n  Posição GPS: (" << car.actualPosition << ", " << car.lane << ")\n"
                         << "  Velocidade: " << car.speed << "\n"
                         << "  Aceleração: " << car.acceleration << "\n"
                         << "  Mantidas essas características, ele " << (car.canCrash ? "poderá bater" : "não irá bater")
                         << endl;
                    if (car.extraInfos) {
                        extraInfos.lock();
                        cout << " Informações extras:\n"
                             << "  Proprietário: " << (*carsExtraInfos)[q.first].name << "\n"
                             << "  Model: " << (*carsExtraInfos)[q.first].model << "\n"
                             << "  Ano: " << (*carsExtraInfos)[q.first].year
                             << endl;
                        extraInfos.unlock();
                    }
                    cout << "-----------------------------------------------------------" << endl;
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }

            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                map<string, carData>* carMap = hw.second;
                if (((*(*highwayInfos)[hw.first]).timeCrash != "") & ((*(*highwayInfos)[hw.first]).timeOverSpeed != "")) {
                    if ((*(*highwayInfos)[hw.first]).timeCrash.compare((*(*highwayInfos)[hw.first]).timeOverSpeed) < 0) analysisTime = (*(*highwayInfos)[hw.first]).timeCrash;
                    else analysisTime = (*(*highwayInfos)[hw.first]).timeOverSpeed;
                    if ((analysisTimesHighways[hw.first]).size() == 0) analysisTimesHighways[hw.first].push_back(analysisTime);
                    if (analysisTime.compare((analysisTimesHighways[hw.first]).back()) != 0) analysisTimesHighways[hw.first].push_back(analysisTime);
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
                calculateTime(analysisTime, "cálculo total das análises");
            }
        }
    }
}