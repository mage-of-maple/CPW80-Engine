
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"

struct szobrist {
    U64 piecesquare[NUM_TYPES][NUM_PLAYERS][NUM_SQUARES];
    U64 color;
    U64 castling[16];
    U64 ep[NUM_SQUARES];
};

extern szobrist zobrist;

enum ettflag {
    TT_EXACT,
    TT_ALPHA,
    TT_BETA
};

struct stt_entry {
    U64  hash;
    int  val;
    U8	 depth;
    U8   flags;
    U8   bestmove;
};

extern stt_entry* tt;

struct spawntt_entry {
    U64  hash;
    int  val;
};

extern spawntt_entry* ptt;

struct sevaltt_entry {
    U64 hash;
    int val;
};

extern sevaltt_entry* ett;

extern int tt_size;
extern int ptt_size;
extern int ett_size;

U64 rand64();
int tt_init();
int tt_setsize(int size);
int tt_probe(U8 depth, int alpha, int beta, char * best);
void tt_save(U8 depth, int val, char flags, char best);
int ttpawn_setsize(int size);
int ttpawn_probe();
void ttpawn_save(int val);
int tteval_setsize(int size);
int tteval_probe();
void tteval_save(int val);
