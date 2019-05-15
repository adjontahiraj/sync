all: p3_grader

p3_grader: autograder_main.cpp thread_lib
	g++ autograder_main.cpp threads.o -o p3_grader

thread_lib:threads.cpp
	g++ -c threads.cpp -o threads.o
