#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <curl/curl.h>

namespace Utils {
    // Read URLs from a file, one URL per line
    std::vector<std::string> readUrlsFromFile(const std::string& filename);
    
    // Write a string to a file
    bool writeToFile(const std::string& filename, const std::string& content, bool append = false);
    
    // Escape CSV fields (handle quotes, commas, etc.)
    std::string escapeCsvField(const std::string& field);
    
    // HTTP request callback for libcurl
    size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* s);
    
    // Make an HTTP GET request and return the HTML content
    bool makeHttpRequest(const std::string& url, std::string& response, std::string& error);
    
    // Initialize libcurl (call once at the beginning of the program)
    bool initCurl();
    
    // Clean up libcurl (call once at the end of the program)
    void cleanupCurl();
    
    // Get a timestamp string in the format YYYY-MM-DD HH:MM:SS
    std::string getCurrentTimestamp();
    
    // Print progress information
    void printProgress(int completed, int total);
} 