CXX = g++
CXXFLAGS = -O3 -std=c++17
LDFLAGS = -lncursesw 

all: clean fmatrix

fmatrix: fmatrix.cpp
	$(CXX) $(CXXFLAGS) fmatrix.cpp -o fmatrix $(LDFLAGS)

install: fmatrix
	install -m 755 fmatrix /usr/local/bin/fmatrix

clean:
	rm -f fmatrix