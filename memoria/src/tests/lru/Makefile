sources = lru.c 
pathLibraries =  -lcommons -lpthread -lm

default: Lru
all: default

Lru: $(sources)
	gcc -g -o Lru $(sources) $(pathLibraries)
clean: 
	rm Lru
	rm *.o
