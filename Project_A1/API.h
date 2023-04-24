#include <iostream>
#include <fstream>
#include <sstream>
//#include <vector>
//#include <string>
//#include <mutex>

#include "structs.h"

using namespace std;

class externalAPI {
private:
public:
    plateData lastQueryResults;
    string databaseFilename;
    vector<string> queue;
    mutex semaphoreMutex;
    int semaphore = 0;
    int sizeQueue = 0;
    int maxSizeQueue;
    string lastPlate;
    bool ifEntrance;

    externalAPI (int maxSize, string filename){
            databaseFilename = filename;
            maxSizeQueue = maxSize;
        }

    static externalAPI& getInstance(int q_size, string filename) {
        static externalAPI instance(q_size, filename);
        return instance;
    }

    void query_next_plate() {
        semaphoreMutex.lock();
        ifEntrance = ((semaphore > 0) || (sizeQueue == 0));
        semaphoreMutex.unlock();
        if (ifEntrance) return;

        cout << semaphore << " e " << sizeQueue << " " << ifEntrance << endl;

        semaphoreMutex.lock();
        string plate = queue.back();
        cout << "nossa plaquinha: " << plate << endl;
        queue.pop_back();
        sizeQueue--;
        cout << "nossa plaquinha: " << plate << endl;

        ifstream file(databaseFilename);
        string line;
        vector<string> tokens;
        plateData searched;

        while (getline(file, line)) {
            stringstream ss(line);
            string token;
            while (getline(ss, token, ',')) {
                // cout << token << " e a placa buscada é " << plate << endl;
                tokens.push_back(token);
                if (tokens[0] != plate) break;
            }

            if (tokens[0] == plate) {
                lastPlate      = tokens[0]       ;
                searched.name  = tokens[1]       ;
                searched.model = tokens[2]       ;
                searched.year  = stoi(tokens[3]) ;
                // cout << "API " << tokens[0] << tokens[1] << tokens[2] << tokens[3] << endl;
                break;
            }
            tokens.clear();

        }
        file.close();
        lastQueryResults = searched;
        semaphore = 3;
        semaphoreMutex.unlock();
    }

    bool query_vehicle(string plate) {
        // fila está cheia
        if (sizeQueue == maxSizeQueue) return false;

        // se a placa não está na fila, a gente insere
        if (find(queue.begin(), queue.end(), plate) == queue.end()) {
            semaphoreMutex.lock();
            queue.insert(queue.begin(), plate);
            sizeQueue++;
            semaphoreMutex.unlock();
        }

        // tenta fazer a próxima query
        query_next_plate();
        return true;
    }
    
    pair<string, string> get_name(){
        semaphoreMutex.lock();
        if (lastQueryResults.year == 0) {
            semaphoreMutex.unlock();
            query_next_plate();
            semaphoreMutex.lock();
        }
        pair<string, string> result;
        result.first = lastPlate;
        result.second = lastQueryResults.name;
        semaphore--;
        semaphoreMutex.unlock();
        query_next_plate();
        return result;
    }
    
    pair<string, string> get_model(){
        semaphoreMutex.lock();
        if (lastQueryResults.year == 0) {
            semaphoreMutex.unlock();
            query_next_plate();
            semaphoreMutex.lock();
        }
        pair<string, string> result;
        result.first = lastPlate;
        result.second = lastQueryResults.model;
        semaphore--;
        semaphoreMutex.unlock();
        query_next_plate();
        return result;
    }

    pair<string, int> get_year(){
        semaphoreMutex.lock();
        if (lastQueryResults.year == 0) {
            semaphoreMutex.unlock();
            query_next_plate();
            semaphoreMutex.lock();
        }
        pair<string, int> result;
        result.first = lastPlate;
        result.second = lastQueryResults.year;
        semaphore--;
        semaphoreMutex.unlock();
        query_next_plate();
        return result;
    }
};

externalAPI &API = externalAPI::getInstance(20, "mockData/extraInfoCars.txt");