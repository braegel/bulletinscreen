bulletinscreen: bulletinscreen.o
	gcc -o bulletinscreen bulletinscreen.o

bulletinscreen.o: bulletinscreen.c
	gcc -c bulletinscreen.c
