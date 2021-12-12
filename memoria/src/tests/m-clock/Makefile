sources = m-clock.c 
pathLibraries =  -lcommons -lpthread -lm

default: Mclock
all: default

Mclock: $(sources)
	gcc -g -o Mclock $(sources) $(pathLibraries)
clean: 
	rm Mclock
	rm *.o
