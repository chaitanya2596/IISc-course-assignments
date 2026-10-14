CXX      = g++
CXXFLAGS = -std=c++11 -O2 -w
TARGET   = tdma

$(TARGET): $(wildcard src/*.cpp) $(wildcard src/headerFiles/*.h)
	$(CXX) $(CXXFLAGS) -o $@ src/*.cpp

run: $(TARGET)
	./run.sh

clean:
	rm -f $(TARGET)
