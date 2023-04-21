#include "readFiles.h"

long int numHighways = 0;
long int numCars = 0;

void countHighways(map<int, highwayData*> *highwayInfos) {
    cout << "Número de rodovias: " << (*highwayInfos).size() << endl;
}

void analysisStats(map<int, map<string, carData>*> *carInfos, map<int, highwayData*> *highwayInfos) {
    vector<string> times;
    long int numCars = 0;
    for (auto it = (*(carInfos)).cbegin(); it != (*(carInfos)).cend(); ++it) {
        (*(*highwayInfos)[it->first]).highwayDataBlocker.lock();
        times.push_back((*(*highwayInfos)[it->first]).infoTime);
        numCars += (*(*carInfos)[it->first]).size();
        (*(*highwayInfos)[it->first]).highwayDataBlocker.unlock();
    }
    cout << "Estatísticas gerais da simulação:" << endl;
    cout << "Número de rodovias: " << (*highwayInfos).size() << endl;
    cout << "Número de carros (total): " << numCars << endl;
    
}