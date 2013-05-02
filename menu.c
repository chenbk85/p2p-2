// Patrick Brodie

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

int exec_cmd (char *cmd, int sockfd, struct sockaddr_in *server_addr)
// execute a user-specified command.
{
	int ret;
	int num = get_cmd (cmd);
	switch (num) {
		case 0: ret = do_menu (); break;
		case 1: ret = do_list (sockfd, server_addr); break;
		case 2: ret = do_search (sockfd, server_addr); break;
		case 3: ret = do_get (sockfd, server_addr); printf ("back from get\n");break;
		case 4: ret = do_quit (); break;
		default: ret = do_invalid (); break;
	}
	return num;
}


/*int main (int argc, char *argv[])
// loop continually for input.
{
	char cmd[32];
	printf ("Relocate your missing troops with Napsterius.\n\n");

	while (1) {
		printf ("Enter Command: ");
		scanf ("%s", cmd);
		exec_cmd (cmd);
	}


}*/

