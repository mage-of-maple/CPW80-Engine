
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"
#include "0x88_math.h"
#include "movegen.h"
#include "variant.h"

U8 movecount;

smove * m;

char num_moves[NUM_TYPES] = { 8, 8, 4, 4, 8, 0, 12, 12 };
char num_slide_moves[NUM_TYPES] = { 0, 8, 4, 4, 0, 0, 4, 4 };
char vector[NUM_TYPES][MAX_DIRECTIONS] = {
    { SW, SOUTH, SE, WEST, EAST, NW, NORTH, NE                     }, // King
    { SW, SOUTH, SE, WEST, EAST, NW, NORTH, NE                     }, // Queen
    { SOUTH, WEST, EAST, NORTH                                     }, // Rook
    { SW, SE, NW, NE                                               }, // Bishop
    { -33, -31, -18, -14, 14, 18, 31, 33                           }, // Knight
    {                                                              }, // Pawn
    { SW, SE, NW, NE, -33, -31, -18, -14, 14, 18, 31, 33           }, // Archbishop
    { SOUTH, WEST, EAST, NORTH, -33, -31, -18, -14, 14, 18, 31, 33 }  // Chancellor
};


// returns movecount
U8 movegen(smove * moves, U8 tt_move) {

    m = moves;

    movecount = 0;

    // Castling
    if ( b.stm == WHITE ) {
        if ( b.castle & CASTLE_WK ) {
			// white castling to the east
			bool can_castle = true;
			int file, steps;
			// are the squares empty?
			for (file = COL(cv.kingStartSq[0]) + 1; file < COL(cv.rookStartSq[0][1]) && can_castle; file++)
				if (b.color[SET_SQ(ROW_1, file)] != COLOR_EMPTY)
					can_castle = false;
			// are the squares attacked?
			file = COL(cv.kingStartSq[0]);
			for (steps = 0; can_castle && steps <= cv.maxCastlingStepsEast; steps++)
			{
				U8 sq = SET_SQ(ROW_1, file);
				if (isAttacked(BLACK, sq))
					can_castle = false;
				else if( steps >= cv.minCastlingStepsEast )
					movegen_push(cv.kingStartSq[0], sq, KING, PIECE_EMPTY, MFLAG_CASTLE);
				file++;
			}
        }
        if ( b.castle & CASTLE_WQ ) {
			// white castling to the west
			bool can_castle = true;
			int file, steps;
			// are the squares empty?
			for (file = COL(cv.kingStartSq[0]) - 1; file > COL(cv.rookStartSq[0][0]) && can_castle; file--)
				if (b.color[SET_SQ(ROW_1, file)] != COLOR_EMPTY)
					can_castle = false;
			// are the squares attacked?
			file = COL(cv.kingStartSq[0]);
			for (steps = 0; can_castle && steps <= cv.maxCastlingStepsWest; steps++)
			{
				U8 sq = SET_SQ(ROW_1, file);
				if (isAttacked(BLACK, sq))
					can_castle = false;
				else if (steps >= cv.minCastlingStepsWest)
					movegen_push(cv.kingStartSq[0], sq, KING, PIECE_EMPTY, MFLAG_CASTLE);
				file--;
			}
		}
    } else {
        if ( b.castle & CASTLE_BK ) {
			// black castling to the east
			bool can_castle = true;
			int file, steps;
			// are the squares empty?
			for (file = COL(cv.kingStartSq[1]) + 1; file < COL(cv.rookStartSq[1][1]) && can_castle; file++)
				if (b.color[SET_SQ(ROW_8, file)] != COLOR_EMPTY)
					can_castle = false;
			// are the squares attacked?
			file = COL(cv.kingStartSq[1]);
			for (steps = 0; can_castle && steps <= cv.maxCastlingStepsEast; steps++)
			{
				U8 sq = SET_SQ(ROW_8, file);
				if (isAttacked(WHITE, sq))
					can_castle = false;
				else if (steps >= cv.minCastlingStepsEast)
					movegen_push(cv.kingStartSq[1], sq, KING, PIECE_EMPTY, MFLAG_CASTLE);
				file++;
			}
		}
        if ( b.castle & CASTLE_BQ ) {
			// black castling to the west
			bool can_castle = true;
			int file, steps;
			// are the squares empty?
			for (file = COL(cv.kingStartSq[1]) - 1; file > COL(cv.rookStartSq[1][0]) && can_castle; file--)
				if (b.color[SET_SQ(ROW_8, file)] != COLOR_EMPTY)
					can_castle = false;
			// are the squares attacked?
			file = COL(cv.kingStartSq[0]);
			for (steps = 0; can_castle && steps <= cv.maxCastlingStepsWest; steps++)
			{
				U8 sq = SET_SQ(ROW_8, file);
				if (isAttacked(WHITE, sq))
					can_castle = false;
				else if (steps >= cv.minCastlingStepsWest)
					movegen_push(cv.kingStartSq[1], sq, KING, PIECE_EMPTY, MFLAG_CASTLE);
				file--;
			}
		}
    }

	// iterate through all squares and generate moves for pieces thereupon
	for (int x =0; x < NUM_SQUARES/2; x++) {

		U8 sq = ((x / NUM_RANKS) * NUM_RANKS * 2) + (x % NUM_RANKS);

        if (b.color[sq] == b.stm) {

            if (b.pieces[sq] == PAWN) {
                movegen_pawn_move(sq, 0);
                movegen_pawn_capt(sq);
            } else {
				assert(b.pieces[sq] < (sizeof num_moves / sizeof num_moves[0]) && b.pieces[sq] >= 0);
                for (char dir=0; dir< num_moves[b.pieces[sq]]; dir++) {

                    for (U8 pos = sq;;) {

                        pos = pos + vector[b.pieces[sq]][dir];

                        if (! IS_SQ(pos)) break;

                        if (b.pieces[pos] == PIECE_EMPTY) {
                                movegen_push(sq, pos, b.pieces[sq], PIECE_EMPTY, MFLAG_NORMAL);
						} else {
							if (b.color[pos] != b.stm) 
                               movegen_push(sq, pos, b.pieces[sq], b.pieces[pos], MFLAG_CAPTURE);
                            break; // we're hitting a piece, so looping is over
                        }

                        if (dir >= num_slide_moves[b.pieces[sq]]) break;
                    }
                }
            }
        }
    }

    /* if we have a best-move fed into movegen(), then increase its score */

    if (  ( tt_move != -1 ) && ( tt_move < movecount ) ) moves[tt_move].score = SORT_HASH;

    return movecount;
}

