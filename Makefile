CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0
LDFLAGS = 

# Detect OS
ifdef OS
    WINDOWS = 1
else
    ifeq ($(shell uname), Linux)
        LINUX = 1
    endif
endif

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Windows-specific settings
ifdef WINDOWS
    EXEC = $(BIN_DIR)/parallel_crawler.exe
    CXXFLAGS += -I/mingw64/include -D_GLIBCXX_USE_CXX11_ABI=0 -D_GLIBCXX_USE_C99=1
    LDFLAGS += -L/mingw64/lib -lcurl -lpthread
else
    EXEC = $(BIN_DIR)/parallel_crawler
    CXXFLAGS += -pthread
    LDFLAGS += -lcurl
endif

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean dirs

all: dirs $(EXEC)

dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR) data

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: all
	$(EXEC) -i data/simple_urls.txt -o data/results.csv -t 4

single-thread: all
	$(EXEC) -i data/simple_urls.txt -o data/results.csv -t 1

extended: all
	$(EXEC) -i data/simple_urls.txt -o data/results.csv -t 4 -e

time-compare: all
	@echo "Running single-threaded crawler..."
	@time $(EXEC) -i data/urls.txt -o data/results_single.csv -t 1
	@echo "\nRunning with 2 threads..."
	@time $(EXEC) -i data/urls.txt -o data/results_2threads.csv -t 2
	@echo "\nRunning with 4 threads..."
	@time $(EXEC) -i data/urls.txt -o data/results_4threads.csv -t 4
	@echo "\nRunning with 8 threads..."
	@time $(EXEC) -i data/urls.txt -o data/results_8threads.csv -t 8 