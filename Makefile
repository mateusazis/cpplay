CXXFLAGS+=-std=c++17
CXXFLAGS+=-Wc++11-extensions
CXXFLAGS+=-Wall
CXXFLAGS+=-DACTUAL_MAIN=mcsp
CC=$(CXX)

mcsp: main.o mcsp.o
