#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <thread>
#include <vector>
#include <string>
#include <chrono>

using namespace std;

string read_file(string file_name) {
    ifstream f(file_name);
    string text;
    if (f) {
        ostringstream ss;
        ss << f.rdbuf();
        text = ss.str();
    }
    return text;
}

void write_file(int times = 100) {
    string text = read_file("shakespeare.txt");
    ofstream fw("shakespeare_extended.txt", ofstream::out);
    if (fw.is_open())
    {
      //store array contents to text file
      for (int i = 0; i < times; i++) {
        fw << text << "\n";
      }
      fw.close();
    }
    else {
        cout << "Problem with opening file";
    }
}

struct dataFunction {
            string text;
            int *numLove;
            int *numHate;
        };

void count_love_hate(dataFunction data) {
    istringstream iss(data.text);
    string word;
    do {
        iss >> word;
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        word.erase(std::remove(word.begin(), word.end(), ','), word.end());
        word.erase(std::remove(word.begin(), word.end(), '.'), word.end());
        word.erase(std::remove(word.begin(), word.end(), '\n'), word.end());

        if (word == "love") (*(data.numLove))++;
        if (word == "hate") (*(data.numHate))++;
    } while (iss);
}

void run(int numThreads = 1, bool log = true) {
    auto start = chrono::steady_clock::now();
    string text = read_file("shakespeare_extended.txt");
    int threadLength = text.length() / numThreads;
    vector<thread *> threads;
    vector<int> loveCount(numThreads, 0);
    vector<int> hateCount(numThreads, 0);

    for (int i = 0; i < numThreads; i++) {
        dataFunction data;
        data.text = text.substr(i * threadLength, threadLength);
        data.numLove = &(loveCount[i]);
        data.numHate = &(hateCount[i]);
        thread *ti = new thread(&count_love_hate, data);
        threads.push_back(ti);
    }

    auto prep = chrono::steady_clock::now();
    for (auto th : threads) {
        th -> join();
    }

    int numLove = accumulate(loveCount.begin(), loveCount.end(), 0);
    int numHate = accumulate(hateCount.begin(), hateCount.end(), 0);

    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;
    chrono::duration<double> prepTime = prep - start;
    chrono::duration<double> searchTime = end - prep;

    // if log, creates a log else create an yaml file with these log info
    if (log) {
        cout << "Used threads: " << numThreads << '\n';
        cout << "Block length: " << threadLength << '\n';
        cout << "Preparation time: " << prepTime.count() << '\n';
        cout << "Search time: " << searchTime.count() << '\n';
        cout << "Total time: " << totalTime.count() << '\n';
        cout << "Word occurrences:\n" << "  Love: " << numLove << '\n' << "  Hate: " << numHate << '\n';
        cout << "Most common word: ";
        if (numLove > numHate) {
            cout << "Love\n\n";
        }
        else {
            cout << "Hate\n\n";
        }
    }
    else {
        cout << "---\n";
        cout << "Results:\n";
        cout << "    - numThreads: " << numThreads << '\n';
        cout << "    - blockLength: " << threadLength << '\n';
        cout << "    - preparationTime: " << prepTime.count() << '\n';
        cout << "    - searchTime: " << searchTime.count() << '\n';
        cout << "    - totalTime: " << totalTime.count() << '\n';
        cout << "    - wordOccurrences:\n" << "        - love: " << numLove << '\n' << "        - hate: " << numHate << '\n';
        cout << "    - mostCommonWord: ";
        if (numLove > numHate) {
            cout << "Love\n\n";
        }
        else {
            cout << "Hate\n\n";
        }
    }
}

int main() {
    write_file(100);
    for (int i = 1; i <= 100; ++i) run(i, false);
    remove("shakespeare_extended.txt");
}