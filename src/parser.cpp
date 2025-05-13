#include "../include/parser.hpp"
#include <algorithm>
#include <iostream>

Parser::Parser() {
    // Initialize default regex patterns for common data
    titleRegex = std::regex("<title[^>]*>(.*?)</title>", std::regex::icase);
    descriptionRegex = std::regex("<meta\\s+name=\"description\"\\s+content=\"([^\"]+)\"", std::regex::icase);
    priceRegex = std::regex("\\$\\s*([0-9,]+\\.?[0-9]*)|([0-9,]+\\.?[0-9]*)\\s*\\$", std::regex::icase);
    dateRegex = std::regex("(\\d{4}-\\d{2}-\\d{2}|\\d{2}/\\d{2}/\\d{4})", std::regex::icase);
}

std::string Parser::extractPattern(const std::string& html, const std::regex& pattern) {
    std::smatch match;
    if (std::regex_search(html, match, pattern) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::string Parser::getDomainFromUrl(const std::string& url) {
    // Extract domain from URL (basic implementation)
    std::string domain;
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        size_t start = pos + 3;
        size_t end = url.find('/', start);
        if (end != std::string::npos) {
            domain = url.substr(start, end - start);
        } else {
            domain = url.substr(start);
        }
    }
    return domain;
}

bool Parser::parseHtml(const std::string& html, const std::string& url, 
                      std::string& title, std::string& description, 
                      std::string& price, std::string& date) {
    // Get domain to check for site-specific configurations
    std::string domain = getDomainFromUrl(url);
    
    // Extract title
    title = extractPattern(html, titleRegex);
    
    // Extract description
    description = extractPattern(html, descriptionRegex);
    
    // Extract price
    price = extractPattern(html, priceRegex);
    
    // Extract date
    date = extractPattern(html, dateRegex);
    
    // Clean up the extracted data
    // Remove HTML tags and entities from title
    std::regex htmlTagPattern("<[^>]*>");
    title = std::regex_replace(title, htmlTagPattern, "");
    
    // Remove HTML tags and entities from description
    description = std::regex_replace(description, htmlTagPattern, "");
    
    // Trim whitespace
    auto trim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    };
    
    trim(title);
    trim(description);
    trim(price);
    trim(date);
    
    // Consider successful if at least title or description was extracted
    return !title.empty() || !description.empty();
}

void Parser::setCustomPattern(const std::string& domain, const std::string& field, const std::string& pattern) {
    // Store custom regex pattern for specific domain and field
    siteConfigs[domain + ":" + field] = pattern;
    
    // Update the corresponding regex if needed
    std::regex customRegex(pattern);
    if (field == "title") {
        titleRegex = customRegex;
    } else if (field == "description") {
        descriptionRegex = customRegex;
    } else if (field == "price") {
        priceRegex = customRegex;
    } else if (field == "date") {
        dateRegex = customRegex;
    }
} 