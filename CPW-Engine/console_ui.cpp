
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"
#include "variant.h"

void printWelcome() {
    printf(" CPW-80 chess engine ");
    printf(VERSION_STRING);
    printf("\n");
    printf("\n");
    printf(" CPW created by some members of Chessprogramming Wiki \n");
	printf(" http://chessprogramming.wikispaces.com/ \n\n");
	printf(" modified to 80-square variant XBoard engine by Greg Strong\n\n");
    printf(" type 'help' for a list of commands \n\n");
}

void printHelp() {
    printf("------------------------------------------ \n");
	printf("variant x =  variant to play (list below) \n");
    printf("d         =  display current board position \n");
    printf("bench n   =  test search speed to depth n \n");
    printf("perft n   =  test perft numbers up to depth n \n");
    printf("eval      =  display evaluation details \n");
    printf("stat      =  display search statistics \n");
    printf("go        =  play for the side to move \n");
    printf("new       =  start a new game \n");
    printf("sd n      =  set search depth to n plies \n");
    printf("st n      =  set search time to n seconds \n");
    printf("quit      =  exit CPW engine \n");
	printf("------------------------------------------ \n");
	printf("variants: \n");
	printf(cv.variant_display[0]);
	printf(cv.variant_display[1]);
	printf(cv.variant_display[2]);
	printf("------------------------------------------ \n");
    printf("\n");
    printf("Please enter moves in algebraic notation (e2e4 d7d5 e4d5 d8d5 ... b7b8q) \n");
    printf("or better yet, use a GUI compliant with the XBoard/WinBoard protocol \n");
    printf("------------------------------------------ \n");
}

void printStats() {
    U64 nodes = sd.nodes + (sd.nodes == 0);

    printf("-----------------------------\n");
    printf("Nodes       : %llu \n", sd.nodes);
    printf("Quiesc nodes: %llu \n", sd.q_nodes);
    printf("Ratio       : %llu %\n", sd.q_nodes * 100 / nodes);
    printf("-----------------------------\n");
}

void printSearchHeader() {
    printf("-------------------------------------------------------\n");
    printf( "ply      nodes   time score pv\n");
    printf("-------------------------------------------------------\n");
}