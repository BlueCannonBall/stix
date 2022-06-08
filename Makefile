CXX = g++
CXXFLAGS = -s -Ofast -pthread
TARGET = stix
PREFIX = /usr/local

$(TARGET): main.cpp stix.hpp threadpool.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

.PHONY: clean install

clean:
	$(RM) $(TARGET)

install:
	cp $(TARGET) $(PREFIX)/bin/