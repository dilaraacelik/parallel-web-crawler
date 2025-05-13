#include "../include/crawler.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
// Windows does not have unistd.h, use Windows.h instead for Sleep function
#include <windows.h>
// Use pthreads-win32 for Windows
#define HAVE_STRUCT_TIMESPEC
#else
// Unix systems
#include <unistd.h>
#endif

Crawler::Crawler(const std::vector<std::string>& urls, const std::string& outputFile, int numThreads, bool extendedInfo)
    : urls(urls), outputFile(outputFile), numThreads(numThreads), completedUrls(0), extendedInfo(extendedInfo) {
    
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
        if (i < static_cast<int>(data->urls->size())) {
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
                if (data->extendedInfo) {
                    success = parser.parseHtmlExtended(html, url, 
                                                     result.title, 
                                                     result.description, 
                                                     result.price, 
                                                     result.date,
                                                     result.keywords,
                                                     result.author,
                                                     result.links,
                                                     result.images,
                                                     result.headings,
                                                     result.logo);
                } else {
                    success = parser.parseHtml(html, url, 
                                              result.title, 
                                              result.description, 
                                              result.price, 
                                              result.date);
                }
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
    if (extendedInfo) {
        std::cout << "Extended information will be collected." << std::endl;
    }
    
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
        threadData[i].extendedInfo = extendedInfo;
        
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
    if (extendedInfo) {
        saveExtendedResults();
    } else {
        saveResults();
    }
}

void Crawler::runSingleThreaded() {
    std::cout << "Starting crawler in single-threaded mode." << std::endl;
    if (extendedInfo) {
        std::cout << "Extended information will be collected." << std::endl;
    }
    
    completedUrls = 0;
    
    // Process all URLs sequentially
    for (size_t i = 0; i < urls.size(); i++) {
        results[i] = processSingleUrl(urls[i]);
        
        // Update progress
        completedUrls++;
        Utils::printProgress(completedUrls, urls.size());
    }
    
    // Save results to file
    if (extendedInfo) {
        saveExtendedResults();
    } else {
        saveResults();
    }
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
        if (extendedInfo) {
            success = parser.parseHtmlExtended(html, url, 
                                             result.title, 
                                             result.description, 
                                             result.price, 
                                             result.date,
                                             result.keywords,
                                             result.author,
                                             result.links,
                                             result.images,
                                             result.headings,
                                             result.logo);
        } else {
            success = parser.parseHtml(html, url, 
                                      result.title, 
                                      result.description, 
                                      result.price, 
                                      result.date);
        }
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
    
    // Write header row with spaces after commas
    file << "URL, Title, Description, Price, Date, Success, Error\n";
    
    // Write results with spaces after commas
    for (const auto& result : results) {
        file << formatUrlForCsv(result.url) << ", "
             << Utils::escapeCsvField(result.title) << ", "
             << Utils::escapeCsvField(result.description) << ", "
             << Utils::escapeCsvField(result.price) << ", "
             << Utils::escapeCsvField(result.date) << ", "
             << (result.success ? "true" : "false") << ", "
             << Utils::escapeCsvField(result.error) << "\n";
    }
    
    file.close();
}

// Function to properly format URLs in CSV export
std::string Crawler::formatUrlForCsv(const std::string& url) {
    // Remove any invalid characters from the beginning
    std::string cleaned = url;
    while (!cleaned.empty() && 
           (cleaned[0] == '@' || cleaned[0] == ' ' || cleaned[0] == '\t' || 
            cleaned[0] == '\r' || cleaned[0] == '\n')) {
        cleaned.erase(0, 1);
    }
    
    // Ensure URLs are properly escaped for CSV
    return Utils::escapeCsvField(cleaned);
}

void Crawler::saveExtendedResults() {
    std::ofstream file(outputFile);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }
    
    // Write header row for basic fields with spaces after commas
    file << "URL, Title, Description, Price, Date, Keywords, Author, Logo, Links Count, Images Count, Headings Count, Success, Error\n";
    
    // Write results with spaces after commas
    for (const auto& result : results) {
        file << formatUrlForCsv(result.url) << ", "
             << Utils::escapeCsvField(result.title) << ", "
             << Utils::escapeCsvField(result.description) << ", "
             << Utils::escapeCsvField(result.price) << ", "
             << Utils::escapeCsvField(result.date) << ", "
             << Utils::escapeCsvField(result.keywords) << ", "
             << Utils::escapeCsvField(result.author) << ", "
             << Utils::escapeCsvField(result.logo) << ", "
             << result.links.size() << ", "
             << result.images.size() << ", "
             << result.headings.size() << ", "
             << (result.success ? "true" : "false") << ", "
             << Utils::escapeCsvField(result.error) << "\n";
    }
    
    // Create separate files for detailed data if there's any
    bool hasLinks = false;
    bool hasImages = false;
    bool hasHeadings = false;
    
    for (const auto& result : results) {
        if (!result.links.empty()) hasLinks = true;
        if (!result.images.empty()) hasImages = true;
        if (!result.headings.empty()) hasHeadings = true;
        
        if (hasLinks && hasImages && hasHeadings) break;
    }
    
    // Save links to a separate file
    if (hasLinks) {
        std::string linksFile = outputFile.substr(0, outputFile.find_last_of('.')) + "_links.csv";
        std::ofstream linkStream(linksFile);
        
        if (linkStream.is_open()) {
            linkStream << "Source URL, Link URL, Link Type\n";
            
            for (size_t i = 0; i < results.size(); i++) {
                const auto& result = results[i];
                std::string sourceUrl = result.url;
                std::string sourceDomain = Parser::getDomainFromUrl(sourceUrl);
                
                for (const auto& link : result.links) {
                    // Determine link type
                    std::string linkType = "External";
                    if (link.find("javascript:") == 0) {
                        linkType = "JavaScript";
                    } else if (link.find("mailto:") == 0) {
                        linkType = "Email";
                    } else if (link.find("tel:") == 0) {
                        linkType = "Telephone";
                    } else if (link.find('#') == 0) {
                        linkType = "Anchor";
                    } else if (link.find('/') == 0) {
                        linkType = "Relative (Root)";
                    } else if (link.find("://") == std::string::npos) {
                        linkType = "Relative";
                    } else {
                        // Check if same domain
                        std::string linkDomain = Parser::getDomainFromUrl(link);
                        if (linkDomain == sourceDomain) {
                            linkType = "Internal";
                        }
                    }
                    
                    linkStream << formatUrlForCsv(sourceUrl) << ", "
                               << formatUrlForCsv(link) << ", "
                               << linkType << "\n";
                }
            }
            
            linkStream.close();
            std::cout << "Links saved to " << linksFile << std::endl;
        }
    }
    
    // Save images to a separate file
    if (hasImages) {
        std::string imagesFile = outputFile.substr(0, outputFile.find_last_of('.')) + "_images.csv";
        std::ofstream imageStream(imagesFile);
        
        if (imageStream.is_open()) {
            imageStream << "Source URL, Image URL, Is Relative\n";
            
            for (size_t i = 0; i < results.size(); i++) {
                const auto& result = results[i];
                for (const auto& image : result.images) {
                    bool isRelative = (image.find("://") == std::string::npos);
                    imageStream << formatUrlForCsv(result.url) << ", "
                                << formatUrlForCsv(image) << ", "
                                << (isRelative ? "Yes" : "No") << "\n";
                }
            }
            
            imageStream.close();
            std::cout << "Images saved to " << imagesFile << std::endl;
        }
    }
    
    // Save headings to a separate file
    if (hasHeadings) {
        std::string headingsFile = outputFile.substr(0, outputFile.find_last_of('.')) + "_headings.csv";
        std::ofstream headingStream(headingsFile);
        
        if (headingStream.is_open()) {
            headingStream << "Source URL, Heading Text\n";
            
            for (size_t i = 0; i < results.size(); i++) {
                const auto& result = results[i];
                for (const auto& heading : result.headings) {
                    headingStream << formatUrlForCsv(result.url) << ", "
                                  << Utils::escapeCsvField(heading) << "\n";
                }
            }
            
            headingStream.close();
            std::cout << "Headings saved to " << headingsFile << std::endl;
        }
    }
    
    file.close();
} 