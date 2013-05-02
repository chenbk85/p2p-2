// Patrick Brodie


#include "node.h"




void wait_for_heartbeats (int num_beats, int sockfd)
//	Listen for the given number of heartbeats.  When all are heard, return.
{
	int recvd, addr_len;
	struct msg msg;
	struct sockaddr_in client_addr;
	struct general *fromgen;

	addr_len = sizeof (addr_len);

	printf ("Waiting for General heartbeats ...\n");
	while (num_beats) {
		if ((recvd = recvfrom (sockfd, &msg, sizeof (struct msg), 0, 
					(struct sockaddr*) &client_addr, &addr_len)) < 0) {
			perror ("Receiving: ");
			exit (1);
		}
		register_local_event ();
		reconcile_clocks (myclock, msg.vclock);
		if (msg.type == HEARTBEAT) {
			fromgen = find_general (msg.ownerid);
			if (fromgen -> ready != READY){
				fromgen -> ready = READY;
				--num_beats;
			}
		} else if (msg.type == OA_HEARTBEAT) {
			fromgen = find_general (msg.ownerid);
			overanxius = fromgen;
			if (fromgen -> ready != READY) {
				fromgen -> ready == READY;
				--num_beats;
			}
		}
	}
}


void send_caesar_heartbeat (int sockfd, struct sockaddr_in *server_addr)
// Send out the caesar notification for generals to stop sending heartbeats.
{
	int sent;
	struct msg msg;
	struct general *currgen;

	currgen = genlist;
	msg.type = I_AM_CAESAR;
	msg.ownerid = myid;

	while (currgen) {
		if (currgen -> id != myid) {
			register_local_event ();
			copy_vclock (msg.vclock, myclock);
			if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
						(struct sockaddr*) &(currgen -> server_addr), sizeof (struct sockaddr_in))) < 0) {
				perror ("Sending: ");
				exit (1);
			}
		}
		currgen = currgen -> next;
	}
}

void begin_overanxius (int sockfd, struct sockaddr_in *server_addr)
// Send out the caesar notification for generals to stop sending heartbeats.
{
	int sent;
	struct msg msg;
	struct general *currgen;

	msg.type = BEGIN_OVERANXIUS;
	msg.ownerid = myid;

	
	register_local_event ();
	copy_vclock (msg.vclock, myclock);
	if ((sent = sendto (sockfd, &msg, sizeof (struct msg), 0, 
		(struct sockaddr*) &(overanxius -> server_addr), sizeof (struct sockaddr_in))) < 0) {
		perror ("Sending: ");
		exit (1);
	}
}

