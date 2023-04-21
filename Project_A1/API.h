#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <map>

using namespace std;


struct Data
{
    string plate;
    string model;
    string name;
    int year;
};

class Api {
private:
    string m_filename;
    int m_Q_Size;
    vector<string> m_Queue;
    Data m_last_query;
    mutex insert_mutex;
    mutex search_mutex;
    mutex name_mutex;
    mutex model_mutex;
    mutex year_mutex;

    void query_vehicle(string plate) {
        name_mutex.lock();
        model_mutex.lock();
        year_mutex.lock();

        // forçando a fila a encher
        this_thread::sleep_for(chrono::milliseconds(100));

        ifstream file(m_filename);
        string line;
        vector<string> tokens;
        Data searched_car;

        while (getline(file, line)) {
            stringstream ss(line);
            string token;

            while (getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens[0] == plate) {
                searched_car.plate = tokens[0]       ;
                searched_car.name  = tokens[1]       ;
                searched_car.model = tokens[2]       ;
                searched_car.year  = stoi(tokens[3]) ;
                break;
            }
            tokens.clear();
        }
        file.close();
        m_last_query = searched_car;
    }

    Api(int q_size, string filename){
            m_filename = filename;
            m_Q_Size = q_size;
            max_Q_Size = 0;
        }

public:
    int max_Q_Size;

    static Api& getInstance(int q_size, string filename) {
        static Api instance(q_size, filename);
        return instance;
    }


    void m_insert(string plate){
        insert_mutex.lock();
        if(m_Queue.size() < m_Q_Size){
            m_Queue.insert(m_Queue.begin(), plate);
            if(max_Q_Size < m_Queue.size()){
                max_Q_Size = m_Queue.size();
            }
            insert_mutex.unlock();
            query_vehicle(plate);
        }
        else{
            insert_mutex.unlock();
        }
    }

    string get_name(){
        name_mutex.unlock();
        return m_last_query.name;
    }
    string get_model(){
        model_mutex.unlock();
        return m_last_query.model;
    }
    int get_year(){
        m_Queue.pop_back();
        year_mutex.unlock();
        return m_last_query.year;
    }

    void m_execute(vector<Data *> * cars, Data * car){
        m_insert((*car).plate);
        (*car).name  = get_name()  ;
        (*car).model = get_model() ;
        (*car).year  = get_year()  ;
        (*cars).push_back(car);
    }
};