U8 movegen_qs(smove * moves) {

    m = moves;

    movecount = 0;

    for (U8 sq=0; sq<NUM_SQUARES; sq++) {

        if (b.color[sq] == b.stm) {

            if (b.pieces[sq] == PAWN) {
                movegen_pawn_move(sq, 1);
                movegen_pawn_capt(sq);
            } else {
				assert(b.pieces[sq] < (sizeof num_moves / sizeof num_moves[0]) && b.pieces[sq] >= 0);
                for (char dir=0; dir< num_moves[b.pieces[sq]]; dir++) {

                    for (U8 pos = sq;;) {

                        pos = pos + vector[b.pieces[sq]][dir];

                        if (! IS_SQ(pos)) break;

                        if (b.pieces[pos] != PIECE_EMPTY) {
                            if (b.color[pos] != b.stm)
                                movegen_push(sq, pos, b.pieces[sq], b.pieces[pos], MFLAG_CAPTURE);
                            break; // we're hitting a piece, so looping is over
                        }

                        if (dir >= num_slide_moves[b.pieces[sq]]) break;
                    }
                }
            }
        }
    }

    return movecount;
}


void movegen_pawn_move(U8 sq, bool promotion_only) {

    if ( b.stm == WHITE ) {
        if (promotion_only && (ROW(sq) != ROW_7)) return;

        if (b.pieces[sq+NORTH] == PIECE_EMPTY) {
            movegen_push(sq, sq+NORTH, PAWN, PIECE_EMPTY, MFLAG_NORMAL);

            if ( ( ROW(sq) == ROW_2 )  // white double pawn push
            &&   ( b.pieces[sq+NORTH+NORTH] == PIECE_EMPTY ) ) 
                 movegen_push(sq, sq+NORTH+NORTH, PAWN, PIECE_EMPTY, MFLAG_EP);
        }
    } else {
        if (promotion_only && (ROW(sq) != ROW_2)) return;

        if (b.pieces[sq+SOUTH] == PIECE_EMPTY) {
            movegen_push(sq, sq+SOUTH, PAWN, PIECE_EMPTY, MFLAG_NORMAL);
            
			if ( ( ROW(sq) == ROW_7 ) // black double pawn push
            &&   ( b.pieces[sq+(SOUTH)+(SOUTH)] == PIECE_EMPTY ) ) 
                 movegen_push(sq, sq+(SOUTH)+(SOUTH), PAWN, PIECE_EMPTY, MFLAG_EP);
        }
    }
}

