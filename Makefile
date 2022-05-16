# sample Makefile
RES = src/fm0509.cpp
EXE = pa2
all:
	g++ -std=c++14 src/fm0509.cpp -o pa2 -O3
	g++ -std=c++14 src/verifier.cpp -o ver -O2
clean:
	rm $(EXE)

