// Patrick Brodie


#include "node.h"


struct trooplist {
	char troop[16];
	struct trooplist *next;
};

struct trooplist *TroopTable[5] = {0};


void print_trooplist (struct trooplist *list)
{
	while (list) {
		printf ("%s -> ", list -> troop);
		list = list -> next;
	}
	printf ("\n");
}


void await_begin_order (int sockfd, struct sockaddr_in *serveraddr)
{
	int recvd, sent, addr_len;
	struct sockaddr_in client_addr;
	struct msg msg;

	addr_len = sizeof (struct sockaddr);

	while (1) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
					(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		register_local_event ();
		reconcile_clocks (myclock, msg.vclock);
		if (msg.type == BEGIN_OVERANXIUS) {
			printf ("Received begin order from caesar.\n");
			return;
		}
	}
}


void push_troop (struct trooplist **list, struct trooplist *troop)
{
	troop -> next = *list;
	*list = troop;
}


void store_troops (struct msg *msg)
{
	char *cp1, *cp2, *gib;
	struct trooplist *troopr;
	cp1 = msg -> text;

	gib = "gibberish";
	while (*cp1++ = *gib++);

	printf ("%s\n", msg -> text);
	while (*cp1) {
		printf ("%s\n", cp1);
		if (*cp1++ == '|') {
			troopr = (struct trooplist *) malloc (sizeof (struct trooplist));
			cp2 = troopr->troop;
			while (*cp1 && *cp1 != '|') {
				*cp2++ = *cp1++;
			}
			if (*cp1) {
				*cp2 = 0;
				push_troop (&(TroopTable[msg->ownerid]), troopr);
			}
		}
	}
}


void begin_contest (int sockfd, struct sockaddr_in *server_addr) {
	int sent, recvd;
	struct general *currgen;
	struct msg msg;
	struct sockaddr_in client_addr;
	int addr_len;

	addr_len = sizeof (struct sockaddr);
	currgen = genlist;

	while (currgen) {
		msg.type = BEGIN_P2P;
		msg.ownerid = myid;
		if (currgen->id != myid) {
			printf ("sending begin order to %d\n", currgen->id);
			register_local_event ();
			copy_vclock (msg.vclock, myclock);
			if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
						(struct sockaddr*) &(currgen -> server_addr), sizeof (struct sockaddr_in))) < 0) {
				perror ("Sending: ");
				exit (1);
			}
			printf ("waiting for troops from %d\n", currgen -> id);
			if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
				(struct sockaddr*)&client_addr, &addr_len)) < 0) {
				perror ("Receiving");
				exit (1);
			}
			if (msg.type == REPLY_TROOP_LIST) {
				store_troops (&msg);
			}
			print_trooplist (TroopTable[msg.ownerid]);
		}
		currgen = currgen -> next;
	}
}


// handle list, handle search, handle get.

void handlelist (struct msg *msg, int sockfd, struct sockaddr_in *server_addr, 
				struct sockaddr_in *client_addr)
{
	int sent;
	struct msg ret;
	struct trooplist *list;
	struct general *currgen;

	list = TroopTable[msg -> relaytoid];

	printf ("Responding to list request.\n");

	// copy list into message text field.
	bzero (ret.text, 512);
	strcat (ret.text, "gibberish|");
	while (list) {
		strcat (ret.text, list -> troop);
		strcat (ret.text, "|");
		list = list -> next;
	}

	printf ("preparing to send %s\n", ret.text);

	ret.type = LIST_RESPONSE;

	register_local_event ();
	copy_vclock (ret.vclock, myclock);


	if ((sent = sendto (sockfd, &ret, sizeof (struct msg), 0, 
				(struct sockaddr*) client_addr, sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending: ");
		exit (1);
	}


}

void handlesearch (struct msg *msg, int sockfd, struct sockaddr_in *server_addr, 
				struct sockaddr_in *client_addr)
{
	int i, sent, cmp = -1;
	char name[32], *cp;
	struct msg ret;
	struct trooplist *trp;

	// look for troop
	cp = msg -> text;

	bzero (name, 0);
	while (*cp++ != '|');
	strcpy (name, cp);

	for (i = 0; i < 5 && cmp != 0; ++i) {
		trp = TroopTable[i];
		while (trp) {
			printf ("looking for %s comparing to %s\n", name, trp->troop);
			if ((cmp = strcmp (name, trp -> troop)) == 0) break;
			trp = trp -> next;
		}
	}
	if (i == 5) {
		ret.relaytoid = -1;
	} else {
		ret.relaytoid = i - 1;
	}


	ret.type = SEARCH_RESPONSE;

	if ((sent = sendto (sockfd, &ret, sizeof (struct msg), 0, 
				(struct sockaddr*) client_addr, sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending: ");
		exit (1);
	}
}

void handleget (struct msg *msg, int sockfd, struct sockaddr_in *server_addr, 
			struct sockaddr_in *client_addr)
{
	
	// send relay message

	// shift location of troops

	


}


void handlemsg (struct msg *msg, int sockfd, struct sockaddr_in *server_addr, 
				struct sockaddr_in *client_addr)
{
	switch (msg->type) {
		case LIST: handlelist (msg, sockfd, server_addr, client_addr);	break;
		case SEARCHREQ: handlesearch (msg, sockfd, server_addr, client_addr); break;
		case GET: handleget (msg, sockfd, server_addr, client_addr); break;
		default: break;	
	}
}


void await_competitors (int sockfd, struct sockaddr_in *server_addr)
// Accept competitors (wait for RPC calls on this node)
{
	int recvd, sent, addr_len;
	struct sockaddr_in client_addr;
	struct msg msg;

	addr_len = sizeof (struct sockaddr);
	printf ("Accepting requests ... \n");

	while (1) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
					(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		register_local_event ();
		reconcile_clocks (myclock, msg.vclock);
		handlemsg (&msg, sockfd, server_addr, &client_addr);
	}
}