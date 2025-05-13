cmake_minimum_required(VERSION 3.10)
project(ParallelWebCrawler)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Source files
file(GLOB SOURCES "src/*.cpp")

# Include directories
include_directories(include)

# Find packages
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Windows-specific configuration
if(WIN32)
    # For pthread compatibility on Windows
    add_definitions(-DHAVE_STRUCT_TIMESPEC)
    
    # Add pthread include directories
    if(EXISTS "C:/msys64/mingw64/include")
        include_directories("C:/msys64/mingw64/include")
    endif()
    
    # Add pthread library directories
    if(EXISTS "C:/msys64/mingw64/lib")
        link_directories("C:/msys64/mingw64/lib")
    endif()
    
    # Add pthread library
    set(PTHREAD_LIB pthread)
else()
    # Unix-like systems have native pthread support
    set(PTHREAD_LIB pthread)
endif()

# Create executable
add_executable(parallel_crawler ${SOURCES})

# Link libraries
target_link_libraries(parallel_crawler ${CURL_LIBRARIES} ${PTHREAD_LIB})

# Create directories for data
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)

# Copy sample data files if they don't exist
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/data/urls.txt)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/data/urls.txt "https://example.com\nhttps://example.org\n")
endif() 