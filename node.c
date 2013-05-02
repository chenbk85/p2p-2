// Patrick Brodie
// CS 464
// 3/29/13
// <T.Gamage>
// PA2 - Overanxius - Distributed Mutual Exclusion (Ricart-Agrawala and RPC)


#include "node.h"


void allocate_general (int port, char *hostname)
// Allocate a new general at the given port and hostname.
{
	struct general *currgen = genlist;
	while (currgen -> next) {
		currgen = currgen -> next;
	}
	currgen -> next = (struct general*) malloc (sizeof (struct general));
	currgen = currgen -> next;
	currgen -> port = port;
	currgen -> replied = 0;
	strcpy (currgen -> hostname, hostname);
	currgen -> next = NULL;
}


void allocate_5_generals ()
// Allocate generals for 4-general scenario on DETER test project
{
	genlist = (struct general*) malloc (sizeof (struct general));
	genlist -> next = NULL;
	genlist -> port = 13000;
	genlist -> replied = 0;
	strcpy (genlist -> hostname, "node-0.ppa3.cpts464.isi.deterlab.net");
	allocate_general (13000, "node-1.ppa3.cpts464.isi.deterlab.net");
	allocate_general (13000, "node-2.ppa3.cpts464.isi.deterlab.net");
	allocate_general (13000, "node-3.ppa3.cpts464.isi.deterlab.net");
	allocate_general (13000, "node-4.ppa3.cpts464.isi.deterlab.net");
}



struct general *find_general (int id) 
//  Locate a general in the general list by id.
{
	struct general *g = genlist;
	while (g -> id != id) 
		g = g -> next;
	return g;
}


void copy_vclock (int *to, int *from) 
// Copy a vector clock from clock from to clock to.
{
	int num_gen;
	num_gen = get_size (genlist);
	while (num_gen) {
		*(to + num_gen - 1) = *(from + num_gen - 1);
		--num_gen;
	}
}


void register_local_event () 
// Increment the logical clock for this process.
{
	++myclock[myid];
}


void reconcile_clocks (int *to, int *from) 
// Update the local clock to reflect the clock that came in from a different proc.
{
	int num_gen, *ito, *ifrom;
	num_gen = get_size (genlist);
	while (num_gen) {
		ito = to + num_gen - 1;
		ifrom = from + num_gen - 1;
		if (*ito < *ifrom) {
			*ito = *ifrom;
		}
		--num_gen;
	}
}



void print_clock (int *clock)
// print a vector clock.
{
	int i, num_gen;
	num_gen = get_size (genlist);
	printf ("{");
	for (i = 0; i < num_gen; ++i) {
		printf (" %d ", clock[i]);
	}
	printf ("}\n");
}



void clean_up ()
// Free all allocated structures.
{
	struct general *tmp;
	struct msg *tmpmsg, *msg;
	// Free generals.
	while (genlist) {
		tmp = genlist;
		genlist = genlist -> next;
		free (tmp);
	}
}


int get_size (struct general *list)
// Return the number of generals contained in the given list.
{
	int size = 0;
	while (list) {
		list = list -> next;
		size++;
	}
	return size;
}


void init_generals () 
// Get the socket addresses of all generals and assign IDs based on the node number
// in DETER.
{
	struct hostent *host;
	struct general *currgen = genlist;
	char hostbuf[64];

	while (currgen){
		currgen -> id = (currgen -> hostname)[5] - '0'; //// THIS IS HARDCODED -- HACK ALERT
		currgen -> ready = UNKNOWN;
		gethostname (hostbuf, 64);
		if (strcmp (currgen-> hostname, hostbuf) == 0) {
			myid = currgen -> id;
		}
		host = (struct hostent *) gethostbyname (currgen -> hostname);
		(currgen -> server_addr).sin_family = AF_INET;
		(currgen -> server_addr).sin_port = htons (currgen -> port);
		(currgen -> server_addr).sin_addr = *((struct in_addr*) host -> h_addr);
		bzero (&((currgen -> server_addr).sin_zero), 8);
		currgen = currgen -> next;
	}
}


