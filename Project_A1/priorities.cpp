#include <pthread.h>
#include <thread>

#include "transformers.h"

int main() {
    sched_param sch;
    int epochs = 5;
    int policy;

    map<int, map<string, carData>*> carInfos;
    map<string, plateData*> carsExtraInfos;
    map<int, highwayData*> highwayInfos;
    vector<thread*> threads;
    externalAPI& API = externalAPI::getInstance(20, "extraInfoCars.txt");

    // readFiles(&carInfos, &highwayInfos, false, API);

    thread threadReading(readFiles, &carInfos, &highwayInfos, false, API);
    pthread_getschedparam(threadReading.native_handle(), &policy, &sch);
    sch.sched_priority = 20;
    if (pthread_setschedparam(threadReading.native_handle(), SCHED_FIFO, &sch)) cout << "Failed to setschedparam: " << strerror(errno) << '\n';
    
    
    
    threadReading.join();

    return 0;

}