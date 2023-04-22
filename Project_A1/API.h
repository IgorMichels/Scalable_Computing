#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <mutex>

using namespace std;

struct plateData
{
    string plate;
    string model;
    string name;
    int year;
};

class externalAPI {
private:
public:
    string databaseFilename;
    int maxSizeQueue;
    vector<string> queue;
    plateData lastQueryResults;
    int semaphore = 0;
    mutex semaphoreMutex;

    externalAPI (int maxSize, string filename){
            databaseFilename = filename;
            maxSizeQueue = maxSize;
        }

    static externalAPI& getInstance(int q_size, string filename) {
        static externalAPI instance(q_size, filename);
        return instance;
    }

    void query_next_plate() {
        if ((semaphore > 0) || (queue.size() == 0)) return;

        semaphoreMutex.lock();
        string plate = queue.back();
        queue.pop_back();

        ifstream file(databaseFilename);
        string line;
        vector<string> tokens;
        plateData searched;

        while (getline(file, line)) {
            stringstream ss(line);
            string token;
            while (getline(ss, token, ',')) {
                tokens.push_back(token);
                if (tokens[0] != plate) break;
            }

            if (tokens[0] == plate) {
                searched.plate = tokens[0]       ;
                searched.name  = tokens[1]       ;
                searched.model = tokens[2]       ;
                searched.year  = stoi(tokens[3]) ;
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
        if (queue.size() == maxSizeQueue) return false;

        // placa já está na fila
        if (find(queue.begin(), queue.end(), plate) != queue.end()) return false;
        
        // insere na fila
        queue.insert(queue.begin(), plate);
        
        // tenta fazer a próxima query
        query_next_plate();
        return true;
    }
    
    pair<string, string> get_name(){
        semaphoreMutex.lock();
        pair<string, string> result;
        result.first = lastQueryResults.plate;
        result.second = lastQueryResults.name;
        semaphore--;
        semaphoreMutex.unlock();
        query_next_plate();
        return result;
    }
    
    pair<string, string> get_model(){
        semaphoreMutex.lock();
        pair<string, string> result;
        result.first = lastQueryResults.plate;
        result.second = lastQueryResults.model;
        semaphore--;
        semaphoreMutex.unlock();
        query_next_plate();
        return result;
    }

    pair<string, int> get_year(){
        semaphoreMutex.lock();
        pair<string, int> result;
        result.first = lastQueryResults.plate;
        result.second = lastQueryResults.year;
        semaphore--;
        semaphoreMutex.unlock();
        query_next_plate();
        return result;
    }
};