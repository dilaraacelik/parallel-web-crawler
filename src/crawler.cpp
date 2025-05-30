#include "../include/crawler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <curl/curl.h>
#include <filesystem>
#include <set>
#include <cctype>
#include <random>
#include <array>
namespace fs = std::filesystem;

// Helper to fill string from CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Sanitize filename
std::string sanitizeFilename(const std::string& filename) {
    std::string result;
    result.reserve(filename.length());
    
    for (char c : filename) {
        if (std::isalnum(c) || c == '.' || c == '-' || c == '_') {
            result += c;
        } else {
            result += '_';
        }
    }
    
    return result;
}

// Realistic User-Agent's
const std::array<std::string, 5> USER_AGENTS = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36 Edg/121.0.0.0",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:123.0) Gecko/20100101 Firefox/123.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.3.1 Safari/605.1.15",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36"
};

// Select random User-Agent
std::string getRandomUserAgent() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, USER_AGENTS.size() - 1);
    return USER_AGENTS[dis(gen)];
}

Crawler::Crawler(const std::string& url, int threadId, const std::string& runFolder)
    : url(url), threadId(threadId) {
    domain = std::regex_replace(url, std::regex(R"(https?://)"), "");
    std::replace(domain.begin(), domain.end(), '/', '_');
    
    // Use directly given run folder
    folderName = runFolder + "/thread_" + std::to_string(threadId) + "/" + domain;
    fs::create_directories(folderName);
}

void Crawler::start() {
    std::cout << "[Thread " << threadId << "] Downloading main page: " << url << std::endl;

    std::string htmlContent;
    if (!downloadPage(url, folderName + "/index.html")) {
        std::cerr << "[Thread " << threadId << "] ERROR: Main page not downloaded: " << url << std::endl;
        return;
    }

    std::ifstream in(folderName + "/index.html");
    if (!in.is_open()) {
        std::cerr << "[Thread " << threadId << "] ERROR: Downloaded file not opened: " << url << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    htmlContent = buffer.str();

    std::cout << "[Thread " << threadId << "] Downloading assets: " << url << std::endl;
    extractAndDownloadAssets(htmlContent, url);
    
    std::cout << "[Thread " << threadId << "] Scanning links: " << url << std::endl;
    crawlLinksFromHtml(htmlContent, url);
}

bool Crawler::downloadPage(const std::string& pageUrl, const std::string& savePath) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[Thread " << threadId << "] ERROR: CURL not initialized" << std::endl;
        return false;
    }

    std::string response;
    struct curl_slist* headers = nullptr;

    // Add realistic headers
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
    headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br");
    headers = curl_slist_append(headers, "DNT: 1");
    headers = curl_slist_append(headers, "Connection: keep-alive");
    headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
    headers = curl_slist_append(headers, "Sec-Fetch-Dest: document");
    headers = curl_slist_append(headers, "Sec-Fetch-Mode: navigate");
    headers = curl_slist_append(headers, "Sec-Fetch-Site: none");
    headers = curl_slist_append(headers, "Sec-Fetch-User: ?1");
    headers = curl_slist_append(headers, "Cache-Control: max-age=0");

    curl_easy_setopt(curl, CURLOPT_URL, pageUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10 seconds timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L); // 5 seconds connection timeout
    curl_easy_setopt(curl, CURLOPT_USERAGENT, getRandomUserAgent().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_ENCODING, ""); // Automatic gzip support
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4); // Only IPv4
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L); // Keep-alive connections
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L); // 120 seconds idle
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L); // 60 seconds interval

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[Thread " << threadId << "] ERROR: " << curl_easy_strerror(res) << " - " << pageUrl << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "[Thread " << threadId << "] ERROR: HTTP " << http_code << " - " << pageUrl << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    saveContentToFile(response, savePath);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return true;
}

void Crawler::extractAndDownloadAssets(const std::string& html, const std::string& baseUrl) {
    std::smatch match;
    // More specific regex patterns
    std::vector<std::regex> patterns = {
        std::regex(R"(<link[^>]+href=["']([^"']+\.(css|js))["'])"),
        std::regex(R"(<script[^>]+src=["']([^"']+\.js)["'])"),
        std::regex(R"(<img[^>]+src=["']([^"']+\.(png|jpg|jpeg|gif))["'])")
    };

    std::set<std::string> downloadedUrls; // Prevent duplicate URLs
    int assetCount = 0;
    const int MAX_ASSETS = 10; // Maximum 10 assets per site

    for (const auto& pattern : patterns) {
        if (assetCount >= MAX_ASSETS) break;

        std::string::const_iterator searchStart(html.cbegin());
        while (std::regex_search(searchStart, html.cend(), match, pattern) && assetCount < MAX_ASSETS) {
            std::string assetUrl = match[1].str();
            
            // Fix URL
            if (assetUrl.find("//") == 0) {
                assetUrl = "https:" + assetUrl;
            } else if (assetUrl.find("http") != 0) {
                if (assetUrl[0] == '/') {
                    assetUrl = baseUrl + assetUrl;
                } else {
                    assetUrl = baseUrl + "/" + assetUrl;
                }
            }

            // Clean unnecessary characters from URL
            size_t pos = assetUrl.find_first_of("\"\'");
            if (pos != std::string::npos) {
                assetUrl = assetUrl.substr(0, pos);
            }

            // If this URL hasn't been downloaded yet
            if (downloadedUrls.find(assetUrl) == downloadedUrls.end()) {
                std::string filename = assetUrl.substr(assetUrl.find_last_of("/") + 1);
                filename = sanitizeFilename(filename);
                if (!filename.empty()) {
                    std::string savePath = folderName + "/" + filename;
                    if (downloadPage(assetUrl, savePath)) {
                        downloadedUrls.insert(assetUrl);
                        assetCount++;
                    }
                }
            }

            searchStart = match.suffix().first;
        }
    }
}

void Crawler::crawlLinksFromHtml(const std::string& html, const std::string& baseUrl) {
    std::smatch match;
    std::regex pattern(R"(<a\s+(?:[^>]*?\s+)?href=["']([^"']+)["'])");

    std::string::const_iterator searchStart(html.cbegin());
    int linkCount = 0;
    const int MAX_LINKS = 2; // Maximum 2 links

    while (std::regex_search(searchStart, html.cend(), match, pattern) && linkCount < MAX_LINKS) {
        std::string link = match[1].str();
        
        // Fix URL
        if (link.find("//") == 0) {
            link = "https:" + link;
        } else if (link.find("http") != 0) {
            if (link[0] == '/') {
                link = baseUrl + link;
            } else {
                link = baseUrl + "/" + link;
            }
        }

        // Clean unnecessary characters from URL
        size_t pos = link.find_first_of("\"\'");
        if (pos != std::string::npos) {
            link = link.substr(0, pos);
        }

        std::string filename = "linked_" + std::to_string(linkCount) + ".html";
        downloadPage(link, folderName + "/" + filename);
        linkCount++;
        searchStart = match.suffix().first;
    }
}

void Crawler::saveContentToFile(const std::string& content, const std::string& filePath) {
    try {
        std::ofstream out(filePath, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "[Thread " << threadId << "] ERROR: File not written: " << filePath << std::endl;
            return;
        }
        out << content;
        out.close();
    } catch (const std::exception& e) {
        std::cerr << "[Thread " << threadId << "] ERROR: File write error: " << e.what() << std::endl;
    }
}
