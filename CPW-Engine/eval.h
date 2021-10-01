
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/


/* king safety*/
int wKingShield();
int bKingShield();

/* pawn structure */
int getPawnScore();
int evalPawnStructure();
int EvalPawn(U8 sq, S8 side);
void EvalKnight(U8 sq, S8 side);
void EvalBishop(U8 sq, S8 side);
void EvalRook(U8 sq, S8 side);
void EvalQueen(U8 sq, S8 side);
void EvalChancellor(U8 sq, S8 side);
void EvalArchbishop(U8 sq, S8 side);
int isPawnSupported(U8 sq, S8 side);
int isWPSupported(U8 sq);
int isBPSupported(U8 sq);

/* pattern detection */
void blockedPieces(int side);
