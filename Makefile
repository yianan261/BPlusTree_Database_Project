CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
OBJS = main.o BPlusTree.o BPlusNode.o BTreeIndex.o LeaderDB.o WriteAhreadLog.o
TARGET = leaderdb

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
