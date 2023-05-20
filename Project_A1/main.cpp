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

    thread threadReading(readFiles, &carInfos, &highwayInfos, false);
    pthread_getschedparam(threadReading.native_handle(), &policy, &sch);
    sch.sched_priority = 99;
    if (pthread_setschedparam(threadReading.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
    
    thread threadSpeed(updateSpeed, &carInfos, &highwayInfos);
    pthread_getschedparam(threadSpeed.native_handle(), &policy, &sch);
    sch.sched_priority = 99;
    if (pthread_setschedparam(threadSpeed.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';

    thread threadCrash(calculateCrash, &carInfos, &highwayInfos, epochs);
    pthread_getschedparam(threadCrash.native_handle(), &policy, &sch);
    sch.sched_priority = 99;
    if (pthread_setschedparam(threadCrash.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';

    thread threadLimit(carsOverLimit, &carInfos, &highwayInfos);
    pthread_getschedparam(threadLimit.native_handle(), &policy, &sch);
    sch.sched_priority = 5;
    if (pthread_setschedparam(threadLimit.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';

    thread threadInfos(updateExtraInfos, &carsExtraInfos);
    pthread_getschedparam(threadInfos.native_handle(), &policy, &sch);
    sch.sched_priority = 1;
    if (pthread_setschedparam(threadInfos.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
    
    thread threadCarsInfos(printCarInfos, &carInfos, &highwayInfos, &carsExtraInfos);
    pthread_getschedparam(threadCarsInfos.native_handle(), &policy, &sch);
    sch.sched_priority = 1;
    if (pthread_setschedparam(threadCarsInfos.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';

    threadReading.join();
    threadSpeed.join();
    threadCrash.join();
    threadLimit.join();
    threadInfos.join();
    threadCarsInfos.join();
    
    return 0;

}