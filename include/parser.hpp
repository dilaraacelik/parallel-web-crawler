#pragma once

#include <string>
#include <map>
#include <regex>
#include <vector>

// Parser class responsible for extracting data from HTML content
class Parser {
private:
    // Regular expressions for different data fields
    std::regex titleRegex;
    std::regex descriptionRegex;
    std::regex priceRegex;
    std::regex dateRegex;
    std::regex keywordsRegex;
    std::regex authorRegex;
    std::regex linksRegex;
    std::regex imagesRegex;
    std::regex headingsRegex;
    std::regex logoRegex;
    
    // Site-specific parsing configurations
    std::map<std::string, std::string> siteConfigs;
    
    // Helper methods
    std::string extractPattern(const std::string& html, const std::regex& pattern);
    std::vector<std::string> extractAllPatterns(const std::string& html, const std::regex& pattern);
    std::string normalizeUrl(const std::string& url, const std::string& baseUrl);

public:
    Parser();
    
    // Static helper methods
    static std::string getDomainFromUrl(const std::string& url);
    
    // Parse HTML content and extract data
    bool parseHtml(const std::string& html, const std::string& url, 
                  std::string& title, std::string& description, 
                  std::string& price, std::string& date);
    
    // Extended parsing with more data fields
    bool parseHtmlExtended(const std::string& html, const std::string& url, 
                  std::string& title, std::string& description, 
                  std::string& price, std::string& date,
                  std::string& keywords, std::string& author,
                  std::vector<std::string>& links,
                  std::vector<std::string>& images,
                  std::vector<std::string>& headings,
                  std::string& logo);
    
    // Set custom regex patterns for specific sites
    void setCustomPattern(const std::string& domain, const std::string& field, const std::string& pattern);
}; 