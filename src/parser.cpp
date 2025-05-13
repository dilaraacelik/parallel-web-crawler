#include "../include/parser.hpp"
#include <algorithm>
#include <iostream>

Parser::Parser() {
    // Initialize default regex patterns for common data
    titleRegex = std::regex("<title[^>]*>(.*?)</title>", std::regex::icase);
    descriptionRegex = std::regex("<meta\\s+name=\"description\"\\s+content=\"([^\"]+)\"", std::regex::icase);
    priceRegex = std::regex("\\$\\s*([0-9,]+\\.?[0-9]*)|([0-9,]+\\.?[0-9]*)\\s*\\$", std::regex::icase);
    dateRegex = std::regex("(\\d{4}-\\d{2}-\\d{2}|\\d{2}/\\d{2}/\\d{4})", std::regex::icase);
    
    // Initialize additional regex patterns
    keywordsRegex = std::regex("<meta\\s+name=\"keywords\"\\s+content=\"([^\"]+)\"", std::regex::icase);
    authorRegex = std::regex("<meta\\s+name=\"author\"\\s+content=\"([^\"]+)\"", std::regex::icase);
    
    // Improved link detection
    linksRegex = std::regex("<a\\s+[^>]*href=\"([^\"]*)\"[^>]*>", std::regex::icase);
    
    imagesRegex = std::regex("<img\\s+[^>]*src=\"([^\"]+)\"[^>]*>", std::regex::icase);
    headingsRegex = std::regex("<h[1-6][^>]*>(.*?)</h[1-6]>", std::regex::icase);
    logoRegex = std::regex("<link\\s+[^>]*rel=\"icon\"[^>]*href=\"([^\"]+)\"[^>]*>|<link\\s+[^>]*rel=\"shortcut icon\"[^>]*href=\"([^\"]+)\"[^>]*>", std::regex::icase);
}

std::string Parser::extractPattern(const std::string& html, const std::regex& pattern) {
    std::smatch match;
    if (std::regex_search(html, match, pattern) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

std::vector<std::string> Parser::extractAllPatterns(const std::string& html, const std::regex& pattern) {
    std::vector<std::string> results;
    std::string::const_iterator searchStart(html.cbegin());
    std::smatch match;
    
    while (std::regex_search(searchStart, html.cend(), match, pattern)) {
        if (match.size() > 1) {
            results.push_back(match[1].str());
        }
        searchStart = match.suffix().first;
    }
    
    return results;
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

// Normalize a URL based on the base URL of the page
std::string Parser::normalizeUrl(const std::string& url, const std::string& baseUrl) {
    if (url.empty()) {
        return "";
    }
    
    // If the URL is already absolute, return it as is
    if (url.find("://") != std::string::npos) {
        return url;
    }
    
    // Get the base domain and protocol
    std::string domain = getDomainFromUrl(baseUrl);
    std::string protocol;
    size_t protocolPos = baseUrl.find("://");
    if (protocolPos != std::string::npos) {
        protocol = baseUrl.substr(0, protocolPos + 3); // including "://"
    } else {
        protocol = "http://";
    }
    
    // Handle relative URLs
    if (url.find('/') == 0) {
        // URL starts with /, it's a root-relative URL
        return protocol + domain + url;
    } else {
        // Get the base path from the baseUrl
        std::string basePath;
        size_t domainEnd = baseUrl.find('/', protocolPos + 3);
        if (domainEnd != std::string::npos) {
            size_t lastSlash = baseUrl.find_last_of('/');
            if (lastSlash != std::string::npos && lastSlash > domainEnd) {
                basePath = baseUrl.substr(0, lastSlash + 1);
            } else {
                basePath = baseUrl + "/";
            }
        } else {
            basePath = baseUrl + "/";
        }
        
        return basePath + url;
    }
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

bool Parser::parseHtmlExtended(const std::string& html, const std::string& url, 
                  std::string& title, std::string& description, 
                  std::string& price, std::string& date,
                  std::string& keywords, std::string& author,
                  std::vector<std::string>& links,
                  std::vector<std::string>& images,
                  std::vector<std::string>& headings,
                  std::string& logo) {
    
    // First extract basic fields
    bool success = parseHtml(html, url, title, description, price, date);
    
    // Get domain to check for site-specific configurations
    std::string domain = getDomainFromUrl(url);
    
    // Extract keywords
    keywords = extractPattern(html, keywordsRegex);
    
    // Extract author
    author = extractPattern(html, authorRegex);
    
    // Extract all links and normalize them
    std::vector<std::string> rawLinks = extractAllPatterns(html, linksRegex);
    links.clear();
    
    for (const auto& rawLink : rawLinks) {
        // Skip javascript: links, mailto:, tel:, etc.
        if (rawLink.find("javascript:") == 0 || 
            rawLink.find("mailto:") == 0 || 
            rawLink.find("tel:") == 0 ||
            rawLink.find("#") == 0) {
            continue;
        }
        
        // Normalize the URL
        std::string normalizedLink = normalizeUrl(rawLink, url);
        if (!normalizedLink.empty()) {
            links.push_back(normalizedLink);
        }
    }
    
    // Extract all images
    std::vector<std::string> rawImages = extractAllPatterns(html, imagesRegex);
    images.clear();
    
    for (const auto& rawImage : rawImages) {
        // Normalize the image URL
        std::string normalizedImage = normalizeUrl(rawImage, url);
        if (!normalizedImage.empty()) {
            images.push_back(normalizedImage);
        }
    }
    
    // Extract all headings
    headings = extractAllPatterns(html, headingsRegex);
    
    // Extract logo
    logo = extractPattern(html, logoRegex);
    if (!logo.empty()) {
        logo = normalizeUrl(logo, url);
    }
    
    // Clean up HTML tags from headings
    std::regex htmlTagPattern("<[^>]*>");
    for (auto& heading : headings) {
        heading = std::regex_replace(heading, htmlTagPattern, "");
        
        // Trim whitespace
        auto trim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        };
        
        trim(heading);
    }
    
    // Remove duplicate links
    std::sort(links.begin(), links.end());
    links.erase(std::unique(links.begin(), links.end()), links.end());
    
    // Remove duplicate images
    std::sort(images.begin(), images.end());
    images.erase(std::unique(images.begin(), images.end()), images.end());
    
    // Consider successful if at least title, description, or any other field was extracted
    return success || !keywords.empty() || !author.empty() || 
           !links.empty() || !images.empty() || !headings.empty() || !logo.empty();
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
    } else if (field == "keywords") {
        keywordsRegex = customRegex;
    } else if (field == "author") {
        authorRegex = customRegex;
    } else if (field == "links") {
        linksRegex = customRegex;
    } else if (field == "images") {
        imagesRegex = customRegex;
    } else if (field == "headings") {
        headingsRegex = customRegex;
    } else if (field == "logo") {
        logoRegex = customRegex;
    }
} 