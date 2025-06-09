all:
	gcc -g -fsanitize=address -lm src/*.c -o main 
