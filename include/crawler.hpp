#pragma once

#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <pthread.h>
#include <atomic>
#include "parser.hpp"

// Structure to hold URL data and parsing results
struct CrawlResult {
    std::string url;
    std::string title;
    std::string description;
    std::string price;
    std::string date;
    bool success;
    std::string error;
};

// Structure to be passed to worker threads
struct ThreadData {
    int id;
    std::vector<std::string>* urls;
    std::vector<CrawlResult>* results;
    std::mutex* resultsMutex;
    int startIndex;
    int endIndex;
    std::atomic<int>* completedUrls;
    int totalUrls;
};

class Crawler {
private:
    std::vector<std::string> urls;
    std::string outputFile;
    int numThreads;
    std::vector<pthread_t> threads;
    std::vector<ThreadData> threadData;
    std::vector<CrawlResult> results;
    std::mutex resultsMutex;
    std::atomic<int> completedUrls;
    Parser parser;

    // Thread worker function
    static void* crawlWorker(void* arg);
    
    // Process a single URL and return the result
    CrawlResult processSingleUrl(const std::string& url);
    
    // Save results to output file
    void saveResults();

public:
    Crawler(const std::vector<std::string>& urls, const std::string& outputFile, int numThreads);
    ~Crawler();
    
    // Run the crawler with multiple threads
    void run();
    
    // For single-threaded testing
    void runSingleThreaded();
}; 