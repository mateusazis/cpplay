CXXFLAGS+=-std=c++17
CXXFLAGS+=-Wc++11-extensions
CXXFLAGS+=-Wall
CC=$(CXX)

bin/%: %.o
		$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean

clean:
		rm bin/*
		rm *.o
