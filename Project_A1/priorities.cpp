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
    

    thread threadReading(readFiles, &carInfos, &highwayInfos, true); // wt ok
    

    
    thread threadSpeed(updateSpeed, &carInfos, &highwayInfos); // wt ok
    thread threadCrash(calculateCrash, &carInfos, &highwayInfos, epochs); // wt ok
    // thread threadInfos(updateExtraInfos, &carsExtraInfos); // wt ok
    thread threadCarsInfos(printCarInfos, &carInfos, &highwayInfos, &carsExtraInfos);
    
    
    
    thread threadLimit(carsOverLimit, &carInfos, &highwayInfos); // wt ok
    thread threadStats(analysisStats, &carInfos, &highwayInfos); // wt ok
    
    /*
    pthread_getschedparam(threadReading.native_handle(), &policy, &sch);
    sch.sched_priority = 99;
    if (pthread_setschedparam(threadReading.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
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