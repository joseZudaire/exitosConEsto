sources = main.c client.c server.c lib.c memory.c signals.c tests.c
pathLibraries =  -lcommons -lpthread -lm

default: Memoria
all: default

Memoria: $(sources)
	gcc -g -o Memoria $(sources) $(pathLibraries)
clean: 
	rm Memoria
	rm *.o
