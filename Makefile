CXX = g++
CXXFLAGS = -O3
LDFLAGS = -lncurses

all: fmatrix

fmatrix: fmatrix.cpp
	$(CXX) $(CXXFLAGS) fmatrix.cpp -o fmatrix $(LDFLAGS)

install: fmatrix
	install -m 755 fmatrix /usr/local/bin/fmatrix

clean:
	rm -f fmatrix