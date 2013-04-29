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

int do_list ()
// List troops at the given general's camp.
{
	char general[32];
	printf ("List troops at which general's camp? ");
	scanf ("%s", general);
	printf ("Listing troops currently at %s's camp\n", general);
	// Make connection to Overanxius
	// Call list_soldiers (general);
	// Destroy connection.
	return 0;
}

int do_search ()
// Search Overanxius for the given troop name.
{
	char troop[32];
	printf ("Search for which Centurion? ");
	scanf ("%s", troop);
	printf ("Searching for Centurion %s....\n", troop);
	// Make connection to overanxius
	// call search
	// Destroy connection
	// report if found and where (or not found)
	return 0;
}

int do_get ()
// Download the given file from the given general.
{
	char troop[32], general[32];
	printf ("Get which troop?");
	scanf ("%s", troop);
	printf ("From which general? ");
	scanf ("%s", general);
	printf ("Relocating Centurion %s to %s's camp.\n", troop, general);
	// Call Download function -- establish TCP connection and read/write the file.

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