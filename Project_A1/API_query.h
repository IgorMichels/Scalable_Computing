#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Data.h"

using namespace std;

class MyClass {
private:
    string filename = "car.txt";
    Data last_query;

public:

    void write(Data * car) {
        ofstream file(filename, ios::app); 
        
        file << (*car).plate << "," << (*car).model << "," << (*car).name << "," << (*car).year << endl;
        file.close();
    }

    void search(string plate) {
        ifstream file(filename);
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
                searched_car.plate = tokens[0];
                searched_car.model = tokens[1];
                searched_car.name = tokens[2];
                searched_car.year = stoi(tokens[3]);
                break;
            }

            tokens.clear();
        }

        file.close();
        last_query = searched_car;
    }

    string get_name(){
        return last_query.name;
    }
    string get_model(){
        return last_query.model;
    }
    int get_year(){
        return last_query.year;
    }
};