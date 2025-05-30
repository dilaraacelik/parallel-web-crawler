#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include "../include/thread_manager.hpp"
#include "../include/utils.hpp"

namespace fs = std::filesystem;

void printMenu() {
    std::cout << "=== Paralel Web Crawler ===\n";
    std::cout << "1. Normal Mod (Crawl all the urls)\n";
    std::cout << "2. Time Compare Mod (Compare 1,2,4,8,16 threads)\n";
    std::cout << "Your choice (1 or 2): ";
}



int main() {
    printMenu();
    int choice;
    std::cin >> choice;

    // URL dosyasını oku
    std::vector<std::string> urls = Utils::readUrlsFromFile("data/url.txt");
    if (urls.empty()) {
        std::cerr << "URL list is empty or not read!" << std::endl;
        return 1;
    }

    // Clean output folder
    if (fs::exists("output")) {
        fs::remove_all("output");
    }
    fs::create_directories("output");


    ThreadManager manager;

    if (choice == 1) {
        std::cout << "How many threads do you want to use? (example: 4): ";
        int threadCount;
        std::cin >> threadCount;

        manager.runWithThreads(urls, threadCount);
    } 
    else if (choice == 2) {
        std::vector<int> threadCounts = {1, 2, 4, 8, 16};
        manager.compareExecutionTimes(urls, threadCounts, "timeelapsed.txt");
    } 
    else {
        std::cerr << "Invalid choice." << std::endl;
    }

    return 0;
}

int WinMain() { return main(); }
