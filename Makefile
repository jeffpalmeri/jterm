main: main.o
	# gcc main.o -o main -lX11 -lfontconfig -lXft -lfreetype
	gcc main.o -o main `pkg-config --libs x11 xft fontconfig`

main.o: main.c
	# gcc -c main.c -o main.o `pkg-config --cflags freetype2`
	gcc -c main.c -o main.o `pkg-config --cflags x11 xft fontconfig`

clean:
	rm -rf main.o main .cache

