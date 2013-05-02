// Patrick Brodie



#ifndef NODE_H_
#define NODE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>

// ====== Message Types ======
#define I_AM_CAESAR		 	0		// Caesar's heartbeat.  Only one permitted.
#define HEARTBEAT 			1 		// General's heartbeat.
#define REQUEST_TROOP_COUNT	2		// Caesar requests a general send a headcount.
#define REPLY_TROOP_LIST	3		// General replies with a headcount.
#define SEND_TROOPS			4		// Caesar orders troops to be sent.
#define REINFORCEMENT		5		// A message containing troops.
#define BEGIN_OVERANXIUS	6		// Order Overanxius to begin execution
#define OA_HEARTBEAT		7		// Overanxius' heartbeat
#define LIST 				8		// Request to list all troops at a given general's camp
#define SEARCHREQ			9		// Find the general who has the given troop name.
#define GET 				10		// Request that a trooper be transferred here from another camp.
#define BEGIN_P2P 			11		// Begin the p2p program.
#define LIST_RESPONSE		12
#define SEARCH_RESPONSE		13
#define GET_RESPONSE 		14
// ===========================

// ======= Gen. Status =======
#define UNKNOWN 			0		// Heartbeat has not yet been registered.
#define READY				1 		// Heartbeat has been registered.
// ===========================

// ======== Gen. Types =======
#define GENERAL				0		// Non-Caesar General
#define CAESAR 				1       // Marker for Caesar
#define OVERANXIUS 			2
// ===========================


struct msg {
	int type;
	int ownerid;
	int relaytoid;
	int dmeclock;
	int vclock[4];
	char text[512];
	struct msg *next;
};


struct general {
	int id;
	int name;
	int ready;
	int port;
	int replied;
	char hostname[128];
	struct sockaddr_in server_addr;
	struct general *next;
};


// GLOBALS

struct general *genlist;			// List of all generals.
struct general *caesar;				// Pointer to caesar's general node.
struct general *overanxius;			// Pointer to Overanxius' general node.
struct general *mygeneral;			// Pointer to my general's node.
struct army *myarmy;				// Pointer to my army.
struct genstate *mystate;			// Struct for recording state.
int myid;
int myclock[5];


// PROTOTYPES

struct general *find_general (int);
void print_army (int);
void update_troop_count (struct army*, struct army*);
void zero_army (struct army*);
void reconcile_clocks (int*, int*);
void register_local_event ();
void copy_vclock (int*, int*);
void print_clock (int*);
void record_msg (struct msg*);

#endif

