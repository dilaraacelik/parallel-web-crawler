# Parallel Web Crawler

A multi-threaded web crawler for real-time data collection from multiple sources, implemented in C++ using POSIX threads.

## Overview

This application allows you to crawl a list of URLs in parallel, leveraging multi-threading capabilities to significantly speed up the data collection process. It extracts important information like titles, descriptions, prices, and dates from web pages and saves them to a CSV file for further analysis.

## Features

- Multi-threaded crawling with configurable thread count
- HTTP requests using libcurl
- HTML parsing using regex patterns
- Progress visualization
- CSV export with proper field escaping
- Command-line interface with customizable options
- Performance comparison tools

## Requirements

- C++17 compatible compiler (g++ or clang++)
- libcurl development libraries
- POSIX threads support

### Installation on Ubuntu/Debian

```bash
sudo apt update
sudo apt install g++ make libcurl4-openssl-dev
```

### Installation on Windows with MinGW

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-curl make
```

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/parallel_crawler.git
cd parallel_crawler
```

2. Build the project:
```bash
make
```

## Usage

### Basic Usage

```bash
./bin/parallel_crawler -i data/urls.txt -o data/results.csv -t 4
```

### Command Line Options

- `-i, --input <file>`: Specify the input file containing URLs (default: data/urls.txt)
- `-o, --output <file>`: Specify the output file for results (default: data/results.csv)
- `-t, --threads <num>`: Specify the number of threads to use (default: 4)
- `-h, --help`: Display help message

### Input File Format

The input file should contain one URL per line. Lines starting with `#` are treated as comments and ignored.

Example:
```
# News sites
https://www.example.com
https://www.example.org
```

### Performance Testing

To compare the performance with different thread counts:

```bash
make time-compare
```

This will run the crawler with 1, 2, 4, and 8 threads and display the execution times.

## Project Structure

- `src/`: Source code files
- `include/`: Header files
- `data/`: Input and output data files
- `bin/`: Compiled binary
- `obj/`: Object files generated during compilation

## Implementation Details

### Components

1. **Crawler**: Manages multiple threads and distributes URLs among them
2. **Parser**: Extracts data from HTML content using regex patterns
3. **Utils**: Provides utility functions for file I/O, HTTP requests, etc.

### Threading Model

The crawler divides the URL list evenly among the specified number of threads. Each thread processes its assigned URLs independently. The results are collected in a thread-safe manner using mutexes.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Future Improvements

- Add support for JavaScript-rendered websites using a headless browser
- Implement a thread pool to avoid thread creation overhead
- Add proxy support for anonymization
- Implement more sophisticated HTML parsing using a dedicated library
- Add support for depth-based crawling of websites 