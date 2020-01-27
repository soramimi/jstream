CXXFLAGS = -std=c++11

all: a.out

a.out: main.o test.o
	g++ $^

clean:
	rm -f a.out
	rm -f *.o

