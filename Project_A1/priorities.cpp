#include "transformers.h"
// #include "readFiles.h" // já incluso no import acima
// #include "API.h"

int main() {
    /*
    Testar as funções sem passar ponteiro, mas com o objeto carInfos
    */
    // thread th_read       ;
    // thread th_updtSpeed  ;
    // thread th_overSpeed  ;
    // thread th_crash      ;


    int epochs = 5;

    map<int, map<string, carData>*> carInfos;
    map<string, plateData*> carsExtraInfos;
    map<int, highwayData*> highwayInfos;
    externalAPI& API = externalAPI::getInstance(20, "extraInfoCars.txt");

    auto start = chrono::steady_clock::now();
    thread th_read(readFiles, &carInfos, &highwayInfos, false, 1, API);
    th_read.join();
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;

    cout << "Total time: " << totalTime.count() << endl;
    
    start = chrono::steady_clock::now();
    thread th_updtSpeed(updateSpeed, carInfos, &highwayInfos);
    th_updtSpeed.join();
    /*
    cout << "GAW8W32" << endl;
    cout << (*(carInfos[103]))["GAW8W32"].lane << endl;
    cout << (*(carInfos[103]))["GAW8W32"].actualPosition << endl;
    cout << (*(carInfos[103]))["GAW8W32"].lastPosition << endl;
    cout << (*(carInfos[103]))["GAW8W32"].penultimatePosition << endl;
    cout << (*(carInfos[103]))["GAW8W32"].speed << endl;
    cout << (*(carInfos[103]))["GAW8W32"].acceleration << endl;
    */
    end = chrono::steady_clock::now();totalTime = end - start;
    cout << "Total time (update): " << totalTime.count() << endl;

    start = chrono::steady_clock::now();
    thread th_overSpeed(carsOverLimit, &carInfos, &highwayInfos);
    end = chrono::steady_clock::now();
    totalTime = end - start;
    cout << "Total time (cars): " << totalTime.count() << endl;

    // calculateNextPositions(carInfos, &highwayInfos, epochs);
    // cout << (*(carInfos[104]))["IWU4J03"].nextPositions.size() << endl;
    thread th_crash(calculateCrash, carInfos, &highwayInfos, epochs);


      // Define as prioridades das threads
    th_overSpeed.native_handle();
    sched_param sch_over = {1}; // Prioridade 1
    pthread_setschedparam(th_overSpeed.native_handle(), SCHED_FIFO, &sch_over);

    th_crash.native_handle();
    sched_param sch_crash = {2}; // Prioridade 2
    pthread_setschedparam(th_crash.native_handle(), SCHED_FIFO, &sch_crash);

    // th_updtSpeed.native_handle();
    // sched_param sch_upd = {3}; // Prioridade 3
    // pthread_setschedparam(th_updtSpeed.native_handle(), SCHED_FIFO, &sch_upd);

    // th_read.native_handle(); 
    // sched_param sch_read = {4}; // Prioridade 4
    // pthread_setschedparam(th_read.native_handle(), SCHED_FIFO, &sch_read);


    th_overSpeed.join()  ;
    th_crash.join()      ;
    

    // for(auto it = (*(carInfos[103])).cbegin(); it != (*(carInfos[103])).cend(); ++it) {
    //     cout << it->first << endl;
    // }

    // cout << endl;
    // cout << (*(highwayInfos[101])).infoTime << endl;
    // cout << (*(highwayInfos[101])).maxSpeed << endl;
    // cout << (*(highwayInfos[101])).carMaxSpeed << endl;

    // countHighways(&highwayInfos);
    // analysisStats(&carInfos, &highwayInfos);

    
    
    // cout << "\nTestando mock de dados externos" << endl;
    
    // string plate;
    // pair<string, string> name;
    // pair<string, string> model;
    // pair<string, int> year;

    // plate = "GAW8W32";
    // API.query_vehicle(plate);

    // name = API.get_name();
    // model = API.get_model();
    // year = API.get_year();

    // cout << name.first << ' ' << name.second << endl;
    // cout << model.first << ' ' << model.second << endl;
    // cout << year.first << ' ' << year.second << endl;

    // plate = "LWU9U69";
    // API.query_vehicle(plate);

    // plate = "WOI8F40";
    // API.query_vehicle(plate);

    // plate = "TBM0Z27";
    // API.query_vehicle(plate);

    // plate = "TBM0Z27";
    // API.query_vehicle(plate);

    // plate = "TBM0Z27";
    // API.query_vehicle(plate);

    // plate = "TBM0Z27";
    // API.query_vehicle(plate);

    // plate = "TBM0Z27";
    // API.query_vehicle(plate);

    // name = API.get_name();
    // model = API.get_model();
    // year = API.get_year();

    // cout << name.first << ' ' << name.second << endl;
    // cout << model.first << ' ' << model.second << endl;
    // cout << year.first << ' ' << year.second << endl;

    // name = API.get_name();
    // model = API.get_model();
    // year = API.get_year();

    // cout << name.first << ' ' << name.second << endl;
    // cout << model.first << ' ' << model.second << endl;
    // cout << year.first << ' ' << year.second << endl;

    // name = API.get_name();
    // model = API.get_model();
    // year = API.get_year();

    // cout << name.first << ' ' << name.second << endl;
    // cout << model.first << ' ' << model.second << endl;
    // cout << year.first << ' ' << year.second << endl;

    // name = API.get_name();
    // model = API.get_model();
    // year = API.get_year();

    // cout << name.first << ' ' << name.second << endl;
    // cout << model.first << ' ' << model.second << endl;
    // cout << year.first << ' ' << year.second << endl;
    
    

    return 0;

}