
all:
	g++ main.cpp -o main -pthread -g -std=c++11

test:
	g++ test.cpp -o test -pthread -g -std=c++11