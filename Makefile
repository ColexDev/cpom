cpom: src/main.c src/cpom.c
	gcc -o cpom -lncurses src/cpom.c src/main.c
install: cpom
	mv cpom /usr/bin
