#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

class Utils {
public:
    static std::vector<std::string> readUrlsFromFile(const std::string& filepath);
};

#endif // UTILS_HPP
