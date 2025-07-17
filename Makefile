CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

INCLUDES = -I./include

SRCS = main.cpp src/TaskQueue.cpp src/ThreadPool.cpp

OBJS = $(SRCS:.cpp=.o)

TARGET = threadpool

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean