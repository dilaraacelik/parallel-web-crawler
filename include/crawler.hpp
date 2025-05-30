#pragma once

#include <string>

class Crawler {
public:
    Crawler(const std::string& url, int threadId, const std::string& runFolder);

    // Start crawling process
    void start();

private:
    std::string url;
    int threadId;
    std::string domain;
    std::string folderName;

    bool downloadPage(const std::string& pageUrl, const std::string& savePath);
    void extractAndDownloadAssets(const std::string& htmlContent, const std::string& baseUrl);
    void saveContentToFile(const std::string& content, const std::string& filePath);
    void crawlLinksFromHtml(const std::string& htmlContent, const std::string& baseUrl);
};
