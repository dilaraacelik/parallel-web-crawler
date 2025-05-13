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
    std::string keywords;
    std::string author;
    std::string logo;
    std::vector<std::string> links;
    std::vector<std::string> images;
    std::vector<std::string> headings;
    bool success;
    std::string error;
    
    // Constructor with default values
    CrawlResult() : success(false) {}
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
    bool extendedInfo;  // Whether to collect extended information
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
    bool extendedInfo;  // Whether to collect extended information

    // Thread worker function
    static void* crawlWorker(void* arg);
    
    // Process a single URL and return the result
    CrawlResult processSingleUrl(const std::string& url);
    
    // Save results to output file
    void saveResults();
    
    // Save extended results to output file
    void saveExtendedResults();
    
    // Helper function to format URLs for CSV output
    std::string formatUrlForCsv(const std::string& url);

public:
    Crawler(const std::vector<std::string>& urls, const std::string& outputFile, int numThreads, bool extendedInfo = false);
    ~Crawler();
    
    // Run the crawler with multiple threads
    void run();
    
    // For single-threaded testing
    void runSingleThreaded();
}; 