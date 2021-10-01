
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"
#include "0x88_math.h"


int isAttacked( char byColor, U8 sq ) {

    /* pawns */
    if ( byColor == WHITE && b.pawn_ctrl[WHITE][sq] )
         return 1;
    
	if ( byColor == BLACK && b.pawn_ctrl[BLACK][sq])
		 return 1;

    /* knights (including knight-attacks from chancellors and archbishops) */
    if ( knightAttack( byColor, sq ) )
        return 1;

    /* kings */
    if ( leaperAttack( byColor, sq, KING ) )
        return 1;

    /* orthogonal sliders (rook, queen, chancellor) */
    if ( straightAttack( byColor, sq, NORTH ) 
	||   straightAttack( byColor, sq, SOUTH ) 
	||   straightAttack( byColor, sq, EAST  ) 
	||   straightAttack( byColor, sq, WEST  ) )
         return 1;

    /* diagonal sliders (bishop, queen, archbishop) */
    if ( diagAttack( byColor, sq, NE ) 
	||   diagAttack( byColor, sq, SE ) 
	||   diagAttack( byColor, sq, NW ) 
	||   diagAttack( byColor, sq, SW ) )
         return 1;

    return 0;
}

int knightAttack( char byColor, U8 sq ) {
    U8 nextSq;
    for ( int dir = 0; dir < num_moves[KNIGHT]; dir++ ) {
        nextSq = sq + vector[KNIGHT][dir];
        if ( IS_SQ(nextSq) &&
		     (isPiece( byColor, KNIGHT, nextSq ) || 
              isPiece( byColor, ARCHBISHOP, nextSq ) || 
              isPiece( byColor, CHANCELLOR, nextSq )) )
            return 1;
    }
    return 0;
}

int leaperAttack( char byColor, U8 sq, char byPiece ) {
    U8 nextSq;
    for ( int dir = num_slide_moves[byPiece]; dir < num_moves[byPiece]; dir++ ) {
        nextSq = sq + vector[byPiece][dir];
        if ( IS_SQ(nextSq) 
		&&  isPiece( byColor, byPiece, nextSq ) )
            return 1;
    }
    return 0;
}

int straightAttack(char byColor, U8 sq, int vect) {
    int nextSq = sq + vect;

    while ( IS_SQ(nextSq) ) {
        if ( b.color[nextSq] != COLOR_EMPTY ) {
            if ( (b.color[nextSq] == byColor) 
			&&   (b.pieces[nextSq] == ROOK || b.pieces[nextSq] == QUEEN || b.pieces[nextSq] == CHANCELLOR) )
               return 1;
            return 0;
        }
        nextSq = nextSq + vect;
    }
    return 0;
}

int diagAttack(int byColor, U8 sq, int vect) {
    int nextSq = sq + vect;

    while ( IS_SQ( nextSq ) ) {
        if ( b.color[ nextSq ] != COLOR_EMPTY ) {
            if ( (b.color[nextSq] == byColor) 
			&&   (b.pieces[nextSq] == BISHOP || b.pieces[nextSq] == QUEEN || b.pieces[nextSq] == ARCHBISHOP) )
               return 1;
            return 0;
        }
        nextSq = nextSq + vect;
    }
    return 0;
}

int bishAttack(int byColor, U8 sq, int vect) {
	int nextSq = sq + vect;

	while (IS_SQ(nextSq)) {
		if (b.color[nextSq] != COLOR_EMPTY) {
			if (b.color[nextSq] == byColor
			&&  b.pieces[nextSq] == BISHOP)
				return 1;
			return 0;
		}
		nextSq = nextSq + vect;
	}
	return 0;
}
