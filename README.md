# Parallel Web Crawler

A high-performance web crawler that utilizes multi-threading to efficiently crawl multiple URLs in parallel. This project demonstrates the power of parallel processing in web crawling applications.

## Features

- Multi-threaded web crawling for improved performance
- Two operation modes:
  - Normal Mode: Crawl URLs with user-specified thread count
  - Time Comparison Mode: Compare performance across different thread counts (1, 2, 4, 8, 16 threads)
- Organized output structure with timestamped folders
- Detailed logging for each thread
- Performance metrics and speedup analysis

## Prerequisites

- C++17 or later
- Make
- pthread library
- libcurl library

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/parallel-web-crawler.git
cd parallel-web-crawler
```

2. Build the project:
```bash
make
```

## Usage

1. Prepare your URLs:
   - Add the URLs you want to crawl in `data/url.txt`
   - One URL per line

2. Run the program:
```bash
./crawler
```

3. Choose the operation mode:
   - Mode 1: Normal Mode
     - Enter the desired number of threads
     - The crawler will process all URLs using the specified thread count
   - Mode 2: Time Comparison Mode
     - The program will automatically test with 1, 2, 4, 8, and 16 threads
     - Results will be saved in `timeelapsed.txt`

## Output Structure

The program creates an organized output structure:
```
output/
└── run_YYYYMMDD_HHMMSS_threads_N/
    ├── thread_0/
    │   └── log.txt
    ├── thread_1/
    │   └── log.txt
    └── ...
```

- Each run creates a timestamped folder
- Each thread has its own subfolder with a log file
- Performance comparison results are saved in `timeelapsed.txt`

## Performance Analysis

The Time Comparison Mode provides:
- Execution time for each thread count
- Speedup ratio compared to single-threaded execution
- Detailed performance metrics

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is licensed under the MIT License - see the LICENSE file for details. 