# type "make" into terminal for ./rush

rush: main.o shell_util.o
	gcc -o rush main.o shell_util.o

main.o: main.c shell_util.h
	gcc -c main.c

shell_util.o: shell_util.c shell_util.h
	gcc -c shell_util.c
