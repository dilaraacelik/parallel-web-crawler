CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -mconsole
INCLUDES = -Iinclude
LIBS = -lcurl
SRCS = src/main.cpp src/utils.cpp src/thread_manager.cpp src/crawler.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = crawler

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(EXEC) $(LIBS)

clean:
	rm -f $(EXEC) src/*.o
