#include <string>
#include <vector>
#include <mutex>

using namespace std;

struct plateData
{
    string model;
    string name;
    int year;
};

struct highwayData {
    int maxSpeed = 0;
    int carMaxSpeed = 0;
    string infoTime = "";
    mutex highwayDataBlocker;
};

struct carData {
    int lane = 0;
    int actualPosition = 0;
    int lastPosition = 0;
    int penultimatePosition = 0;
    bool isInHighway = true;
    int speed = 0;
    int acceleration = 0;
    bool canCrash = false;
    bool extraInfos = false;
};

bool active = true;
mutex extraInfos;