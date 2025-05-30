#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <vector>
#include <string>

class ThreadManager {
public:
    // Normal mod: start with a certain amount of threads   
    void runWithThreads(const std::vector<std::string>& urls, int threadCount);

    // Time Compare mod: compare execution times with different thread counts
    void compareExecutionTimes(const std::vector<std::string>& urls,
                                const std::vector<int>& threadCounts,
                                const std::string& outputFile);
};

#endif // THREAD_MANAGER_HPP
