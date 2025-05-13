#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

// URL listesi
std::vector<std::string> readUrlsFromFile(const std::string& filename) {
    std::vector<std::string> urls;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open input file: " << filename << std::endl;
        return urls;
    }
    
    std::string url;
    while (std::getline(file, url)) {
        // Trim whitespace
        url.erase(url.begin(), std::find_if(url.begin(), url.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        url.erase(std::find_if(url.rbegin(), url.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), url.end());
        
        // Skip empty lines and comments
        if (!url.empty() && url[0] != '#') {
            urls.push_back(url);
        }
    }
    
    file.close();
    return urls;
}

// HTML verilerini temsilen basit bir struct
struct CrawlResult {
    std::string url;
    std::string title = "Sample Title";
    std::string description = "Sample Description";
    bool success = true;
};

// İş parçacığı verileri
struct ThreadData {
    int id;
    std::vector<std::string>* urls;
    std::vector<CrawlResult>* results;
    std::mutex* resultsMutex;
    int startIndex;
    int endIndex;
    std::atomic<int>* completedUrls;
    int totalUrls;
};

// İlerleme göstergesi
void printProgress(int completed, int total) {
    const int barWidth = 50;
    float progress = static_cast<float>(completed) / total;
    int pos = barWidth * progress;
    
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% (" 
              << completed << "/" << total << ")\r";
    std::cout.flush();
    
    if (completed == total) {
        std::cout << std::endl;
    }
}

// İş parçacığı fonksiyonu
void crawlWorker(ThreadData* data) {
    // URL aralığını işle
    for (int i = data->startIndex; i < data->endIndex; i++) {
        if (i < data->urls->size()) {
            std::string url = (*data->urls)[i];
            
            // URL'yi işle (gerçekte HTTP isteği yapılacak)
            CrawlResult result;
            result.url = url;
            
            // Web isteği simülasyonu - gerçek kodda burada HTTP isteği olacak
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Web isteği simülasyonu
            
            // Sonucu ekle
            {
                std::lock_guard<std::mutex> lock(*data->resultsMutex);
                (*data->results)[i] = result;
            }
            
            // Tamamlanan sayısını artır ve ilerlemeyi göster
            int completed = ++(*data->completedUrls);
            if (data->id == 0) { // Sadece ilk iş parçacığı ilerlemeyi gösterir
                printProgress(completed, data->totalUrls);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    std::string inputFile = "data/urls.txt";
    std::string outputFile = "data/results.csv";
    int numThreads = 4;

    // Komut satırı argümanlarını işleme
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-t" && i + 1 < argc) {
            numThreads = std::stoi(argv[++i]);
        } else if (arg == "-i" && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }

    std::cout << "Test Crawler Application" << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    std::cout << "Number of threads: " << numThreads << std::endl;

    // URL listesini oku
    std::vector<std::string> urls = readUrlsFromFile(inputFile);
    if (urls.empty()) {
        std::cerr << "No URLs found in input file or file couldn't be opened." << std::endl;
        return 1;
    }

    std::cout << "Loaded " << urls.size() << " URLs." << std::endl;

    // Zamanı başlat
    auto startTime = std::chrono::high_resolution_clock::now();

    // İş parçacıkları ve veri yapılarını hazırla
    std::vector<std::thread> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);
    std::vector<CrawlResult> results(urls.size());
    std::mutex resultsMutex;
    std::atomic<int> completedUrls(0);

    // URL'leri iş parçacıkları arasında böl
    int urlsPerThread = urls.size() / numThreads;
    int extraUrls = urls.size() % numThreads;
    int startIndex = 0;

    // İş parçacıklarını oluştur ve başlat
    for (int i = 0; i < numThreads; i++) {
        threadData[i].id = i;
        threadData[i].urls = &urls;
        threadData[i].results = &results;
        threadData[i].resultsMutex = &resultsMutex;
        threadData[i].startIndex = startIndex;
        threadData[i].completedUrls = &completedUrls;
        threadData[i].totalUrls = urls.size();
        
        // Bu iş parçacığı için son indeksi hesapla
        int urlsForThisThread = urlsPerThread + (i < extraUrls ? 1 : 0);
        threadData[i].endIndex = startIndex + urlsForThisThread;
        
        // Sonraki iş parçacığı için başlangıç indeksini güncelle
        startIndex = threadData[i].endIndex;
        
        // İş parçacığını başlat
        threads[i] = std::thread(crawlWorker, &threadData[i]);
    }
    
    // Tüm iş parçacıklarının tamamlanmasını bekle
    for (int i = 0; i < numThreads; i++) {
        threads[i].join();
    }

    // Süreyi hesapla
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "Crawling completed in " << duration.count() / 1000.0 << " seconds." << std::endl;
    
    // Sonuçları CSV dosyasına yaz
    std::ofstream file(outputFile);
    if (file.is_open()) {
        file << "URL,Title,Description,Success\n";
        for (const auto& result : results) {
            file << result.url << ","
                << result.title << ","
                << result.description << ","
                << (result.success ? "true" : "false") << "\n";
        }
        file.close();
        std::cout << "Results saved to " << outputFile << std::endl;
    } else {
        std::cerr << "Failed to open output file for writing." << std::endl;
    }

    return 0;
} 