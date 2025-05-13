#include "../include/utils.hpp"
#include <chrono>
#include <iomanip>
#include <ctime>

namespace Utils {

std::vector<std::string> readUrlsFromFile(const std::string& filename) {
    std::vector<std::string> urls;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open input file: " << filename << std::endl;
        return urls;
    }
    
    std::string url;
    while (std::getline(file, url)) {
        // Trim whitespace
        url.erase(url.begin(), std::find_if(url.begin(), url.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        url.erase(std::find_if(url.rbegin(), url.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), url.end());
        
        // Skip empty lines and comments
        if (!url.empty() && url[0] != '#') {
            urls.push_back(url);
        }
    }
    
    file.close();
    return urls;
}

bool writeToFile(const std::string& filename, const std::string& content, bool append) {
    std::ofstream file;
    if (append) {
        file.open(filename, std::ios::app);
    } else {
        file.open(filename);
    }
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

std::string escapeCsvField(const std::string& field) {
    // Check if field contains comma, newline, or double quote
    if (field.find(',') != std::string::npos || 
        field.find('\n') != std::string::npos || 
        field.find('"') != std::string::npos) {
        
        // Replace double quotes with double-double quotes
        std::string escaped = field;
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\"\"");
            pos += 2;
        }
        
        // Enclose in double quotes
        return "\"" + escaped + "\"";
    }
    
    return field;
}

// libcurl callback function to write received data
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        // Handle memory problem
        return 0;
    }
}

// Global CURL initialization flag
static bool curlInitialized = false;

bool initCurl() {
    if (!curlInitialized) {
        CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (code != CURLE_OK) {
            std::cerr << "Failed to initialize libcurl: " << curl_easy_strerror(code) << std::endl;
            return false;
        }
        curlInitialized = true;
    }
    return true;
}

void cleanupCurl() {
    if (curlInitialized) {
        curl_global_cleanup();
        curlInitialized = false;
    }
}

bool makeHttpRequest(const std::string& url, std::string& response, std::string& error) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        error = "Failed to initialize CURL session";
        return false;
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ParallelCrawler/1.0");
    
    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Set timeout (10 seconds)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    // Set write function and data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);
    
    if (!success) {
        error = curl_easy_strerror(res);
    } else {
        // Check HTTP response code
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            success = false;
            error = "HTTP error: " + std::to_string(http_code);
        }
    }
    
    // Clean up
    curl_easy_cleanup(curl);
    
    return success;
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void printProgress(int completed, int total) {
    const int barWidth = 50;
    float progress = static_cast<float>(completed) / total;
    int pos = barWidth * progress;
    
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% (" 
              << completed << "/" << total << ")\r";
    std::cout.flush();
    
    if (completed == total) {
        std::cout << std::endl;
    }
}

} // namespace Utils 