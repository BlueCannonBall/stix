CXX = g++
CXXFLAGS = -s -Ofast
TARGET = stix
PREFIX = /usr/local

$(TARGET): main.cpp stix.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

.PHONY: clean install

clean:
	$(RM) $(TARGET)

install:
	cp $(TARGET) $(PREFIX)/bin/