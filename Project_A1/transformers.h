#include "readFiles.h"

void calculateTime(string time, string function) {
    tm tm{};
    stringstream ss{time};
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto tp = chrono::system_clock::from_time_t(mktime(&tm));
    try {
        tp = tp + chrono::microseconds{std::stoi(time.substr(20))};
    }
    catch (...) {
        cout << time << endl;
        abort;
    }

    auto now = chrono::system_clock::now();
    auto duration = chrono::duration_cast<std::chrono::microseconds>(now - tp);
    cout << "Tempo de processamento (" << function << "): " << duration.count() / 1000.0 << " milisegundos" << endl;
}

void analysisStats(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    vector<string> times;
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
            string time = times[0];
            cout << "Estatísticas gerais da simulação:" << endl;
            cout << "Número de rodovias: " << numHighways << endl;
            cout << "Número de carros (total): " << numCars << endl;
            calculateTime(time, "contagem de veículos e rodovias");
            times.clear();
        }
        // break;
    }
}

void carsOverLimit(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    int maxSpeedHw;
    vector<string> times;
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
            for (auto hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                map<string, carData>* carMap = hw.second;
                maxSpeedHw = (*(*highwayInfos)[hw.first]).maxSpeed;
                times.push_back((*(*highwayInfos)[hw.first]).infoTime);
                for (auto car : *carMap) {
                    if (abs(car.second.speed) > maxSpeedHw) continue; // cout << car.first << " está acima da velocidade máxima permitida" << endl;
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }
            sort(times.begin(), times.end());
            string time = times[0];
            calculateTime(time, "contagem de carros acima da velocidade");
            times.clear();
        }
        // break;
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
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock(); // bloqueio o acesso à rodovia
                map<string, carData>* carMap = hw.second;
                for (auto &q : *carMap) {
                    carData &car = q.second;
                    if (car.lastPosition != 0) car.speed = static_cast<int>(car.actualPosition - car.lastPosition); 
                    if (car.penultimatePosition != 0) car.acceleration = static_cast<int>(car.actualPosition + car.penultimatePosition -  2 * car.lastPosition);
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock(); // libero o acesso à rodovia
            }
        }
        // break;
    }
}

void calculateCrash(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, int epochs) {
    int vel;
    int maxSpeedCar;
    vector<string> times;
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
            for (auto &hw : *carInfos) {
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.lock();
                times.push_back((*(*highwayInfos)[hw.first]).infoTime);
                maxSpeedCar = (*(*highwayInfos)[hw.first]).carMaxSpeed;
                map<int, vector<tuple<string, vector<int>>>> lanes; // lane : plate, positions
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
                                // cout << get<0>(actualLane[i]) << " pode bater no " << get<0>(actualLane[i + 1]) << " daqui a " << j << " iteração(ões)" << endl;
                                break;
                            }
                        }
                    }
                }

                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }
            sort(times.begin(), times.end());
            string time = times[0];
            calculateTime(time, "possíveis colisões");
            times.clear();
        }
        // break;
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
                cout << name << " " << model << " " << year << endl;
                (*carsExtraInfos)[plate] = actualPlate;

                API.semaphoreMutex.lock();
                update = (API.queue.size() > 0) || (API.semaphore > 0);
                API.semaphoreMutex.unlock();
            }
        }
        // break;
    }
}

void printCarInfos(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos, map<string, plateData> *carsExtraInfos) {
    while (true) {
        if (active) {
            while ((*carInfos).size() == 0) this_thread::sleep_for(chrono::milliseconds(100));
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
                        cout << " Informações extras:\n"
                            << "  Proprietário: " << (*carsExtraInfos)[q.first].name << "\n"
                            << "  Model: " << (*carsExtraInfos)[q.first].model << "\n"
                            << "  Ano: " << (*carsExtraInfos)[q.first].year
                            << endl;
                    }
                    cout << "-----------------------------------------------------------" << endl;
                }
                (*(*highwayInfos)[hw.first]).highwayDataBlocker.unlock();
            }
        }
        // break;
    }
}