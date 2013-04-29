#include "node.h"



// Will need search by name function at overanxius.
// Will need to establish TCP connection between generals.
// Can do that in Download Function.  Just need one Download.
// Fork a process for each general that polls for download requests.

// Write put function if have enough time.





char *cmdlist[] = {"menu", "list", "search", "get", "quit", 0};


int get_cmd (char *cmd)
// Locate the command in the command list.
{
	int i = 0;
	while (cmdlist[i] != 0) {
		if (strcmp (cmd, cmdlist[i]) == 0) {
			return i;
		}
		++i;
	}
	return -1;
}

int exec_cmd (char *cmd)
// execute a user-specified command.
{
	int num = get_cmd (cmd);
	switch (num) {
		case 0: do_menu (); break;
		case 1: do_list (); break;
		case 2: do_search (); break;
		case 3: do_get (); break;
		case 4: do_quit (); break;
		default: do_invalid (); break;
	}
	return num;
}


int main (int argc, char *argv[])
// loop continually for input.
{
	char cmd[32];
	printf ("Relocate your missing troops with Napsterius.\n\n");
	while (1) {
		printf ("Enter Command: ");
		scanf ("%s", cmd);
		exec_cmd (cmd);
	}


}