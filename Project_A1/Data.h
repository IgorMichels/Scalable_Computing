#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <map>

using namespace std;


mutex mtx;

struct Data
{
    string plate;
    string model;
    string name;
    int year;
};

class External{
public:
  static External& getInstance(int maxSize) {
    static External instance(maxSize);
    return instance;
  }

private:
    // Declaração dos vetores dos dados à serem gerados
    vector<string> m_names;
    vector<string> m_cars;
    vector<string> m_plates;
    vector<int> m_years;

    // Geradores aleatórios
    mt19937 m_gen;
    uniform_int_distribution<> m_dis_names;
    uniform_int_distribution<> m_dis_cars;
    uniform_int_distribution<> m_dis_plates;
    uniform_int_distribution<> m_dis_years;
  
    // Informações gerais da classe
    vector<Data *> m_Queue;
    string m_genStatus;
    map<string, Data> m_genData;

    void m_read_files(vector<string> * names, vector<string> * cars, vector<string> * plates){

            string data_dir = "Dados/";

            // Leitura do arquivo "names.txt"
            ifstream file_names(data_dir + "names.txt");
            string name;
            while (getline(file_names, name)) {
                (*names).push_back(name);
            }
            file_names.close();
            
            // Leitura do arquivo "cars.txt"
            ifstream file_cars(data_dir + "cars.txt");
            string car;
            while (getline(file_cars, car)) {
                (*cars).push_back(car);
            }
            file_cars.close();
            
            // Leitura do arquivo "plates.txt"
            ifstream file_plates(data_dir + "plates.txt");
            string plate;
            while (getline(file_plates, plate)) {
                (*plates).push_back(plate);
            }
            file_plates.close();

            for(int i = 1990; i <= 2023; i++){
                m_years.push_back(i);
            }
        }

public:
    int m_nProc;
    bool m_Multi;
    int m_maxQ;
    int m_nRef;
    int m_maxSize;
    
private:
    External(int qSize){
        m_read_files(&m_names, &m_cars, &m_plates);
        m_maxSize = qSize;
        m_genStatus = "standing";
        m_nProc = 0;
        m_Multi = false;
        m_maxQ = 0;
        m_nRef = 0;

        // Geradores aleatórios
        random_device rd;
        m_gen = mt19937(rd());
        m_dis_names  = uniform_int_distribution<int>(0, m_names.size()-1);
        m_dis_cars   = uniform_int_distribution<int>(0, m_cars.size()-1);
        m_dis_plates = uniform_int_distribution<int>(0, m_plates.size()-1);
        m_dis_years  = uniform_int_distribution<int>(0, m_years.size()-1);
    }

    void m_Generator(){

        while(m_Queue.size() > 0){
            m_nProc++;

            // verifica se está usando mais de um processo ao mesmo tempo
            if(m_nProc > 1){
                m_Multi = true;
            }

            Data * vehiData = m_Queue.back();
            string plate = m_plates[m_dis_plates(m_gen)];

            // Seleciona uniformemente dados aleatórios de uma lista pré determinada
            (*vehiData).plate = plate                        ;
            (*vehiData).model   = m_cars[m_dis_cars(m_gen)]    ;
            (*vehiData).name  = m_names[m_dis_names(m_gen)]  ;
            (*vehiData).year  = m_years[m_dis_years(m_gen)]  ;

            m_Queue.pop_back();

            m_nProc--;
        }
        m_genStatus = "standing";
    }

public:
    void request(Data * car){

        if(m_Queue.size() >= m_maxSize){
            m_nRef++;
        }

        else{
            m_Queue.insert(m_Queue.begin(), car);
            if(m_maxQ < m_Queue.size()){m_maxQ = m_Queue.size();}

            mtx.lock();
            if(m_genStatus == "standing"){
                m_genStatus = "running";
                m_Generator();
            }
            else{}
            mtx.unlock();
        }
    }
};