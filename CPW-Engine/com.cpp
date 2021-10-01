
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"
#include "transposition.h"
#include "variant.h"


enum class eproto {
    PROTO_NOTHING,
    PROTO_XBOARD,
    PROTO_UCI
};

eproto mode = eproto::PROTO_NOTHING;

int debug = 0;

#ifndef __linux__
#include <windows.h>
int pipe;
HANDLE hstdin;

int com_init() {

    unsigned long dw;
    hstdin = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(hstdin, &dw);

    if (!pipe) {
        SetConsoleMode(hstdin,dw&~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(hstdin);
    } else {
        setvbuf(stdin,NULL,_IONBF,0);
        setvbuf(stdout,NULL,_IONBF,0);
    }

    /* default search settings */
    chronos.movetime = 5000;
    chronos.flags = FMOVETIME;

    printWelcome();

    return 0;
}

int input() {

    unsigned long dw=0;

    if (task == etask::TASK_NOTHING || task == etask::TASK_FORCE)
        return 1;

    // if (stdin->_cnt > 0) return 1;

    if (pipe) {
        if (!PeekNamedPipe(hstdin, 0, 0, 0, &dw, 0)) 
            return 1;
        return dw;
    } else {
        GetNumberOfConsoleInputEvents(hstdin, &dw);
        if (dw > 1)
        {
            task = etask::TASK_WAIT;
            return 1;
        }
    }

    return 0;
}
#else
#include "sys/time.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include "sys/select.h"
int com_init()
{

}

int input()
{
	  fd_set readfds;
	  struct timeval tv;
	  FD_ZERO (&readfds);
	  FD_SET (fileno(stdin), &readfds);
	  tv.tv_sec=0; tv.tv_usec=0;
	  int ret=select(16, &readfds, 0, 0, &tv);
	  if (ret==-1) {
	#ifndef NDEBUG
		  switch(errno) {
		  case EBADF:
			  printf("Bad file number???\n");
			  break;
		  case EINTR:
			  printf("******************************* Interrupt signal \n");
			  return 0;	//HACK
			  break;
		  case EINVAL:
			  printf("The timeout argument is invalid; one of the components is negative or too large.\n");
			  break;
		  }
	#else
		  // in release build just return 0 when select errors out.
		  return 0;
	#endif
	  }
	  return (FD_ISSET(fileno(stdin), &readfds));
}

#endif

char string[32767];
char *getsafe(char *buffer, int count)
{
	char *result = buffer, *np;
	if ((buffer == NULL) || (count < 1))
		result = NULL;
	else if (count == 1)
		*result = '\0';
	else if ((result = fgets(buffer, count, stdin)) != NULL)
	if (np = strchr(buffer, '\n'))
		*np = '\0';
	return result;
}

static char command[65536];

int com() {


    if (!input()) return 0;

    /* unwind the search-stack first */
    if (task == etask::TASK_SEARCH) {
        task = etask::TASK_NOTHING;
        return 0;
    }

    getsafe(command, sizeof command);

    switch (mode) {
	case eproto::PROTO_XBOARD :
        com_xboard(command);
        break;
    case eproto::PROTO_UCI    :
        com_uci(command);
        break;
    case eproto::PROTO_NOTHING:
        com_nothing(command);
        break;
    }

    return 0;
}

int com_nothing(char * command) {
	int converted;
	if (!strcmp(command, "xboard"))			com_xboard(command);
//	else if (!strcmp(command, "uci"))		com_uci(command);
	else if (!strncmp(command, "variant", 7)) com_xboard(command);
    else if (!strncmp(command,"perft", 5))	perft_start(command);
    else if (!strncmp(command,"bench", 5))	util_bench(command);
    else if (!strcmp(command, "eval"))      printEval();
    else if (!strcmp(command, "stat"))      printStats();
    else if (!strcmp(command, "d"))			board_display();
    else if (!strcmp(command, "new"))		board_loadFromFen(cv.startFEN);
    else if (!strncmp(command, "pos", 3))	board_loadFromFen(command+4);
    else if (!strcmp(command, "go"))		time_nothing_go();
    else if (!strcmp(command, "quit"))		exit(0);
    else if (!strcmp(command, "help"))		printHelp();
    else if (com_ismove(command) ) {
        if ( algebraic_moves(command) )
            time_nothing_go();
        else
            printf("Sorry, this is not a legal move\n");
    }
    else if (!strncmp(command, "st", 2)) {
        converted = sscanf(command, "st %d", &chronos.movetime);
        chronos.movetime *= 1000;
        chronos.flags = FMOVETIME;
    }
    else if (!strncmp(command, "sd", 2)) {
		converted = sscanf(command, "sd %d", &chronos.depth);
        chronos.flags = FDEPTH;
    }
    else if (command[0] == '\n')			{}
    else {
        strcat(command, " - UNKNOWN COMMAND (type 'help' for a list of commands)");
        com_send(command);
    }

    return 0;
}

int SetMemorySize(int mb_to_use)
{
	static int oldSize = 4;
	if (mb_to_use == oldSize) 
		// nothing to do
		return 0; 
	// remember current size
	oldSize = mb_to_use;
	int mb_used = tt_setsize(mb_to_use * 0x0100000) / 0x0100000;
	if (mb_used < mb_to_use) {
		// we have extra space for pawn hash table
		int spare_mb = mb_to_use - mb_used;
		mb_used = ttpawn_setsize(spare_mb * 0x0100000) / 0x0100000;
		if (mb_used < spare_mb)
			// we have extra space for eval cache
			tteval_setsize(spare_mb - mb_used);
		else
			// we need at least 1 MB for eval cache
			tteval_setsize(0x0100000); //  1m
	}
	else {
		// we always want at least 1 MB for the pawn table
		// and 1 MB for the eval cache so things keep working 
		ttpawn_setsize(0x0100000); //  1m
		tteval_setsize(0x0100000); //  1m
	}
}

void SendFeatures()
{
    printf("feature draw=0 ping=1 analyze=0 reuse=0 sigint=0 sigterm=0 memory=1\n");
    printf("feature myname=\"CPW-80 1.2\"\n");
    printf("feature variants=\"%s\"\n", cv.variants);
    printf("feature done=1\n");
}

int com_xboard(char * command) {
	int converted;
    if (!strcmp(command, "xboard"))
        mode = eproto::PROTO_XBOARD;

    else if (!strncmp(command, "protover", 8))
    {
        int protover;
        int l = sscanf(command, "protover %d", &protover);
        if (protover >= 2)
            SendFeatures();
    }

    else if (!strncmp(command, "variant", 7))
    {
		cv.VariantByName(command + 8);
		board_loadFromFen(cv.startFEN);
		cv.FinalizeInit();
	}

	else if (!strncmp(command, "memory", 6))
	{
		SetMemorySize(atoi(command + 7));
	}

    else if (!strcmp(command, "new"))
    {
		task = etask::TASK_FORCE;
        board_loadFromFen(cv.startFEN);
    }

    else if (!strcmp(command, "force"))
        task = etask::TASK_FORCE;

	else if (!strncmp(command, "ping", 4)) 
		printf("pong%s\n", command + 4);

    else if (!strcmp(command, "white"))
        ; // sd.myside = WHITE;

    else if (!strcmp(command, "black"))
        ; // sd.myside = BLACK;

    else if (!strncmp(command, "st", 2)) {
		converted = sscanf(command, "st %d", &chronos.movetime);
        chronos.movetime *= 1000;
        chronos.flags = FMOVETIME;
    }

    else if (!strncmp(command, "sd", 2)) {
		converted = sscanf(command, "sd %d", &chronos.depth);
        chronos.flags = FDEPTH;
    }

    else if (!strncmp(command, "time", 4)) {
		converted = sscanf(command, "time %d", &chronos.time[sd.myside]);
        chronos.time[sd.myside] *= 10;
        chronos.flags = FTIME;
    }

    else if (!strncmp(command, "otim", 4)) {
		converted = sscanf(command, "otim %d", &chronos.time[!sd.myside]);
		chronos.time[!sd.myside] *= 10;
		chronos.flags = FTIME;
}

    else if (!strcmp(command, "go"))
        time_xboard_go();

    else if (!strcmp(command, "hint")) {
        // hint
    }

    else if (!strcmp(command, "undo")) {
        // undo
    }

    else if (!strcmp(command, "remove")) {
        // remove
    }

    else if (!strcmp(command, "post")) {
        // post
    }

    else if (!strcmp(command, "nopost")) {
        // nopost
    }

    else if (!strcmp(command, "quit")) {
        exit(0);
    }

	else if (!strncmp(command, "result", 6)) {
		task = etask::TASK_GAMEOVER;
	}

    else if (com_ismove(command) && task != etask::TASK_GAMEOVER) {
        etask oldtask = task;
        task = etask::TASK_FORCE;
        algebraic_moves(command);
        if (oldtask != etask::TASK_FORCE)
        {
            task = oldtask;
            time_xboard_go();
        }
    }

    return 0;
}


int com_uci(char * command) {
	int converted;

    if (!strcmp(command, "uci")) {
        mode = eproto::PROTO_UCI;

        com_send("id name CPW-80 Engine 1.2");
        com_send("id author Computer Chess Wiki and Greg Strong");

        printf("option name Hash type spin default 64 min 1 max 1024\n");
		printf("option name Ponder type check default true\n");
        // send options

        com_send("uciok");
    }

    if (!strcmp(command, "isready"))
        com_send("readyok");

    if (!strncmp(command, "setoption", 9)) {
        char name[256];
        char value[256];

	    if (strstr(command, "setoption name Ponder value"))	
            options.ponder = (strstr(command, "value true") != 0);

		converted = sscanf(command, "setoption name %s value %s", name, value);

        if (!strcmp(name, "Hash")) {
            int val;
            converted = sscanf(value, "%d", &val);
            tt_setsize(val<<20);
            ttpawn_setsize(val<<18);
        }
    }

    if (!strcmp(command, "ucinewgame")) {}

    if (!strncmp(command, "position", 8)) {
        //position [fen | startpos] [moves ...]

        if (!strncmp(command,"position fen",12)) {
            board_loadFromFen(command + 13);
        } else {
            board_loadFromFen(cv.startFEN);
        }

        char * moves = strstr(command, "moves");
        if (moves) algebraic_moves(moves+6);
    }

    if (!strncmp(command, "go", 2))
        time_uci_go(command);

    if (!strncmp(command, "debug", 5))
        debug = strcmp(command,"debug off");

    if (!strcmp(command, "ponderhit"))
        time_uci_ponderhit();

    if (!strcmp(command, "stop"))
        task = etask::TASK_NOTHING;

    if (!strcmp(command, "quit"))
        exit(0);

    return 0;
}


int com_send(const char * command) {
    printf("%s\n",command);
    return 0;
}

int com_sendmove(smove m) {

    int promotion = 0;
    char parray[NUM_TYPES] = { 0, 'q', 'r', 'b', 'n', 'p', cv.archbishop, cv.chancellor };

    char command[20];
    char move[6];

    switch (mode) {
    case eproto::PROTO_XBOARD:
        strcpy(command,"move ");
        break;
    case eproto::PROTO_UCI:
        strcpy(command,"bestmove ");
        break;
    default:
        strcpy(command,"CPW: ");
    }

    convert_0x88_a(m.from, move);
    convert_0x88_a(m.to, move+2);

    //Promotion piece
    if (m.piece_to != m.piece_from) {
        promotion = m.piece_to;
    }
    move[4] = parray[promotion];
    move[5] = 0;

    strcat(command, move);

    com_send(command);

    /* in xboard and nothing actually do the move on the board */
    if (mode == eproto::PROTO_XBOARD || mode == eproto::PROTO_NOTHING)
        move_make(m);

    return 0;
}

int com_ismove(char * command) {
    return (command[0] >= 'a' && command[0] <= 'j' &&
            command[1] >= '1' && command[1] <= '8' &&
            command[2] >= 'a' && command[2] <= 'j' &&
            command[3] >= '1' && command[3] <= '8' &&
            ( command[4] == ' ' || command[4] == '\n' || command[4] == 0 ||
              command[4] == '-' ||
              command[4] == 'q' || command[4] == cv.archbishop || command[4] == cv.chancellor || command[4] == 'r' || command[4] == 'b' || command[4] == 'n'));

    /***************************************************************
    *  command[4] might be:                                        *
    *                                                              *
    *  (a) any kind of a blank space                               *
    *	(b) '-' or any other mark used in opening book processing   *
    *	(c) first letter of a name of a promoted piece              *
    ***************************************************************/
}
