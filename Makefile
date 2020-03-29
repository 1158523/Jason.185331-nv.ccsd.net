linear: Makefile linear.c
	gcc -ggdb -std=c99 -Wall -Werror -o linear linear.c -lncurses

clean:
	rm -f *.o a.out core log.txt linear
