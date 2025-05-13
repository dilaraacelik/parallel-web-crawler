#pragma once

#include <string>
#include <map>
#include <regex>

// Parser class responsible for extracting data from HTML content
class Parser {
private:
    // Regular expressions for different data fields
    std::regex titleRegex;
    std::regex descriptionRegex;
    std::regex priceRegex;
    std::regex dateRegex;
    
    // Site-specific parsing configurations
    std::map<std::string, std::string> siteConfigs;
    
    // Helper methods
    std::string extractPattern(const std::string& html, const std::regex& pattern);
    std::string getDomainFromUrl(const std::string& url);

public:
    Parser();
    
    // Parse HTML content and extract data
    bool parseHtml(const std::string& html, const std::string& url, 
                  std::string& title, std::string& description, 
                  std::string& price, std::string& date);
    
    // Set custom regex patterns for specific sites
    void setCustomPattern(const std::string& domain, const std::string& field, const std::string& pattern);
}; 