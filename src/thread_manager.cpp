#include "../include/thread_manager.hpp"
#include "../include/crawler.hpp"
#include <pthread.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <sstream>
namespace fs = std::filesystem;

struct ThreadArgs {
    std::vector<std::string> urls;
    int threadId;
    std::string logFile;
    std::string runFolder;
};

// Create unique folder name with timestamp
std::string createTimestampFolder() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << now_ms.count();
    return ss.str();
}

void* threadWorker(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    std::ofstream log(args->logFile);
    
    for (const auto& url : args->urls) {
        log << "Processing URL: " << url << std::endl;
        Crawler crawler(url, args->threadId, args->runFolder);
        crawler.start();
    }
    
    log.close();
    return nullptr;
}

void ThreadManager::runWithThreads(const std::vector<std::string>& urls, int threadCount) {
    // Constrainst threadCount to urls.size()
    threadCount = std::min(threadCount, static_cast<int>(urls.size()));
    if (threadCount <= 0) return;

    // Create unique folder with timestamp
    std::string timestamp = createTimestampFolder();
    std::string runFolder = "output/run_" + timestamp + "_threads_" + std::to_string(threadCount);
    fs::create_directories(runFolder);

    // Create subfolder for each thread
    for (int i = 0; i < threadCount; ++i) {
        fs::create_directories(runFolder + "/thread_" + std::to_string(i));
    }

    std::vector<pthread_t> threads(threadCount);
    std::vector<ThreadArgs> threadArgs(threadCount);

    // Split urls into equal parts
    int totalUrls = urls.size();
    int baseUrlsPerThread = totalUrls / threadCount;
    int extraUrls = totalUrls % threadCount;

    int currentUrlIndex = 0;
    for (int i = 0; i < threadCount; ++i) {
        // Calculate number of urls for each thread
        int urlsForThisThread = baseUrlsPerThread + (i < extraUrls ? 1 : 0);
        
        // Set thread arguments
        threadArgs[i].urls.clear();
        threadArgs[i].urls.reserve(urlsForThisThread);
        threadArgs[i].threadId = i;
        threadArgs[i].logFile = runFolder + "/thread_" + std::to_string(i) + "/log.txt";
        threadArgs[i].runFolder = runFolder;

        // Assign urls to this thread
        for (int j = 0; j < urlsForThisThread; ++j) {
            threadArgs[i].urls.push_back(urls[currentUrlIndex++]);
        }
    }

    // Start threads
    for (int i = 0; i < threadCount; ++i) {
        pthread_create(&threads[i], nullptr, threadWorker, &threadArgs[i]);
    }
    for (int i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], nullptr);
    }
}

void ThreadManager::compareExecutionTimes(const std::vector<std::string>& urls,
                                          const std::vector<int>& threadCounts,
                                          const std::string& outputFile) {

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "timeelapsed.txt file not opened." << std::endl;
        return;
    }

    out << "=== Parallel Web Crawler Performance Test ===\n\n";
    out << "Number of Tested URLs: " << urls.size() << "\n\n";
    out << "Thread Count | Elapsed Time (seconds) | Speedup Ratio\n";
    out << "------------------------------------------------\n";

    double singleThreadTime = 0.0;
    for (int count : threadCounts) {
        std::cout << "\n" << count << " thread test starting...\n";
        auto start = std::chrono::high_resolution_clock::now();

        runWithThreads(urls, count);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double elapsedSeconds = elapsed.count();

        if (count == 1) {
            singleThreadTime = elapsedSeconds;
        }

        double speedup = singleThreadTime / elapsedSeconds;
        
        out << std::setw(12) << count << " | " 
            << std::fixed << std::setprecision(2) << elapsedSeconds << " | "
            << std::fixed << std::setprecision(2) << speedup << "x\n";
            
        std::cout << count << " thread test completed. Time: " 
                  << std::fixed << std::setprecision(2) << elapsedSeconds 
                  << " seconds (Speedup: " << std::fixed << std::setprecision(2) 
                  << speedup << "x)\n";
    }

    out.close();
    std::cout << "\nResults saved to timeelapsed.txt file.\n";
}
