#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <bits/stdc++.h>
#include <chrono>
#include <cmath>
#include <thread>

auto start0 = std::chrono::steady_clock::now();
std::vector<int> primos {};


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

bool is_prime(int p){

    if(p < 2){return false;}
    else if(p == 2 || p == 3){
        primos.push_back(p);
        return true;}

    double rp = std::pow(p, 0.5);
    for(int i = 2; i <= rp; i++){
        if(p % i == 0){return false;}
    }
    primos.push_back(p);

    return true;
}

void write(std::vector<int> * primes, std::string file_name = "primos.txt"){
    std::ofstream myfile;
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
    std::vector<std::thread *>  threads;

    int k = 1;
    for(int i = 0; i < n_threads - 1; i++){
        std::thread * ti = new std::thread(&iterate, k, k + block);
        threads.push_back(ti);
        k += block;
    }
    std::thread *ti = new std::thread(&iterate, k, N);
    threads.push_back(ti);

    for (std::thread * th : threads) {
        th -> join();
    }

    return;
}

void printP(int c, std::vector<int> * vec, int k = 20){

    for(int i = 0; i < k; i++){
        if(c >= (*vec).size()){
            std::cout << "\nThe list is over! \n";
            return;
        }
        else if(c % 10 == 0){
            std::cout << "\n[" << c << "-" << c+10 << "]: "  << (*vec)[c];
        }
        else{
            std::cout << " | " << (*vec)[c];
        }
        c++;
    }
    int _k;
    std::cout << "\n \nHow many more primes to display? (type 0 to exit): ";
    std::cin >> _k;
    if(_k == 0){
        std::cout << "exit! \n \n";
        return;
    }
    else{
        std::cout << "\nDisplaying next " << _k << " primes: \n";
        printP(c, &(*vec), _k);
    }

    return;
}

void print(std::chrono::duration<double> prepTime, std::chrono::duration<double> totalTime,
           int N, std::vector<int> * vec, int numThreads = 1, bool PRINT = true){

    int Np = (*vec).size();
    std::cout << "---\n";
    std::cout << "Results:\n";
    std::cout << "    - numThreads: " << numThreads << '\n';
    std::cout << "    - search time: " << totalTime.count() << "s" << '\n';
    std::cout << "    - amount evaluated: " << N << '\n';
    std::cout << "    - number of primes: " << Np << '\n';

    if(PRINT){
        std::cout << "\nDisplaying first 20 primes: \n";
        printP(0, &(*vec), 20);
    }
    return;
}


int main () {

    int n_ = std::pow(10, 4);

    auto end0 = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff0 = end0 - start0;
    std::cout << "---\n";
    std::cout << "preparation time: " << diff0.count() << '\n';

    std::vector<int> vec_n_threads = {1,2,4,8,16,30,50,100};
    for(int n_threads : vec_n_threads){

        primos = {};
        auto start = std::chrono::steady_clock::now();
        n_balanced(n_threads, n_);
        std::sort(primos.begin(), primos.end());
        write(&primos);

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;

        if(n_threads == 100){
            print(diff0, diff, n_, &primos, n_threads, true);
        }
        else{
            print(diff0, diff, n_, &primos, n_threads, false);
        }
    }

    return 0;
}