int init_server (int port, struct sockaddr_in *server_addr)
// Initialize a server listening on the given port, and assign it to the given
// socket address.
{
	int sockfd;
	
	// Get an open socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Socket: ");
            exit(1);
    }

    // Init server sockaddr
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons (port);
    server_addr->sin_addr.s_addr = INADDR_ANY;
    bzero (&(server_addr->sin_zero), 8);

    // bind server to socket
    if (bind (sockfd, (struct sockaddr*) server_addr, sizeof (struct sockaddr)) == -1) {
    	perror ("Bind");
    	exit (1);
    }

    return sockfd;
}	// init_server


void run_caesar ()
{
	int sockfd;
	struct sockaddr_in server_addr;

	printf ("\n\n========   I am Caesar.  Bow or be crushed.\n\n");
	caesar = find_general (myid);
	caesar -> name = CAESAR;
	printf ("Initializing server ...\n");
	sockfd = init_server (13000, &server_addr);	
	printf ("Registering generals ... \n");
	wait_for_heartbeats (get_size (genlist) - 1, sockfd);
	printf ("Acknowledging registration ... \n");
	sleep (2);
	send_caesar_heartbeat (sockfd, &server_addr);
	printf ("Ordering Overanxius to begin transfer ops ...\n");
	begin_overanxius (sockfd, &server_addr);
	printf ("Ordering population ...\n");
	await_orders (sockfd, &server_addr);
	participate (sockfd, &server_addr);
	
}


void run_overanxius ()
// Initialize and run overanxius' protocol.
{
	int sockfd;
	struct sockaddr_in server_addr;

	overanxius = find_general (myid);
	overanxius -> name = OVERANXIUS;
	// Initialize the server at an open socket.
	printf ("Initializing server ... \n");
	sockfd = init_server (13000, &server_addr);
	printf ("Broadcasting heartbeat ... \n");
	send_heartbeat (sockfd, &server_addr, OA_HEARTBEAT);
	printf ("Awaiting orders to begin transfer ...\n");
	await_begin_order (sockfd, &server_addr);
	printf ("Alerting generals to begin ... \n");
	begin_contest (sockfd, &server_addr);
	printf ("Accepting transfer requests ...\n");
	await_competitors (sockfd, &server_addr);

}


void run_general ()
// Init server on port 13000 and listen for orders.
// Future modification: allow port to be specified as parameter to this
// function, read in from a file of hostnames and ports.
{
	int sockfd;
	struct sockaddr_in server_addr;

	// Initialize the server at an open socket.
	printf ("Initializing server ... \n");
	sockfd = init_server (13000, &server_addr);
	printf ("Broadcasting heartbeat ... \n");
	send_heartbeat (sockfd, &server_addr);
	printf ("Awaiting orders ...\n");
	await_orders (sockfd, &server_addr);
	printf ("Order to begin accepting requests received.  Ready ...\n");
	participate (sockfd, &server_addr);
}


void alert_usage_and_exit ()
{
	printf ("USAGE: <exe> [flags: -c for Caesar | -o for Overanxius]\n");
}


int main (int argc, char *argv[])
// ./node -c for caesar
// can specify number of generals?
// read file of hostnames and ports?
{

	int port, caesar, overanxius;

	srand (time (NULL));
	if (argc > 2) {
		alert_usage_and_exit ();
	} else if (argc == 2) {
		caesar = (strcmp (argv[1], "-c") == 0);
		overanxius = (strcmp (argv[1], "-o") == 0);
		if (!caesar && !overanxius) 
			alert_usage_and_exit ();
	} 

	// ===== dummy initialization for 5-general scenario =====
	allocate_5_generals ();
	// =======================================================
	
	init_generals ();

	if (caesar) {
		run_caesar (); // what happens if two processes specify caesar?
	} else if (overanxius) {
		printf ("running overanxius\n");
		run_overanxius ();
	} else {
		run_general ();
	}
	clean_up ();
	return 0;
}



