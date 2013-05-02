// Patrick Brodie
// CS 464
// <T.Gamage>
// 3/29/13
// PA2 - Overanxius - Distributed Mutual Exclusion (Ricart-Agrawala and RPC)


#include "node.h"


void beat (int sockfd, struct sockaddr_in *server_addr, int *pipe, int beattype)
// Send heartbeat every second until parent proc sends signal to stop.
{
	int sent;
	struct general *currgen;
	struct msg msg;

	currgen = genlist;
	msg.type = beattype;
	msg.ownerid = myid;

	close (pipe[0]);
	while (1) {			
		currgen = genlist;
		while (currgen) {
			if (currgen -> id != myid) {
				register_local_event ();
				copy_vclock (msg.vclock, myclock);
				if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
						(struct sockaddr*) &(currgen -> server_addr), sizeof (struct sockaddr_in))) < 0) {
					perror ("Sending: ");
					exit (1);
				}
				write (pipe[1], myclock, sizeof (int) * 4);
				
			}
			currgen = currgen -> next;
		}
		sleep (1);
	}
}


void listen_for_Caesar (int sockfd, int *pipe)
// Listen for the I_AM_CAESAR message, then return to kill heartbeat.
{
	int recvd, addr_len, tmpclock[4];
	struct sockaddr_in client_addr;
	struct msg pmsg;

    addr_len = sizeof (struct sockaddr);
    close (pipe[1]);
    fcntl (pipe[0], F_SETFL, O_NONBLOCK);
	while (1) {
		if ((recvd = recvfrom (sockfd, &pmsg, sizeof (struct msg), 0, 
					(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		register_local_event ();
		while (read (pipe[0], tmpclock, sizeof (int) * 4) > 0) {
			reconcile_clocks (myclock, tmpclock);
		}
		reconcile_clocks (myclock, pmsg.vclock);
		if (!overanxius && pmsg.type == OA_HEARTBEAT) {
			printf ("Overanxius Found ...\n");
			overanxius = find_general (pmsg.ownerid);
			overanxius -> name = OVERANXIUS;
		}
		if (pmsg.type == I_AM_CAESAR) {
			printf ("\n\n====== Hail Caesar! At %s:%d.\n\n", inet_ntoa (client_addr.sin_addr), ntohs (client_addr.sin_port));
			caesar = find_general (pmsg.ownerid);
			caesar -> name = CAESAR;
			// Return to send signal to stop sending heartbeats.
		}
		if (overanxius && caesar) {
			break;
		}
	}
}


void send_heartbeat (int sockfd, struct sockaddr_in *server_addr, int beattype)
{
	// fork a proc: for each destination, send a heartbeat
	// meanwhile wait for incoming acknowledgement of the heartbeat.
	// kill child when heartbeat is ack'd

	int pid, ppid, p[2];

	// Setup pipe: pipe[0]: read end, pipe[1]: write end.
	if (pipe(p) < 0) {
		perror ("Piping: ");
		exit (1);
	}
	if ((pid = fork ()) < 0){
		perror ("Fork: ");
		exit (1);
	} else if (pid == 0) {	// Send heartbeat to other generals
		beat (sockfd, server_addr, p, beattype);
	} else {				// Await discovery of Caesar.
		listen_for_Caesar (sockfd, p);
		kill (pid, SIGKILL);
	}
}	// send_heartbeat


void send_troop_list (int over, int sockfd, struct sockaddr_in *server_addr)
{
	int sent;
	struct msg msg;
	DIR *dp;
	struct dirent *ep;
	if (caesar -> id == myid) { // get caesar's contents
		dp = opendir ("./caesar/");
	} else if (overanxius -> id == myid) {
		printf ("Overanxius don't play like that.  I gotta run this b*tch.\n");
		return;
	} else {
		switch (myid) {		// Can we get these ids to be changed if need be?  Assumes caesar and Ovr are 3 and 4.
			case 0: dp = opendir ("./brutus/"); break;
			case 1:	dp = opendir ("./pompus/"); break;
			case 2: dp = opendir ("./operachorus/"); break;
			default: printf ("general not found\n"); return;
		}
	}
	if (!dp) {
		printf ("could not open directory\n");
		exit (1);
	}

	// copy the filenames into the message buffer.

	msg.type = REPLY_TROOP_LIST;
	msg.ownerid = myid;
	bzero (msg.text, 512);
	strcat (msg.text, "gibberish|");
	while (ep = readdir (dp)) {
		if (ep->d_name[0] != '.') {
			strcat (msg.text, ep->d_name);
			strcat (msg.text, "|");
		}
	}
	printf ("sending %s\n", msg.text);
	register_local_event ();
	copy_vclock (msg.vclock, myclock);
	if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
			(struct sockaddr*) &(overanxius -> server_addr), sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending list to overanxius");
		exit (1);
	}

	return;




}



void await_orders (int sockfd, struct sockaddr_in *server_addr)
{
	int recvd, sent, addr_len;
	struct sockaddr_in client_addr;
	struct msg msg;
	char path[32];

	addr_len = sizeof (struct sockaddr);

	while (1) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
					(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		register_local_event ();
		reconcile_clocks (myclock, msg.vclock);
		if (msg.type == BEGIN_P2P && msg.ownerid == overanxius -> id) {
			printf ("received begin order from %d\n", msg.ownerid);
			printf ("sending troop list ...");
			send_troop_list (msg.ownerid, sockfd, server_addr);
			return;
		}

	}
}

void listen_for_get (int sockfd, struct sockaddr_in *server_addr)
// Loop continuously, handling get requests.
{
	int recvd, sent, addr_len;
	char name[32], path[32], *cp, *gib;
	struct sockaddr_in client_addr;
	struct msg msg, ret;
	FILE *fp;

	addr_len = sizeof (struct sockaddr);

	while (1) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
					(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		if (msg.type == GET) {
			printf ("Got request.\n");
			// look for troop
			cp = msg.text;

			bzero (name, 0);
			while (*cp++ != '|');
			strcpy (name, cp);

			printf ("looking for %s\n", name);

			bzero (path, 32);
			switch (name[0]) {
				case 'B': strcat (path, "./brutus/"); break;
				case 'C': strcat (path, "./caesar/"); break;
				case 'O': strcat (path, "./operachorus/"); break;
				case 'P': strcat (path, "./pompus/"); break;
			}

			strcat (path, name);

			if ((fp = fopen (path, "r")) == 0){
				printf ("unable to open %s\n", path);
				ret.relaytoid = -1;
			}

			// COPY FILE TO BUFFER HERE

			ret.type = GET_RESPONSE;
			ret.ownerid = myid;
			ret.relaytoid = 4;
			bzero (ret.text, 512);
			strcat (ret.text, "gibberish|");
			strcat (ret.text, name);

			printf ("sending %s\n", ret.text);
			if ((sent = sendto (sockfd, &ret, sizeof (struct msg), 0,
					(struct sockaddr*) &client_addr, sizeof (struct sockaddr_in))) < 0) {
				perror ("sending get reply");
				exit (1);
			}
			remove (path);
			fclose (fp);
		}
	}
}



void participate (int sockfd, struct sockaddr_in *server_addr)
{
	char cmd[32];
	int child, ret;
	system ("clear");
	printf ("\nRelocate your missing troops with Napsterius.\n\n");

	do_menu();
	while (1) {
		child = fork ();

		if (!child) {
			listen_for_get (sockfd, server_addr);
		} else if (child > 0) {
			printf ("Enter Command: ");
			scanf ("%s", cmd);
			kill (child, SIGKILL);
			ret = exec_cmd (cmd, sockfd, server_addr);
			printf ("returned from cmd call %s\n", cmd);
			bzero (cmd, 32);
		} else {
			printf ("Fork failed\n");
			exit (1);
		}
	}

}