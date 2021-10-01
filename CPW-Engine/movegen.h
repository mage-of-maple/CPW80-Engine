
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

void movegen_push(U8 from, U8 to, U8 piece_from, U8 piece_cap, char flags);
void movegen_pawn_move(U8 sq, bool promotion_only);
void movegen_pawn_capt(U8 sq);
