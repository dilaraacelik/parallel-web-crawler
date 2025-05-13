#include "../include/crawler.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unistd.h>

Crawler::Crawler(const std::vector<std::string>& urls, const std::string& outputFile, int numThreads)
    : urls(urls), outputFile(outputFile), numThreads(numThreads), completedUrls(0) {
    
    // Initialize libcurl
    if (!Utils::initCurl()) {
        std::cerr << "Failed to initialize libcurl." << std::endl;
        exit(1);
    }
    
    // Resize the threads vector
    threads.resize(numThreads);
    threadData.resize(numThreads);
    
    // Initialize results vector with the size of URLs
    results.resize(urls.size());
}

Crawler::~Crawler() {
    // Cleanup libcurl
    Utils::cleanupCurl();
}

void* Crawler::crawlWorker(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    // Process URLs in the assigned range
    for (int i = data->startIndex; i < data->endIndex; i++) {
        if (i < data->urls->size()) {
            std::string url = (*data->urls)[i];
            
            // Process the URL
            CrawlResult result;
            result.url = url;
            
            // Get HTML content
            std::string html;
            std::string error;
            bool success = Utils::makeHttpRequest(url, html, error);
            
            if (success) {
                // Parse the HTML content
                Parser parser;
                success = parser.parseHtml(html, url, result.title, result.description, result.price, result.date);
                result.success = success;
                if (!success) {
                    result.error = "Failed to parse HTML content";
                }
            } else {
                result.success = false;
                result.error = error;
            }
            
            // Add result to the results vector using a mutex to synchronize access
            {
                std::lock_guard<std::mutex> lock(*data->resultsMutex);
                (*data->results)[i] = result;
            }
            
            // Increment the completed count and print progress
            int completed = ++(*data->completedUrls);
            if (data->id == 0) { // Only thread 0 prints progress
                Utils::printProgress(completed, data->totalUrls);
            }
        }
    }
    
    return nullptr;
}

void Crawler::run() {
    std::cout << "Starting crawler with " << numThreads << " threads." << std::endl;
    
    // Calculate URLs per thread (dividing work evenly)
    int urlsPerThread = urls.size() / numThreads;
    int extraUrls = urls.size() % numThreads;
    
    int startIndex = 0;
    
    // Create and start threads
    for (int i = 0; i < numThreads; i++) {
        threadData[i].id = i;
        threadData[i].urls = &urls;
        threadData[i].results = &results;
        threadData[i].resultsMutex = &resultsMutex;
        threadData[i].startIndex = startIndex;
        threadData[i].completedUrls = &completedUrls;
        threadData[i].totalUrls = urls.size();
        
        // Calculate the end index for this thread
        // Add an extra URL to the first 'extraUrls' threads to distribute the remainder
        int urlsForThisThread = urlsPerThread + (i < extraUrls ? 1 : 0);
        threadData[i].endIndex = startIndex + urlsForThisThread;
        
        // Update start index for the next thread
        startIndex = threadData[i].endIndex;
        
        // Create the thread
        pthread_create(&threads[i], nullptr, crawlWorker, &threadData[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    // Save results to file
    saveResults();
}

void Crawler::runSingleThreaded() {
    std::cout << "Starting crawler in single-threaded mode." << std::endl;
    
    completedUrls = 0;
    
    // Process all URLs sequentially
    for (size_t i = 0; i < urls.size(); i++) {
        results[i] = processSingleUrl(urls[i]);
        
        // Update progress
        completedUrls++;
        Utils::printProgress(completedUrls, urls.size());
    }
    
    // Save results to file
    saveResults();
}

CrawlResult Crawler::processSingleUrl(const std::string& url) {
    CrawlResult result;
    result.url = url;
    
    // Get HTML content
    std::string html;
    std::string error;
    bool success = Utils::makeHttpRequest(url, html, error);
    
    if (success) {
        // Parse the HTML content
        success = parser.parseHtml(html, url, result.title, result.description, result.price, result.date);
        result.success = success;
        if (!success) {
            result.error = "Failed to parse HTML content";
        }
    } else {
        result.success = false;
        result.error = error;
    }
    
    return result;
}

void Crawler::saveResults() {
    std::ofstream file(outputFile);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }
    
    // Write header row
    file << "URL,Title,Description,Price,Date,Success,Error\n";
    
    // Write results
    for (const auto& result : results) {
        file << Utils::escapeCsvField(result.url) << ","
             << Utils::escapeCsvField(result.title) << ","
             << Utils::escapeCsvField(result.description) << ","
             << Utils::escapeCsvField(result.price) << ","
             << Utils::escapeCsvField(result.date) << ","
             << (result.success ? "true" : "false") << ","
             << Utils::escapeCsvField(result.error) << "\n";
    }
    
    file.close();
} 