#include "readFiles.h"
#include "API.h"

int main() {
    map<int, map<string, carData>*> carInfos;
    map<int, highwayData*> highwayInfos;

    auto start = chrono::steady_clock::now();
    readFiles(&carInfos, &highwayInfos, false);
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;

    cout << "Total time: " << totalTime.count() << endl;

    cout << "GAW8W32" << endl;
    cout << (*(carInfos[103]))["GAW8W32"].lane << endl;
    cout << (*(carInfos[103]))["GAW8W32"].actualPosition << endl;
    cout << (*(carInfos[103]))["GAW8W32"].lastPosition << endl;
    cout << (*(carInfos[103]))["GAW8W32"].penultimatePosition << endl;

    // for(auto it = (*(carInfos[103])).cbegin(); it != (*(carInfos[103])).cend(); ++it) {
    //     cout << it->first << endl;
    // }

    cout << endl;
    cout << (*(highwayInfos[101])).infoTime << endl;
    cout << (*(highwayInfos[101])).maxSpeed << endl;
    cout << (*(highwayInfos[101])).carMaxSpeed << endl;

    /*
    cout << "\nTestando mock de dados externos" << endl;
    Api API;
    string plate = "GAW8W32";
    API.query_vehicle(plate);
    cout << API.get_name() << endl;
    cout << API.get_model() << endl;
    cout << API.get_year() << endl;

    (*(carInfos[101]))[plate].name = API.get_name();
    (*(carInfos[101]))[plate].model = API.get_model();
    (*(carInfos[101]))[plate].year = API.get_year();

    cout << (*(carInfos[101]))[plate].name << ' ' << API.get_name() << endl;
    cout << (*(carInfos[101]))[plate].model << ' ' << API.get_model() << endl;
    cout << (*(carInfos[101]))[plate].year << ' ' << API.get_year() << endl;

    cout << endl;
    */
    return 0;

}