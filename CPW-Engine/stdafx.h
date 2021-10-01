
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#pragma once

#include <iostream>
#include <xmmintrin.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define INF 10000
#define INVALID 32767
#define MAX_DEPTH 100

#ifdef __linux__
typedef unsigned char U8;
typedef char S8;
typedef unsigned short int U16;
typedef short int S16;
typedef unsigned int U32;
typedef int S32;
typedef unsigned long long U64;
typedef long long S64;
#else
#define U64 unsigned __int64
#define U32 unsigned __int32
#define U16 unsigned __int16
#define U8  unsigned __int8
#define S64 signed   __int64
#define S32 signed   __int32
#define S16 signed   __int16
#define S8  signed   __int8
#endif

#ifdef _MSC_VER
#pragma warning( disable: 6385 )
#endif

/* Move ordering constants */

#define SORT_KING 400000000
#define SORT_HASH 200000000
#define SORT_CAPT 100000000
#define SORT_PROM  90000000
#define SORT_KILL  80000000

#define VERSION_STRING "1.2"

#define BOOK_NONE 0
#define BOOK_NARROW 1
#define BOOK_BROAD 2

#define NUM_TYPES 8
#define NUM_SQUARES 160
#define NUM_FILES 10
#define NUM_RANKS 8
#define NUM_PLAYERS 2
#define MAX_DIRECTIONS 12

enum epiece {
    KING = 0,
    QUEEN = 1,
    ROOK = 2,
    BISHOP = 3, 
    KNIGHT = 4,
    PAWN = 5,
    ARCHBISHOP = 6,
    CHANCELLOR = 7,
    PIECE_EMPTY = 8
};

enum ecolor {
    WHITE,
    BLACK,
    COLOR_EMPTY
};

enum esqare {
    A1 =   0, A2, A3, A4, A5, A6, A7, A8,
    B1 =  16, B2, B3, B4, B5, B6, B7, B8,
    C1 =  32, C2, C3, C4, C5, C6, C7, C8,
    D1 =  48, D2, D3, D4, D5, D6, D7, D8,
    E1 =  64, E2, E3, E4, E5, E6, E7, E8,
    F1 =  80, F2, F3, F4, F5, F6, F7, F8,
    G1 =  96, G2, G3, G4, G5, G6, G7, G8,
    H1 = 112, H2, H3, H4, H5, H6, H7, H8,
    I1 = 128, I2, I3, I4, I5, I6, I7, I8,
    J1 = 144, J2, J3, J4, J5, J6, J7, J8
};

enum ecastle {
    CASTLE_WK = 1,
    CASTLE_WQ = 2,
    CASTLE_BK = 4,
    CASTLE_BQ = 8
};

enum emflag {
    MFLAG_NORMAL = 0,
    MFLAG_CAPTURE = 1,
    MFLAG_EPCAPTURE = 2,
    MFLAG_CASTLE = 4,
    MFLAG_EP = 8,
    MFLAG_PROMOTION = 16,
    MFLAG_NULLMOVE = 32
};

struct sboard {
    U8 pieces[NUM_SQUARES];
    U8 color[NUM_SQUARES];
    char stm;        // side to move: 0 = white,  1 = black
    char castle;     // 1 = shortW, 2 = longW, 4 = shortB, 8 = longB
    U8 ep;         // en passant square
    U8 ply;
    U64 hash;
    U64	phash;
    int rep_index;
    U64 rep_stack[1024];
    U8 king_loc[NUM_PLAYERS];
    int pcsq_mg[NUM_PLAYERS];
    int pcsq_eg[NUM_PLAYERS];
    int piece_material[NUM_PLAYERS];
    int pawn_material[NUM_PLAYERS];
    U8 piece_cnt[NUM_PLAYERS][NUM_TYPES];
    U8 pawns_on_file[NUM_PLAYERS][NUM_FILES];
    U8 pawns_on_rank[NUM_PLAYERS][NUM_FILES];
    U8 pawn_ctrl[NUM_PLAYERS][NUM_SQUARES];
};
extern sboard b;


struct smove {
    char id;
    U8 from;
    U8 to;
    U8 piece_from;
    U8 piece_to;
    U8 piece_cap;
    char flags;
    char castle;
    char ply;
    U8 ep;
    int score;
};


struct sSearchDriver {
    int myside;
    U8 depth;
    int history[NUM_PLAYERS][NUM_SQUARES][NUM_SQUARES];
    int cutoff [NUM_PLAYERS][NUM_SQUARES][NUM_SQUARES];
    smove killers[1024][2];
    U64 nodes;
    S32 movetime;
    U64 q_nodes;
    unsigned long starttime;
};

extern sSearchDriver sd;

enum etimef {
    FTIME=1,
    FINC=2,
    FMOVESTOGO=4,
    FDEPTH=8,
    FNODES=16,
    FMATE=32,
    FMOVETIME=64,
    FINFINITE=128
};

