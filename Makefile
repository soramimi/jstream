CXXFLAGS = -std=c++17

all: a.out

a.out: main.o test.o
	g++ $^

clean:
	jstream
	rm -f a.out
	rm -f *.o

