nap: cmd.c menu.c general.c caesar.c node.c overanxius.c node.h
	gcc -g -o node cmd.c menu.c general.c caesar.c node.c overanxius.c
clean:
	/bin/rm -rf node node.dSYM