enum class etask {
    TASK_NOTHING,
    TASK_SEARCH,
    TASK_PONDER,
    TASK_FORCE,
    TASK_WAIT,
	TASK_GAMEOVER
};

extern etask task;

struct structtime {
    int time[NUM_PLAYERS];
    int inc[NUM_PLAYERS];
    int movestogo;
    int depth;
    int nodes;
    int mate;
    int movetime;
    U8 flags;
};

extern structtime chronos;

struct s_options {
	int ponder;

};
extern s_options options;

struct s_eval_data {

    int PIECE_VALUE[NUM_TYPES];
    int SORT_VALUE[NUM_TYPES];

    /* Piece-square tables - we use size of the board representation,
    not 0..79, to avoid re-indexing. Initialization routine, however,
    uses 0..79 format for clarity */
    int mgPst[NUM_TYPES][NUM_PLAYERS][NUM_SQUARES];
    int egPst[NUM_TYPES][NUM_PLAYERS][NUM_SQUARES];

    /* piece-square tables for pawn structure */

    int weak_pawn[NUM_PLAYERS][NUM_SQUARES]; // isolated and backward pawns are scored in the same way
    int passed_pawn[NUM_PLAYERS][NUM_SQUARES];
    int protected_passer[NUM_PLAYERS][NUM_SQUARES];

    int sqNearK[NUM_PLAYERS][NUM_SQUARES][NUM_SQUARES];

    /* single values - letter p before a name signifies a penalty */

    int BISHOP_PAIR;
    int P_KNIGHT_PAIR;
    int P_ROOK_PAIR;
    int ROOK_OPEN;
    int ROOK_HALF;
    int P_BISHOP_TRAPPED_A7;
    int P_BISHOP_TRAPPED_A6;
    int P_KNIGHT_TRAPPED_A8;
    int P_KNIGHT_TRAPPED_A7;
    int P_BLOCK_CENTRAL_PAWN;
    int P_KING_BLOCKS_ROOK;

    int SHIELD_2;
    int SHIELD_3;
    int P_NO_SHIELD;

    int RETURNING_BISHOP;
    int P_C3_KNIGHT;
    int P_NO_FIANCHETTO;
    int FIANCHETTO;
    int TEMPO;
    int ENDGAME_MAT;
};
extern s_eval_data e;

extern char vector[NUM_TYPES][MAX_DIRECTIONS];
extern char num_slide_moves[NUM_TYPES];
extern char num_moves[NUM_TYPES];

void board_display();
void clearBoard();
void fillSq(U8 color, U8 piece, U8 sq);
void clearSq(U8 sq);
int board_loadFromFen(const char * fen);


int com_send(const char * command);
int com_sendmove(smove m);
int com_uci(char * command);
int com_xboard(char * command);
int com_nothing(char * command);
int com();
int com_init();
int com_ismove(char * command);
void CheckInput();


U8 movegen(smove * moves, U8 tt_move);
U8 movegen_qs(smove * moves);
void movegen_sort(U8 movecount, smove * m, U8 current);


void convert_0x88_a(U8 sq, char * a);
U8 convert_a_0x88(const char * a);
char * algebraic_writemove(smove m, char * a);
int algebraic_moves(char * a);


int move_make(smove move);
int move_unmake(smove move);
int move_makeNull();
int move_unmakeNull(U8 ep);

// the next couple of functions respond to questions about moves or move lists

int move_iscapt(smove m);
int move_isprom(smove m);
int move_canSimplify(smove m);
int move_countLegal();
int move_isLegal(smove m);


smove strToMove(char * a);

// subsidiary functions used to initialize opening book are hidden in book.h
// void initBook();
// int getBookMove(int book_type);


void search_run(); // interface of the search functions
void clearHistoryTable();


void setDefaultEval();
void setBasicValues();
void setSquaresNearKing();
void setPcsq();
void correctValues();
void readIniFile();
void processIniString(char line[250] );


int eval(int alpha, int beta, int use_hash);
int isPiece(U8 color, U8 piece, U8 sq);
int getTropism(int sq1, int sq2);
void printEval();
void printEvalFactor(int wh, int bl);


int Quiesce( int alpha, int beta );
int badCapture(smove move);
int Blind(smove move);

int isAttacked(char byColor, U8 sq);
int leaperAttack(char byColor, U8 sq, char byPiece);
int knightAttack(char byColor, U8 sq);
int straightAttack(char byColor, U8 sq, int vect);
int diagAttack(int byColor, U8 sq, int vect);
int bishAttack(int byColor, U8 sq, int vect);

void perft_start(char * command);
U64 perft(U8 depth);

void util_bench(char * command);
int util_pv(char * pv);

unsigned int gettime();
int time_uci_ponderhit();
void time_uci_go(char * command);
void time_xboard_go();
void time_nothing_go();
void time_calc_movetime();
bool time_stop_root();
bool time_stop();

int isRepetition();

int isDraw();

void printWelcome();
void printHelp();
void printStats();
void printSearchHeader();
