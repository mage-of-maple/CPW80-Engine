
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

void search_iterate();
int search_widen(int depth, int val);
void search_clearDriver();
int search_root(U8 depth, int alpha, int beta);
int Search(U8 depth, U8 ply, int alpha, int beta, int can_null, int is_pv);
void setKillers(smove m, U8 ply);
void ReorderMoves(smove * m, U8 mcount, U8 ply);
int info_currmove(smove m, int nr);
int info_pv(int val);
unsigned int countNps(unsigned int nodes, unsigned int time);
void ageHistoryTable();
int contempt();
