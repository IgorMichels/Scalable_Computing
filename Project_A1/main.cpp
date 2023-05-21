#include <pthread.h>

#include "transformers.h"
using namespace std;

int main() {
    /*
    Os parâmetros abaixo são referentes ao número de iterações a frente que iremos
    calcular para verificar colisões e ao tamanho máximo da fila da API.
    */
    int epochs = 5;
    API.maxSizeQueue = 5;
    
    int policy;
    sched_param sch;
    map<int, map<string, carData>*> carInfos;
    map<string, plateData> carsExtraInfos;
    map<int, highwayData*> highwayInfos;

    thread threadReading(readFiles, &carInfos, &highwayInfos, true, 3);
    thread threadSpeed1(updateSpeed, &carInfos, &highwayInfos);
    thread threadSpeed2(updateSpeed, &carInfos, &highwayInfos);
    thread threadSpeed3(updateSpeed, &carInfos, &highwayInfos);
    thread threadCrash1(calculateCrash, &carInfos, &highwayInfos, epochs);
    thread threadCrash2(calculateCrash, &carInfos, &highwayInfos, epochs);
    thread threadCrash3(calculateCrash, &carInfos, &highwayInfos, epochs);
    thread threadLimit(carsOverLimit, &carInfos, &highwayInfos);
    thread threadInfos(updateExtraInfos, &carsExtraInfos);
    thread threadCarsInfos(printCarInfos, &carInfos, &highwayInfos, &carsExtraInfos);
    
    threadReading.join();
    threadSpeed1.join();
    threadSpeed2.join();
    threadSpeed3.join();
    threadCrash1.join();
    threadCrash2.join();
    threadCrash3.join();
    threadLimit.join();
    threadInfos.join();
    threadCarsInfos.join();
    
    return 0;

}