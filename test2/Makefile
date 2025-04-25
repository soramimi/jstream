
all: a.out run

run:
	./a.out

a.out: main.o func.o test.o
	g++ -O2 -std=c++17 $^ -o $@ -lgtest

.cpp.o:
	g++ -c -O2 -std=c++17 $^ -o $@


clean:
	rm -f *.o
	rm -f a.out
