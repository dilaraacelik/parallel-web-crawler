@echo off
echo Setting up Windows environment for Parallel Web Crawler...

:: Check if MSYS2 is installed
if not exist C:\msys64\mingw64\bin\g++.exe (
    echo MSYS2 is not installed or not found in the expected location.
    echo Please install MSYS2 from https://www.msys2.org/
    exit /b 1
)

:: Add MinGW to PATH for this session
set PATH=C:\msys64\mingw64\bin;%PATH%

:: Install required packages
echo Installing required packages...
C:\msys64\usr\bin\pacman.exe -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-libcurl mingw-w64-x86_64-cmake

:: Create data directory if it doesn't exist
if not exist data mkdir data

:: Check if data/urls.txt exists, create if not
if not exist data\urls.txt (
    echo Creating sample URLs file...
    echo https://example.com > data\urls.txt
    echo https://example.org >> data\urls.txt
)

echo Environment setup complete!
echo.
echo To build the project, use the following commands:
echo mingw32-make clean
echo mingw32-make
echo.
echo To run the crawler:
echo bin\parallel_crawler.exe -i data\urls.txt -o data\results.csv -t 4 