void movegen_pawn_capt(U8 sq) {
    if (b.stm == WHITE) {
        if (IS_SQ(sq+NW) && ((b.ep==sq+NW) || (b.color[sq+NW] == (b.stm^1)))) {
            movegen_push(sq, sq+NW, PAWN, b.pieces[sq+NW], MFLAG_CAPTURE);
        }
        if (IS_SQ(sq+NE) && ((b.ep==sq+NE) || (b.color[sq+NE] == (b.stm^1)))) {
            movegen_push(sq, sq+17, PAWN, b.pieces[sq+NE], MFLAG_CAPTURE);
        }
    } else {
        if (IS_SQ(sq+SE) && ((b.ep==sq+SE) || (b.color[sq+SE] == (b.stm^1)))) {
            movegen_push(sq, sq+SE, PAWN, b.pieces[sq+SE], MFLAG_CAPTURE);
        }
        if (IS_SQ(sq+SW) && (((b.ep==sq+SW && b.ep!=0)) || (b.color[sq+SW] == (b.stm^1)))) {
            movegen_push(sq, sq+SW, PAWN, b.pieces[sq+SW], MFLAG_CAPTURE);
        }
    }
}


void movegen_push(U8 from, U8 to, U8 piece_from, U8 piece_cap, char flags) {

    m[movecount].from = from;
    m[movecount].to = to;
    m[movecount].piece_from = piece_from;
    m[movecount].piece_to = piece_from;
    m[movecount].piece_cap = piece_cap;
    m[movecount].flags = flags;
    m[movecount].ply = b.ply;
    m[movecount].castle = b.castle;
    m[movecount].ep = b.ep;
    m[movecount].id = movecount;

	/**************************************************************************
	* Quiet moves are sorted by history score.                                *
	**************************************************************************/

    m[movecount].score = sd.history[b.stm][from][to];

    /**************************************************************************
	* Score for captures: add the value of the captured piece and the id      *
	* of the attacking piece. If two pieces attack the same target, the one   *
	* with the higher id (eg. Pawn=5) gets searched first. En passant gets    *
	* the same score as pawn takes pawn. Good captures are put at the front   *
	* of the list, bad captures - after ordinary moves.                       *
    **************************************************************************/

	if (piece_cap != PIECE_EMPTY) {
		if (Blind(m[movecount]) == 0) m[movecount].score = e.SORT_VALUE[piece_cap] + piece_from;
		else                          m[movecount].score = SORT_CAPT + e.SORT_VALUE[piece_cap] + piece_from;
	}

    if ((piece_from == PAWN) && (to == b.ep) && b.ep != 0) {
        m[movecount].score = SORT_CAPT + e.SORT_VALUE[PAWN] + 5;
        m[movecount].flags = MFLAG_EPCAPTURE;
    }

	/**************************************************************************
	* Put all possible promotion moves on the list and score them.            *
	**************************************************************************/

    if ((piece_from == PAWN) && ( (ROW(to)==ROW_1)||(ROW(to)==ROW_8) )) {
        m[movecount].flags |= MFLAG_PROMOTION;

        for (int nPromotion = 0; nPromotion < cv.num_promote_types; nPromotion++) {
            m[movecount+nPromotion] = m[movecount];
            m[movecount+nPromotion].piece_to = cv.promote_types[nPromotion];
            m[movecount+nPromotion].score += SORT_PROM + e.SORT_VALUE[cv.promote_types[nPromotion]];
            m[movecount+nPromotion].id = movecount+nPromotion;
        }
        movecount += cv.num_promote_types - 1;
    }

    movecount++;
}

void movegen_sort(U8 movecount, smove * m, U8 current) {

    //find the move with the highest score - hoping for an early cutoff

    int high = current;

    for (int i=current+1; i<movecount; i++) {
        if (m[i].score > m[high].score)
            high = i;
    }

    smove temp = m[high];
    m[high] = m[current];
    m[current] = temp;
}
