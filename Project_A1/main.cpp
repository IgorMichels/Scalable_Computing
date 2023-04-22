#include "transformers_Michels.h"
// #include "readFiles.h" // já incluso no import acima
#include "API.h"

int main() {
    map<int, map<string, carData>*> carInfos;
    map<int, highwayData*> highwayInfos;

    auto start = chrono::steady_clock::now();
    readFiles(&carInfos, &highwayInfos, false);
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;

    cout << "Total time: " << totalTime.count() << endl;

    // cout << "GAW8W32" << endl;
    // cout << (*(carInfos[103]))["GAW8W32"].lane << endl;
    // cout << (*(carInfos[103]))["GAW8W32"].actualPosition << endl;
    // cout << (*(carInfos[103]))["GAW8W32"].lastPosition << endl;
    // cout << (*(carInfos[103]))["GAW8W32"].penultimatePosition << endl;

    // for(auto it = (*(carInfos[103])).cbegin(); it != (*(carInfos[103])).cend(); ++it) {
    //     cout << it->first << endl;
    // }

    // cout << endl;
    // cout << (*(highwayInfos[101])).infoTime << endl;
    // cout << (*(highwayInfos[101])).maxSpeed << endl;
    // cout << (*(highwayInfos[101])).carMaxSpeed << endl;

    // countHighways(&highwayInfos);
    // analysisStats(&carInfos, &highwayInfos);

    cout << "\nTestando mock de dados externos" << endl;
    externalAPI &API = externalAPI::getInstance(20, "extraInfoCars.txt");
    
    string plate;
    pair<string, string> name;
    pair<string, string> model;
    pair<string, int> year;

    plate = "GAW8W32";
    API.query_vehicle(plate);

    name = API.get_name();
    model = API.get_model();
    year = API.get_year();

    cout << name.first << ' ' << name.second << endl;
    cout << model.first << ' ' << model.second << endl;
    cout << year.first << ' ' << year.second << endl;

    plate = "LWU9U69";
    API.query_vehicle(plate);

    plate = "WOI8F40";
    API.query_vehicle(plate);

    plate = "YLO6Y90";
    API.query_vehicle(plate);

    name = API.get_name();
    model = API.get_model();
    year = API.get_year();

    cout << name.first << ' ' << name.second << endl;
    cout << model.first << ' ' << model.second << endl;
    cout << year.first << ' ' << year.second << endl;

    name = API.get_name();
    model = API.get_model();
    year = API.get_year();

    cout << name.first << ' ' << name.second << endl;
    cout << model.first << ' ' << model.second << endl;
    cout << year.first << ' ' << year.second << endl;

    name = API.get_name();
    model = API.get_model();
    year = API.get_year();

    cout << name.first << ' ' << name.second << endl;
    cout << model.first << ' ' << model.second << endl;
    cout << year.first << ' ' << year.second << endl;

    name = API.get_name();
    model = API.get_model();
    year = API.get_year();

    cout << name.first << ' ' << name.second << endl;
    cout << model.first << ' ' << model.second << endl;
    cout << year.first << ' ' << year.second << endl;
    
    return 0;

}