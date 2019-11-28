CFLAGS = -std=c89 -Wpedantic

all: easy
	
easy:
	gcc $(CFLAGS) -c my_lib.c settings.c
	gcc $(CFLAGS) settings.o my_lib.o master.c -o master
	gcc $(CFLAGS) settings.o my_lib.o giocatore.c -o giocatore

hard: 
	gcc $(CFLAGS) -c my_lib.c settings.c -D HARD
	gcc $(CFLAGS) settings.o my_lib.o master.c -o master
	gcc $(CFLAGS) settings.o my_lib.o giocatore.c -o giocatore

clean:
	rm -f *.o giocatore master *~

run:
	./master
