CXX = g++
CXXFLAGS = -Wall -g -std=c++11
OPENCV = `pkg-config --libs opencv`
LIBS = $(OPENCV)

motionDetection: motionDetection.cpp motionDetection.hpp
	$(CXX) $(CXXFLAGS) motionDetection.cpp $(LIBS) -o motionDetection

clean:
	rm -rf *.o
