all:
  gcc -g -c myshell.c
	gcc myshell.o -o myshell
	gcc matmult_t.c -o matmult_t
	gcc matformatter.c -o matformatter
clean:

	rm -f *.o myshell matmult_t matformatter *.txt *~

