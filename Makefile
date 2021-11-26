main: main.c
	gcc main.c alarme.c appLayer.c macros.c receive.c transmitter.c

clean: rm -f main.o receive_*
