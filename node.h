// Patrick Brodie
// CptS 464
// <T.Gamage>
// 4/26/13
// PA3 - P2P File Sharing


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
//#include "over.h"

// ====== Message Types ======
#define I_AM_CAESAR		 	0		// Caesar's heartbeat.  Only one permitted.
#define HEARTBEAT 			1 		// General's heartbeat.
#define REQUEST_TROOP_COUNT	2		// Caesar requests a general send a headcount.
#define REPLY_TROOP_COUNT	3		// General replies with a headcount.
#define SEND_TROOPS			4		// Caesar orders troops to be sent.
#define REINFORCEMENT		5		// A message containing troops.
#define ATTACK				6		// Cease execution.
#define MARKER				7		// Marker to begin snapshot.
#define BEGIN_OVERANXIUS	8		// Order Overanxius to begin execution
#define OA_HEARTBEAT		9		// Overanxius' heartbeat
#define CS_REQUEST			10		// Request for Critical Section
#define CS_REPLY			11		// Send a vote for a proc to enter CS
#define BEGIN_GAME			12 		// Order the game to begin.
#define STOP_GAME			13 		// End the game to announce the winner.
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


struct army {
	int catapults;
	int archers;
	int cavalry;
	int spearmen;
	int infantry; 
};

struct genstate {
	struct army army;
};


struct msg {
	int type;
	int ownerid;
	int dmeclock;
	int vclock[4];
	struct msg *next;
};

struct channelstate {
	int on;
	struct msg *recorded;
};

struct general {
	int id;
	int name;
	int ready;
	int port;
	int replied;
	char hostname[128];
	struct sockaddr_in server_addr;
	struct army army;
	struct channelstate channel;
	struct general *next;
};


// GLOBALS

struct general *genlist;			// List of all generals.
struct general *caesar;				// Pointer to caesar's general node.
struct general *overanxius;			// Pointer to Overanxius' general node.
struct general *mygeneral;			// Pointer to my general's node.
struct army *myarmy;				// Pointer to my army.
struct genstate *mystate;			// Struct for recording state.
int myid, avgcatapults, avgarchers, avgcavalry, avgspearmen, avginfantry;
int totalcat, totalarch, totalcav, totalspear, totalinf;
int myclock[5];
int RECORDING;


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
void record_state ();
int receive_marker (struct msg*);
void send_markers (int, struct sockaddr_in*);
int are_channels_on ();
void print_rec_state ();
void print_channel (struct channelstate*);
void print_channels ();

#endif

