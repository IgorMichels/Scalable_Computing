#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
// #include <bits/stdc++.h>
#include <chrono>
#include <cmath>
#include <thread>

using namespace std;

auto start0 = chrono::steady_clock::now();
vector<int> primos {};


//// function below doesnt work with threads
// bool is_prime(int p){

//     if(p < 3){
//         return false;
//     }
//     for(int i = 0; i < primos.size(); i++){
//         if(p % primos[i] == 0){
//             return false;
//         }
//     }
//     primos.push_back(p);
//     return true;
// }


bool is_prime2(int p, int MAX_TEST = 100) {
    if (p < 2) return false;
    
    if (p < 4) {
        primos.push_back(p);    
        return true;
    }

    // filtering even numbers and multiples of 3
    if ((p % 6 != 1) && (p % 6 != 5)) return false;
    double rp = pow(p, 0.5);
    for (int factor = 5; factor <= rp; factor += 2) {
        if (p % factor == 0) return false;
    }

    primos.push_back(p);
    return true;
}

bool is_prime(int p){

    if(p < 2){return false;}
    else if(p == 2 || p == 3){
        primos.push_back(p);
        return true;}

    double rp = pow(p, 0.5);
    for(int i = 2; i <= rp; i++){
        if(p % i == 0){return false;}
    }
    primos.push_back(p);

    return true;
}


void write(vector<int> * primes, string file_name = "primos.txt"){
    ofstream myfile;
    myfile.open (file_name);
    for(int i = 0; i < (*primes).size(); i++){
        myfile << (*primes)[i];
        myfile << ' ';
    }
    myfile.close();
}

void iterate(int begin, int end){
    
    for(int i = begin; i < end; i++){
        is_prime(i);
    }
    return;
}

void n_balanced(int n_threads, int N){

    int block = (n_threads - 1) / N;
    vector<thread *>  threads;

    int k = 1;
    for(int i = 0; i < n_threads - 1; i++){
        thread * ti = new thread(&iterate, k, k + block);
        threads.push_back(ti);
        k += block;
    }
    thread *ti = new thread(&iterate, k, N);
    threads.push_back(ti);

    for (thread * th : threads) {
        th -> join();
    }

    return;
}

void printP(int c, vector<int> * vec, int k = 20){

    for(int i = 0; i < k; i++){
        if(c >= (*vec).size()){
            cout << "\nThe list is over! \n";
            return;
        }
        else if(c % 10 == 0){
            cout << "\n[" << c << "-" << c+10 << "]: "  << (*vec)[c];
        }
        else{
            cout << " | " << (*vec)[c];
        }
        c++;
    }
    int _k;
    cout << "\n \nHow many more primes to display? (type 0 to exit): ";
    cin >> _k;
    if(_k == 0){
        cout << "exit! \n \n";
        return;
    }
    else{
        cout << "\nDisplaying next " << _k << " primes: \n";
        printP(c, &(*vec), _k);
    }

    return;
}

void print(chrono::duration<double> prepTime, chrono::duration<double> totalTime,
           int N, vector<int> * vec, int numThreads = 1, bool PRINT = true){

    int Np = (*vec).size();
    cout << "---\n";
    cout << "Results:\n";
    cout << "    - numThreads: " << numThreads << '\n';
    cout << "    - search time: " << totalTime.count() << "s" << '\n';
    cout << "    - amount evaluated: " << N << '\n';
    cout << "    - number of primes: " << Np << '\n';

    if(PRINT){
        cout << "\nDisplaying first 20 primes: \n";
        printP(0, &(*vec), 20);
    }
    return;
}


int main () {

    int n_ = pow(10, 6);

    auto end0 = chrono::steady_clock::now();
    chrono::duration<double> diff0 = end0 - start0;
    cout << "---\n";
    cout << "preparation time: " << diff0.count() << '\n';

    vector<int> vec_n_threads = {1,2,4,8,16,30,50,100};
    int MAX_THREADS = 10;
    // for(int n_threads : vec_n_threads){
    for(int n_threads = 1; n_threads <= MAX_THREADS; n_threads++){

        primos = {};
        auto start = chrono::steady_clock::now();
        n_balanced(n_threads, n_);
        sort(primos.begin(), primos.end());
        write(&primos);

        auto end = chrono::steady_clock::now();
        chrono::duration<double> diff = end - start;

        if(n_threads == 100){
            print(diff0, diff, n_, &primos, n_threads, true);
        }
        else{
            print(diff0, diff, n_, &primos, n_threads, false);
        }
    }

    return 0;
}