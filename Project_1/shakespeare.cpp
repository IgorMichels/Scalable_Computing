#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
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

pair<int, int> count_love_hate(string text, int numLove, int numHate) {
    istringstream iss(text);
    string word;
    do {
        iss >> word;
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        word.erase(std::remove(word.begin(), word.end(), ','), word.end());
        word.erase(std::remove(word.begin(), word.end(), '.'), word.end());
        word.erase(std::remove(word.begin(), word.end(), '\n'), word.end());

        if (word == "love") numLove++;
        if (word == "hate") numHate++;
    } while (iss);
    return make_pair(numLove, numHate);
}

void run(int numThreads = 1) {
    auto start = chrono::steady_clock::now();
    string text = read_file("shakespeare.txt");
    int numLove = 0;
    int numHate = 0;
    int threadLength = text.length() / numThreads;

    auto prep = chrono::steady_clock::now();
    pair<int, int> answer = count_love_hate(text, numLove, numHate);
    
    auto end = chrono::steady_clock::now();
    chrono::duration<double> totalTime = end - start;
    chrono::duration<double> prepTime = prep - start;
    chrono::duration<double> searchTime = end - prep;

    // Log
    cout << "Used threads: " << numThreads << '\n';
    cout << "Block length: " << threadLength << '\n';
    cout << "Preparation time: " << prepTime.count() << '\n';
    cout << "Search time: " << searchTime.count() << '\n';
    cout << "Total time: " << totalTime.count() << '\n';
    cout << "Word occurrences:\n" << "  Love: " << answer.first << '\n' << "  Hate: " << answer.second << '\n';
    cout << "Most common word: ";
    if (answer.first > answer.second) {
        cout << "Love\n\n";
    }
    else {
        cout << "Hate\n\n";
    }
}

int main() {
    for (int i = 1; i <= 100; ++i) run(i);
}