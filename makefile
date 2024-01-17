make: 
	clear
	gcc main.c \
	src/*.c \
	-g -o router -lpthread