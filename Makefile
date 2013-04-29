nap: cmd.c menu.c node.h
	gcc -g -o nap cmd.c menu.c
clean:
	/bin/rm -rf nap nap.dSYM
