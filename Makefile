p2: main.o event.o manager.o resource.o system.o
	gcc -g -Wall -Wextra -pthread -o p2 main.o event.o manager.o resource.o system.o

main.o: main.c defs.h
	gcc -g -Wall -Wextra -pthread -c main.c

event.o: event.c defs.h
	gcc -g -Wall -Wextra -pthread -c event.c

manager.o: manager.c defs.h
	gcc -g -Wall -Wextra -pthread -c manager.c

resource.o: resource.c defs.h
	gcc -g -Wall -Wextra -pthread -c resource.c

system.o: system.c defs.h
	gcc -g -Wall -Wextra -pthread -c system.c

clean: 
	rm -f main.o event.o manager.o resource.o system.o p2

run: p2
	./p2