all : top cup

top : top.o
	gcc -o $@ $^ -lm

cup : cup.o
	gcc -o $@ $^ -lm

clean :
	rm -rf *.o *~
