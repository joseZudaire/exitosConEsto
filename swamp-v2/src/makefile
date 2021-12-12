sources = main.c lib.c swamp.c utils.c
pathLibraries =  -lcommons -lpthread -lm

default: Swamp
all: default

Swamp: $(sources)
	gcc -g -o Swamp $(sources) $(pathLibraries)
clean: 
	rm Swamp
	rm *.o
