#include "readFiles.h"

#include <iostream>
#include <sstream>
#include <chrono>

void calculateTime(string time) {
    tm tm{};
    stringstream ss{time};
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto tp = chrono::system_clock::from_time_t(mktime(&tm)) +
              chrono::microseconds{std::stoi(time.substr(20))};

    auto now = chrono::system_clock::now();
    auto duration = chrono::duration_cast<std::chrono::microseconds>(now - tp);
    cout << "Tempo de processamento: " << duration.count() / 1000.0 << " milisegundos" << endl;
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
    calculateTime(time);
}
