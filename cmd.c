// Patrick Brodie

#include "node.h"


int do_menu ()
// List the available commands for the user.
{
	printf ("Available Commands ******************************************\n");
	printf ("\n");
	printf ("   list  		       list troops at a given general's camp.\n");
	printf ("   search                search Overanxius for a given file.\n");
	printf ("   get                                download the given file.\n");
	printf ("\n");
	printf ("**************************************************************\n");
}

int do_list (int sockfd, struct sockaddr_in *server_addr)
// List troops at the given general's camp.
{
	int general, sent, recvd, addr_len;
	char *cp, *gib;
	struct msg msg;
	struct sockaddr_in client_addr;

	addr_len = sizeof (addr_len);

	printf ("List troops at which general's camp? ");
	scanf ("%d", &general);
	printf ("Listing troops currently at %d's camp\n", general);

	// send message to overanxius

	msg.type = LIST;
	msg.relaytoid = general;
	register_local_event ();
	copy_vclock (msg.vclock, myclock);
	if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
			(struct sockaddr*) &(overanxius -> server_addr), sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending list to overanxius");
		exit (1);
	}

	// wait for response to list
	while (1) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
						(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		if (msg.type == LIST_RESPONSE) break;
	}

	cp = msg.text;
	gib = "gibberish";
	while (*cp++ = *gib++);

	while (*cp) {
		if (*cp != '|') {
			putchar (*cp++);
		} else {
			putchar ('\n'); cp++;
		}
	}

	return 0;
}

int do_search (int sockfd, struct sockaddr_in *server_addr)
// Search Overanxius for the given troop name.
{
	int general, sent, recvd, addr_len;
	char *cp, *gib, trp[32];
	struct msg msg;
	struct sockaddr_in client_addr;

	addr_len = sizeof (addr_len);
	printf ("Search for which Centurion? ");
	scanf ("%s", trp);
	printf ("Searching for Centurion %s....\n", trp);
	
	// send message to overanxius

	bzero (msg.text, 512);
	cp = msg.text;
	gib = "gibberish|";
	while (*cp++ = *gib++);
	strcat (msg.text, trp);
	msg.type = SEARCHREQ;
	msg.ownerid = myid;
	register_local_event ();
	copy_vclock (msg.vclock, myclock);
	if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
			(struct sockaddr*) &(overanxius -> server_addr), sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending search req to overanxius");
		exit (1);
	}

	// wait for response from search
	while (1) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
						(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		if (msg.type == SEARCH_RESPONSE) break;
	}

	if (msg.relaytoid >= 0) {
		printf ("%s found at general %d's camp\n", trp, msg.relaytoid);
	} else {
		printf ("%s not found.\n", trp);
	}

	return 0;
}

int do_get (int sockfd, struct sockaddr_in *server_addr)
// Download the given file from the given general.
{
	char trp[32], filename[32], path[64];
	char *cp, *gib;
	int general, sent, recvd, addr_len;
	struct sockaddr_in *client_addr;
	struct general *currgen;
	struct msg msg, res;
	FILE *fp = NULL;


	printf ("Get which troop?");
	scanf ("%s", trp);
	printf ("From which general?");
	scanf ("%d", &general);
	printf ("Relocating Centurion %s to %d's camp.\n", trp, myid);

	bzero (msg.text, 512);
	cp = msg.text;
	gib = "gibberish|";
	while (*cp++ = *gib++);
	strcat (msg.text, trp);
	msg.type = GET;
	msg.ownerid = myid;

	currgen = genlist;

	while (currgen) {
		printf ("looking for general %d\n", general);
		if (currgen -> id == general) {
			register_local_event ();
			copy_vclock (msg.vclock, myclock);
			if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
					(struct sockaddr*) &(currgen -> server_addr), sizeof (struct sockaddr_in))) < 0) {
				perror ("Sending get");
				exit (1);
			}
			break;
		}
		currgen = currgen -> next;
	}
	if (!currgen) {
		printf ("general %d does not exist\n", general);
		return -1;
	}
	printf ("waiting for response\n");
	
	// wait for response from search
	while (1) {
		if ((recvd = recvfrom (sockfd, &res, sizeof (struct msg), 0, 
						(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		if (res.type == GET_RESPONSE) break;
	}

	printf ("Got response. relayto %d\n", res.relaytoid);

	if (res.relaytoid < 0) {
		printf ("%s not found at %d's camp.\n", trp, general);
		return -1;
	}
	printf ("1 ");

	cp = res.text;
	gib = "gibberish";
	while (*cp++ = *gib++);
	printf ("2 ");
	gib = filename;
	while (*cp && *cp != '|') {
		*gib++ = *cp++;
	}
	*gib = 0;
	bzero (path, 64);	
	switch (myid) {
		case 3: strcat (path, "./caesar/"); break;
		case 0: strcat (path, "./brutus/"); break;
		case 1: strcat (path, "./pompus/"); break;
		case 2: strcat (path, "./operachorus/"); break;
		case 4: printf ("Not Overanxius!\n"); return -1;
	}

	strcat (path, filename);

	printf ("3 \n");
	// Open file for write.
	printf ("Trying to open %s\n", path);

	if ((fp = fopen (path, "wx")) == 0) {
		perror ("Opening file");
		return -1;
	}

	// COPY FILE CONTENTS FROM BUFFER HERE

	fclose (fp);
	printf ("4\n");

	register_local_event ();
	copy_vclock (msg.vclock, myclock);
	if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
			(struct sockaddr*) &(currgen -> server_addr), sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending get");
		exit (1);
	}
	
	return 0;
}

int do_invalid ()
// Alert the user that the specified command is invalid.
{
	printf ("Invalid command.\n");
	return 0;
}

int do_quit ()
// Quit the program.
{
	printf ("Quitting.\n");
	exit (1);
}

