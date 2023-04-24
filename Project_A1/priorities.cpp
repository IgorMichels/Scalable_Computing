#include <pthread.h>
#include <thread>

#include "transformers.h"
using namespace std;

int main() {
    sched_param sch;
    int policy;
    int epochs = 5;
    map<int, map<string, carData>*> carInfos;
    map<string, plateData> carsExtraInfos;
    map<int, highwayData*> highwayInfos;
    
    /*
    readFiles(&carInfos, &highwayInfos, false);
    analysisStats(&carInfos, &highwayInfos);
    carsOverLimit(&carInfos, &highwayInfos);
    
    updateSpeed(carInfos, &highwayInfos);
    calculateCrash(carInfos, &highwayInfos, epochs);
    updateExtraInfos(carsExtraInfos);
    printCarInfos(carInfos, &highwayInfos, carsExtraInfos);
    */
    

    thread threadReading(readFiles, &carInfos, &highwayInfos, true);
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

    /*
    thread threadCarsInfos(printCarInfos, &carInfos, &highwayInfos, &carsExtraInfos);
    pthread_getschedparam(threadCarsInfos.native_handle(), &policy, &sch);
    sch.sched_priority = 10;
    if (pthread_setschedparam(threadCarsInfos.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
    */
    
    thread threadLimit(carsOverLimit, &carInfos, &highwayInfos);
    pthread_getschedparam(threadLimit.native_handle(), &policy, &sch);
    sch.sched_priority = 10;
    if (pthread_setschedparam(threadLimit.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';

    thread threadStats(analysisStats, &carInfos, &highwayInfos);
    pthread_getschedparam(threadStats.native_handle(), &policy, &sch);
    sch.sched_priority = 10;
    if (pthread_setschedparam(threadStats.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
    
    /*
    thread threadInfos(updateExtraInfos, &carsExtraInfos);
    pthread_getschedparam(threadInfos.native_handle(), &policy, &sch);
    sch.sched_priority = 1;
    if (pthread_setschedparam(threadInfos.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
    */
    
    

    threadReading.join();
    

    /*
    threadSpeed.join();
    threadCrash.join();
    threadInfos.join();
    threadCarsInfos.join();
    */
    
    
    threadLimit.join();
    threadStats.join();
    
    return 0;

}