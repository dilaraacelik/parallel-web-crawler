#include "../include/utils.hpp"
#include <fstream>
#include <iostream>

std::vector<std::string> Utils::readUrlsFromFile(const std::string& filepath) {
    std::vector<std::string> urls;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "File not opened: " << filepath << std::endl;
        return urls;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            urls.push_back(line);
    }

    file.close();
    return urls;
}
