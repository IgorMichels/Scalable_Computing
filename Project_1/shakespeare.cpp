#include <iostream>
#include <fstream>
#include <sstream>
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

void run(int numThreads = 1, bool log = true) {
    auto start = chrono::steady_clock::now();
    string text = read_file("shakespeare_extended.txt");
    int numLove = 0;
    int numHate = 0;
    int threadLength = text.length() / numThreads;

    auto prep = chrono::steady_clock::now();
    pair<int, int> answer = count_love_hate(text, numLove, numHate);
    
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
        cout << "Word occurrences:\n" << "  Love: " << answer.first << '\n' << "  Hate: " << answer.second << '\n';
        cout << "Most common word: ";
        if (answer.first > answer.second) {
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
        cout << "    - wordOccurrences:\n" << "        - love: " << answer.first << '\n' << "        - hate: " << answer.second << '\n';
        cout << "    - mostCommonWord: ";
        if (answer.first > answer.second) {
            cout << "Love\n\n";
        }
        else {
            cout << "Hate\n\n";
        }
    }
}

int main() {
    write_file(5);
    for (int i = 1; i <= 100; ++i) run(i, false);
    remove("shakespeare_extended.txt");
}