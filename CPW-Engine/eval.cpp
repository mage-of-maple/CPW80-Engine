
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"
#include "0x88_math.h"
#include "eval.h"
#include "transposition.h"

/******************************************************************************
*  We want our eval to be color-independent, i.e. the same functions ought to *
*  be called for white and black pieces. This requires some way of converting *
*  row and square coordinates.                                                *
******************************************************************************/

static const int seventh[NUM_PLAYERS] = { ROW(A7), ROW(A2) };
static const int eighth[NUM_PLAYERS]  = { ROW(A8), ROW(A1) };
static const int stepFwd[NUM_PLAYERS] = { NORTH, SOUTH };
static const int stepBck[NUM_PLAYERS] = { SOUTH, NORTH };

static const U8 player_sq[NUM_PLAYERS][NUM_SQUARES] = { {
        A1, A2, A3, A4, A5, A6, A7, A8,  0,  0,  0,  0,  0,  0,  0,  0,
        B1, B2, B3, B4, B5, B6, B7, B8,  0,  0,  0,  0,  0,  0,  0,  0,
        C1, C2, C3, C4, C5, C6, C7, C8,  0,  0,  0,  0,  0,  0,  0,  0,
        D1, D2, D3, D4, D5, D6, D7, D8,  0,  0,  0,  0,  0,  0,  0,  0,
        E1, E2, E3, E4, E5, E6, E7, E8,  0,  0,  0,  0,  0,  0,  0,  0,
        F1, F2, F3, F4, F5, F6, F7, F8,  0,  0,  0,  0,  0,  0,  0,  0,
        G1, G2, G3, G4, G5, G6, G7, G8,  0,  0,  0,  0,  0,  0,  0,  0,
        H1, H2, H3, H4, H5, H6, H7, H8,  0,  0,  0,  0,  0,  0,  0,  0,
        I1, I2, I3, I4, I5, I6, I7, I8,  0,  0,  0,  0,  0,  0,  0,  0,
        J1, J2, J3, J4, J5, J6, J7, J8,  0,  0,  0,  0,  0,  0,  0,  0 },
    {
        A8, A7, A6, A5, A4, A3, A2, A1,  0,  0,  0,  0,  0,  0,  0,  0,
        B8, B7, B6, B5, B4, B3, B2, B1,  0,  0,  0,  0,  0,  0,  0,  0,
        C8, C7, C6, C5, C4, C3, C2, C1,  0,  0,  0,  0,  0,  0,  0,  0,
        D8, D7, D6, D5, D4, D3, D2, D1,  0,  0,  0,  0,  0,  0,  0,  0,
        E8, E7, E6, E5, E4, E3, E2, E1,  0,  0,  0,  0,  0,  0,  0,  0,
        F8, F7, F6, F5, F4, F3, F2, F1,  0,  0,  0,  0,  0,  0,  0,  0,
        G8, G7, G6, G5, G4, G3, G2, G1,  0,  0,  0,  0,  0,  0,  0,  0,
        H8, H7, H6, H5, H4, H3, H2, H1,  0,  0,  0,  0,  0,  0,  0,  0,
        I8, I7, I6, I5, I4, I3, I2, I1,  0,  0,  0,  0,  0,  0,  0,  0,
        J8, J7, J6, J5, J4, J3, J2, J1,  0,  0,  0,  0,  0,  0,  0,  0
    } 
};

#define REL_SQ(cl, sq)       (player_sq[cl][sq])

/* adjustements of piece value based on the number of own pawns */
int n_adj[NUM_FILES+1] = { -20, -16, -12, -8, -4, -4,  0,  0,  4,  8, 12};
int r_adj[NUM_FILES+1] = {  15,  12,   9,  6,  3,  3,  0,  0, -3, -6, -9};

static const short SafetyTable[120] = {
      0,   0,   1,   2,   3,   5,   7,   9,  12,  15,
     18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
     68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
    140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
    260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
    377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
    494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

/******************************************************************************
*  This struct holds data about certain aspects of evaluation, which allows   *
*  our program to print them if desired.                                      *
******************************************************************************/

struct eval_vector {
    int gamePhase;   // function of piece material: 24 in opening, 0 in endgame
    int mgMob[2];     // midgame mobility
    int egMob[2];     // endgame mobility
    int attCnt[2];    // no. of pieces attacking zone around enemy king
    int attWeight[2]; // weight of attacking pieces - index to SafetyTable
	int mgTropism[2]; // midgame king tropism score
	int egTropism[2]; // endgame king tropism score
    int kingShield[2];
    int adjustMaterial[2];
    int blockages[2];
    int positionalThemes[2];
} v;

int eval( int alpha, int beta, int use_hash ) {
    int result = 0, mgScore = 0, egScore = 0;
    int stronger, weaker;

    /**************************************************************************
    *  Probe the evaluatinon hashtable, unless we call eval() only in order   *
	*  to display detailed result                                             *
    **************************************************************************/

    int probeval = tteval_probe();
    if (probeval != INVALID && use_hash)
        return probeval;

    /**************************************************************************
    *  Clear all eval data                                                    *
    **************************************************************************/

	v.gamePhase = b.piece_cnt[WHITE][KNIGHT] + b.piece_cnt[WHITE][BISHOP] + 2 * b.piece_cnt[WHITE][ROOK] + 4 * b.piece_cnt[WHITE][QUEEN] + 4 * b.piece_cnt[WHITE][ARCHBISHOP] + 4 * b.piece_cnt[WHITE][CHANCELLOR]
		        + b.piece_cnt[BLACK][KNIGHT] + b.piece_cnt[BLACK][BISHOP] + 2 * b.piece_cnt[BLACK][ROOK] + 4 * b.piece_cnt[BLACK][QUEEN] + 4 * b.piece_cnt[BLACK][ARCHBISHOP] + 4 * b.piece_cnt[BLACK][CHANCELLOR];

	for (int side = 0; side < NUM_PLAYERS; side++) {
		v.mgMob[side] = 0;
		v.egMob[side] = 0;
		v.attCnt[side] = 0;
		v.attWeight[side] = 0;
		v.mgTropism[side] = 0;
		v.egTropism[side] = 0;
		v.adjustMaterial[side] = 0;
		v.blockages[side] = 0;
		v.positionalThemes[side] = 0;
		v.kingShield[side] = 0;
	}

    /************************************************************************** 
	*  Sum the incrementally counted material and piece/square table values   *
	**************************************************************************/

    mgScore = b.piece_material[WHITE] + b.pawn_material[WHITE] + b.pcsq_mg[WHITE]
            - b.piece_material[BLACK] - b.pawn_material[BLACK] - b.pcsq_mg[BLACK];
    egScore = b.piece_material[WHITE] + b.pawn_material[WHITE] + b.pcsq_eg[WHITE]
            - b.piece_material[BLACK] - b.pawn_material[BLACK] - b.pcsq_eg[BLACK];

    /************************************************************************** 
	* add king's pawn shield score and evaluate part of piece blockage score  *
    * (the rest of the latter will be done via piece eval)                    *
	**************************************************************************/

    v.kingShield[WHITE] = wKingShield();
    v.kingShield[BLACK] = bKingShield();
    blockedPieces(WHITE);
	blockedPieces(BLACK);
    mgScore += (v.kingShield[WHITE] - v.kingShield[BLACK]);

    /* tempo bonus */
    if ( b.stm == WHITE ) result += e.TEMPO;
    else				  result -= e.TEMPO;

    /**************************************************************************
    *  Adjusting material value for the various combinations of pieces.       *
    *  Currently it scores bishop, knight and rook pairs. The first one       *
    *  gets a bonus, the latter two - a penalty. Beside that knights lose     *
	*  value as pawns disappear, whereas rooks gain.                          *
    **************************************************************************/

	if (b.piece_cnt[WHITE][BISHOP] > 1) v.adjustMaterial[WHITE] += e.BISHOP_PAIR;
	if (b.piece_cnt[BLACK][BISHOP] > 1) v.adjustMaterial[BLACK] += e.BISHOP_PAIR;
	if (b.piece_cnt[WHITE][KNIGHT] > 1) v.adjustMaterial[WHITE] -= e.P_KNIGHT_PAIR;
	if (b.piece_cnt[BLACK][KNIGHT] > 1) v.adjustMaterial[BLACK] -= e.P_KNIGHT_PAIR;
	if (b.piece_cnt[WHITE][ROOK] > 1  ) v.adjustMaterial[WHITE] -= e.P_ROOK_PAIR;
	if (b.piece_cnt[BLACK][ROOK] > 1  ) v.adjustMaterial[BLACK] -= e.P_ROOK_PAIR;

	v.adjustMaterial[WHITE] += n_adj[b.piece_cnt[WHITE][PAWN]] * b.piece_cnt[WHITE][KNIGHT];
	v.adjustMaterial[BLACK] += n_adj[b.piece_cnt[BLACK][PAWN]] * b.piece_cnt[BLACK][KNIGHT];
	v.adjustMaterial[WHITE] += r_adj[b.piece_cnt[WHITE][PAWN]] * b.piece_cnt[WHITE][ROOK];
	v.adjustMaterial[BLACK] += r_adj[b.piece_cnt[BLACK][PAWN]] * b.piece_cnt[BLACK][ROOK];

    result += getPawnScore();

    /**************************************************************************
    *  Evaluate pieces                                                        *
    **************************************************************************/

	for (int x = 0; x < NUM_SQUARES / 2; x++) {

		U8 sq = ((x / NUM_RANKS) * NUM_RANKS * 2) + (x % NUM_RANKS);

		if (b.color[sq] != COLOR_EMPTY) {
			switch (b.pieces[sq]) {
			case PAWN: // pawns are evaluated separately
				break;
			case KNIGHT:
				EvalKnight(sq, b.color[sq]);
				break;
			case BISHOP:
				EvalBishop(sq, b.color[sq]);
				break;
			case ROOK:
				EvalRook(sq, b.color[sq]);
				break;
			case QUEEN:
				EvalQueen(sq, b.color[sq]);
				break;
			case CHANCELLOR:
				EvalChancellor(sq, b.color[sq]);
				break;
			case ARCHBISHOP:
				EvalArchbishop(sq, b.color[sq]);
				break;
			case KING:
				break;
			}
		}
	}

    /**************************************************************************
    *  Merge  midgame  and endgame score. We interpolate between  these  two  *
    *  values, using a gamePhase value, based on remaining piece material on  *
	*  both sides. With less pieces, endgame score becomes more influential.  *
    **************************************************************************/

    mgScore += (v.mgMob[WHITE] - v.mgMob[BLACK]);
    egScore += (v.egMob[WHITE] - v.egMob[BLACK]);
	mgScore += (v.mgTropism[WHITE] - v.mgTropism[BLACK]);
	egScore += (v.egTropism[WHITE] - v.egTropism[BLACK]);
    if (v.gamePhase > 28) v.gamePhase = 28;
    int mgWeight = v.gamePhase;
    int egWeight = 28 - mgWeight;
    result += ( (mgScore * mgWeight) + (egScore * egWeight) ) / 28;

    /**************************************************************************
    *  Add phase-independent score components.                                *
    **************************************************************************/

    result += (v.blockages[WHITE] - v.blockages[BLACK]);
    result += (v.positionalThemes[WHITE] - v.positionalThemes[BLACK]);
	result += (v.adjustMaterial[WHITE] - v.adjustMaterial[BLACK]);

    /**************************************************************************
    *  Merge king attack score. We don't apply this value if there are less   *
    *  than two attackers or if the attacker has no queen, chancellor, or AB. *
    **************************************************************************/

    if (v.attCnt[WHITE] < 2 || b.piece_cnt[WHITE][QUEEN] + b.piece_cnt[WHITE][ARCHBISHOP] + b.piece_cnt[WHITE][CHANCELLOR] == 0) v.attWeight[WHITE] = 0;
    if (v.attCnt[BLACK] < 2 || b.piece_cnt[BLACK][QUEEN] + b.piece_cnt[BLACK][ARCHBISHOP] + b.piece_cnt[BLACK][CHANCELLOR] == 0) v.attWeight[BLACK] = 0;
    result += SafetyTable[v.attWeight[WHITE]];
    result -= SafetyTable[v.attWeight[BLACK]];

    /**************************************************************************
    *  Low material correction - guarding against an illusory material advan- *
    *  tage. Full blown program should have more such rules, but the current  *
    *  set ought to be useful enough. Please note that our code  assumes      *
	*  different material values for bishop and  knight.                      *
    *                                                                         *
    *  - a single minor piece cannot win                                      *
    *  - two knights cannot checkmate bare king                               *
    *  - bare rook vs minor piece is drawish                                  *
    *  - rook and minor vs rook is drawish                                    *
    **************************************************************************/

    if (result > 0) {
        stronger = WHITE;
        weaker = BLACK;
    } else {
        stronger = BLACK;
        weaker = WHITE;
    }

    if (b.pawn_material[stronger] == 0) {

        if (b.piece_material[stronger] < 400) return 0;

        if (b.pawn_material[weaker] == 0
                && (b.piece_material[stronger] == 2 * e.PIECE_VALUE[KNIGHT]))
            return 0;

        if (b.piece_material[stronger] == e.PIECE_VALUE[ROOK]
                && b.piece_material[weaker] == e.PIECE_VALUE[BISHOP]) result /= 4;

        if (b.piece_material[stronger] == e.PIECE_VALUE[ROOK]
                && b.piece_material[weaker] == e.PIECE_VALUE[KNIGHT]) result /= 4;

//      if (b.piece_material[stronger] == e.PIECE_VALUE[ROOK] + e.PIECE_VALUE[BISHOP]
//              && b.piece_material[weaker] == e.PIECE_VALUE[ROOK]) result /= 2;

//      if (b.piece_material[stronger] == e.PIECE_VALUE[ROOK] + e.PIECE_VALUE[KNIGHT]
//              && b.piece_material[weaker] == e.PIECE_VALUE[ROOK]) result /= 2;

//		if (b.piece_material[stronger] == e.PIECE_VALUE[QUEEN]
//			&& b.piece_material[weaker] == e.PIECE_VALUE[CHANCELLOR]) result /= 2;

//		if (b.piece_material[stronger] == e.PIECE_VALUE[QUEEN]
//			&& b.piece_material[weaker] == e.PIECE_VALUE[ARCHBISHOP]) result /= 2;

//		if (b.piece_material[stronger] == e.PIECE_VALUE[CHANCELLOR]
//			&& b.piece_material[weaker] == e.PIECE_VALUE[ARCHBISHOP]) result /= 2;
	}

    /**************************************************************************
    *  Finally return the score relative to the side to move.                 *
    **************************************************************************/

    if ( b.stm == BLACK ) result = -result;

    tteval_save(result);

    return result;
}

void EvalKnight(U8 sq, S8 side) {
    int att = 0;
    int mob = 0;
    int pos;

    /**************************************************************************
    *  Collect data about mobility and king attacks. This resembles move      *
    *  generation code, except that we are just incrementing the counters     *
    *  instead of adding actual moves.                                        *
    **************************************************************************/

    for (U8 dir=0; dir<num_moves[KNIGHT]; dir++) {
        pos = sq + vector[KNIGHT][dir];
        if ( IS_SQ(pos) && b.color[pos] != side ) {
			// we exclude mobility to squares controlled by enemy pawns
			// but don't penalize possible captures
			if (!b.pawn_ctrl[!side][pos]) ++mob;
            if ( e.sqNearK[!side] [b.king_loc[!side] ] [pos] )
                ++att; // this knight is attacking zone around enemy king
        }
    }

    /**************************************************************************
    *  Evaluate mobility. We try to do it in such a way that zero represents  *
	*  average mobility, but  our formula of doing so is a pure guess.        *
    **************************************************************************/

    v.mgMob[side] += 4 * (mob-4);
    v.egMob[side] += 4 * (mob-4);

    /**************************************************************************
    *  Save data about king attacks                                           *
    **************************************************************************/

    if (att) {
        v.attCnt[side]++;
        v.attWeight[side] += 2 * att;
    }

	/**************************************************************************
	* Evaluate king tropism                                                   *
	**************************************************************************/

	int tropism = getTropism(sq, b.king_loc[!side]);
	v.mgTropism[side] += 3 * tropism;
	v.egTropism[side] += 3 * tropism;
}

void EvalBishop(U8 sq, S8 side) {

    int att = 0;
    int mob = 0;

    /**************************************************************************
    *  Collect data about mobility and king attacks                           *
    **************************************************************************/

    for (char dir=0; dir<num_moves[BISHOP]; dir++) {

        for (U8 pos = sq;;) {

            pos = pos + vector[BISHOP][dir];
            if (! IS_SQ(pos)) break;

            if (b.pieces[pos] == PIECE_EMPTY) {
				if (!b.pawn_ctrl[!side][pos]) mob++;
				// we exclude mobility to squares controlled by enemy pawns
                if ( e.sqNearK[!side] [b.king_loc[!side] ] [pos] ) ++att;
			} else {                                // non-empty square
				if (b.color[pos] != side) {         // opponent's piece
					mob++;
					if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
				}
				break;                              // own piece
			}
        }
    }

    v.mgMob[side] += 3 * (mob-7);
    v.egMob[side] += 3 * (mob-7);

    if (att) {
        v.attCnt[side]++;
        v.attWeight[side] += 2*att;
    }

	int tropism = getTropism(sq, b.king_loc[!side]);
	v.mgTropism[side] += 2 * tropism;
	v.egTropism[side] += 1 * tropism;
}

void EvalRook(U8 sq, S8 side) {

    int att = 0;
    int mob = 0;

	/**************************************************************************
	*  Bonus for rook on the seventh rank. It is applied when there are pawns *
	*  to attack along that rank or if enemy king is cut off on 8th rank      *
	/*************************************************************************/

	if (ROW(sq) == seventh[side]
	&& (b.pawns_on_rank[!side][seventh[side]] || ROW(b.king_loc[!side]) == eighth[side])) {
		v.mgMob[side] += 20;
		v.egMob[side] += 30;
	}

    /**************************************************************************
    *  Bonus for open and half-open files is merged with mobility score.      *
	*  Bonus for open files targetting enemy king is added to attWeight[]     *
    /*************************************************************************/

	if (b.pawns_on_file[side][COL(sq)] == 0) {
		if (b.pawns_on_file[!side][COL(sq)] == 0) { // fully open file
            v.mgMob[side] += e.ROOK_OPEN;
            v.egMob[side] += e.ROOK_OPEN;
			if (abs(COL(sq) - COL(b.king_loc[!side])) < 2) 
			   v.attWeight[side] += 1;
        } else {                                    // half open file
            v.mgMob[side] += e.ROOK_HALF;
            v.egMob[side] += e.ROOK_HALF;
			if (abs(COL(sq) - COL(b.king_loc[!side])) < 2) 
			   v.attWeight[side] += 2;
        }
    }

    /**************************************************************************
    *  Collect data about mobility and king attacks                           *
    **************************************************************************/

    for (char dir=0; dir<num_moves[ROOK]; dir++) {

        for (U8 pos = sq;;) {

            pos = pos + vector[ROOK][dir];
            if (! IS_SQ(pos)) break;

            if (b.pieces[pos] == PIECE_EMPTY) {
                mob++;
                if ( e.sqNearK[!side] [b.king_loc[!side] ] [pos] ) ++att;
			} else {                                // non-empty square
				if (b.color[pos] != side) {         // opponent's piece
					mob++;
					if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
				}
				break;                              // own piece
			}
        }
    }

    v.mgMob[side] += 2 * (mob-7);
    v.egMob[side] += 4 * (mob-7);

    if (att) {
        v.attCnt[side]++;
        v.attWeight[side] += 3*att;
    }

	int tropism = getTropism(sq, b.king_loc[!side]);
	v.mgTropism[side] += 2 * tropism;
	v.egTropism[side] += 1 * tropism;
}

void EvalQueen(U8 sq, S8 side) {

    int att = 0;
    int mob = 0;

	if (ROW(sq) == seventh[side]
		&& (b.pawns_on_rank[!side][seventh[side]] || ROW(b.king_loc[!side]) == eighth[side])) {
		v.mgMob[side] += 5;
		v.egMob[side] += 10;
	}

    /**************************************************************************
    *  A queen should not be developed too early                              *
    **************************************************************************/

	if ((side == WHITE && ROW(sq) > ROW_2) || (side == BLACK && ROW(sq) < ROW_7)) {
		if (isPiece(side, KNIGHT, REL_SQ(side, B1))) v.positionalThemes[side] -= 4;
		if (isPiece(side, KNIGHT, REL_SQ(side, I1))) v.positionalThemes[side] -= 4;
		if (isPiece(side, BISHOP, REL_SQ(side, D1))) v.positionalThemes[side] -= 2;
		if (isPiece(side, BISHOP, REL_SQ(side, G1))) v.positionalThemes[side] -= 2;
	}

    /**************************************************************************
    *  Collect data about mobility and king attacks                           *
    **************************************************************************/

    for (char dir=0; dir<num_moves[QUEEN]; dir++) {

        for (U8 pos = sq;;) {

            pos = pos + vector[QUEEN][dir];
            if (! IS_SQ(pos)) break;

            if (b.pieces[pos] == PIECE_EMPTY) {
                mob++;
                if ( e.sqNearK[!side] [b.king_loc[!side] ] [pos] ) ++att;
			} else {                                 // non-empty square
				if (b.color[pos] != side) {          // opponent's piece
					mob++;
					if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
				}
				break;                               // own piece
			}
        }
    }

    v.mgMob[side] += 1 * (mob-14);
    v.egMob[side] += 2 * (mob-14);

    if (att) {
        v.attCnt[side]++;
        v.attWeight[side] += 4*att;
    }

	int tropism = getTropism(sq, b.king_loc[!side]);
	v.mgTropism[side] += 2 * tropism;
	v.egTropism[side] += 4 * tropism;
}


void EvalChancellor(U8 sq, S8 side) {

    int att = 0;
    int mob = 0;

    // Chancellor gets bonus for pinning the king on the 8th rank same as the Queen
    if (ROW(sq) == seventh[side]
        && (b.pawns_on_rank[!side][seventh[side]] || ROW(b.king_loc[!side]) == eighth[side])) {
        v.mgMob[side] += 5;
        v.egMob[side] += 10;
    }

	/**************************************************************************
	*  A chancellor should not be developed too early                         *
	**************************************************************************/

	if ((side == WHITE && ROW(sq) > ROW_1) || (side == BLACK && ROW(sq) < ROW_8)) {
		if (isPiece(side, KNIGHT, REL_SQ(side, B1))) v.positionalThemes[side] -= 5;
		if (isPiece(side, KNIGHT, REL_SQ(side, I1))) v.positionalThemes[side] -= 5;
		if (isPiece(side, BISHOP, REL_SQ(side, D1))) v.positionalThemes[side] -= 2;
		if (isPiece(side, BISHOP, REL_SQ(side, G1))) v.positionalThemes[side] -= 2;
	}

    /**************************************************************************
    *  Collect data about mobility and king attacks                           *
    **************************************************************************/

    for (char dir = 0; dir < num_moves[CHANCELLOR]; dir++) {

        for (U8 pos = sq;;) {

            pos = pos + vector[CHANCELLOR][dir];
            if (!IS_SQ(pos)) break;

            if (b.pieces[pos] == PIECE_EMPTY) {
                mob++;
                if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
            }
            else {                                 // non-empty square
                if (b.color[pos] != side) {          // opponent's piece
                    mob++;
                    if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
                }
                break;                               // own piece
            }
            if (dir > num_slide_moves[CHANCELLOR])
                break;
        }
    }

    v.mgMob[side] += 1 * (mob - 14);
    v.egMob[side] += 2 * (mob - 14);

    if (att) {
        v.attCnt[side]++;
        v.attWeight[side] += 4 * att;
    }

    int tropism = getTropism(sq, b.king_loc[!side]);
    v.mgTropism[side] += 2 * tropism;
    v.egTropism[side] += 3 * tropism;
}

void EvalArchbishop(U8 sq, S8 side) {

    int att = 0;
    int mob = 0;

	/**************************************************************************
	*  An archbishop should not be developed too early                        *
	**************************************************************************/

	if ((side == WHITE && ROW(sq) > ROW_2) || (side == BLACK && ROW(sq) < ROW_7)) {
		if (isPiece(side, KNIGHT, REL_SQ(side, B1))) v.positionalThemes[side] -= 3;
		if (isPiece(side, KNIGHT, REL_SQ(side, I1))) v.positionalThemes[side] -= 3;
		if (isPiece(side, BISHOP, REL_SQ(side, D1))) v.positionalThemes[side] -= 1;
		if (isPiece(side, BISHOP, REL_SQ(side, G1))) v.positionalThemes[side] -= 1;
	}

    /**************************************************************************
    *  Collect data about mobility and king attacks                           *
    **************************************************************************/

    for (char dir = 0; dir < num_moves[ARCHBISHOP]; dir++) {

        for (U8 pos = sq;;) {

            pos = pos + vector[ARCHBISHOP][dir];
            if (!IS_SQ(pos)) break;

            if (b.pieces[pos] == PIECE_EMPTY) {
                mob++;
                if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
            }
            else {                                 // non-empty square
                if (b.color[pos] != side) {          // opponent's piece
                    mob++;
                    if (e.sqNearK[!side][b.king_loc[!side]][pos]) ++att;
                }
                break;                               // own piece
            }
            if (dir > num_slide_moves[ARCHBISHOP])
                break;
        }
    }

    v.mgMob[side] += 1 * (mob - 14);
    v.egMob[side] += 2 * (mob - 14);

    if (att) {
        v.attCnt[side]++;
        v.attWeight[side] += 4 * att;
    }

    int tropism = getTropism(sq, b.king_loc[!side]);
    v.mgTropism[side] += 2 * tropism;
    v.egTropism[side] += 3 * tropism;
}

int wKingShield() {

    int result = 0;

    /* king on the kingside */
    if ( COL(b.king_loc[WHITE]) > COL_F ) {

        if ( isPiece(WHITE, PAWN, H2) )  result += e.SHIELD_2;
        else if ( isPiece(WHITE, PAWN, H3) )  result += e.SHIELD_3;

        if ( isPiece(WHITE, PAWN, I2) )  result += e.SHIELD_2;
        else if ( isPiece(WHITE, PAWN, I3) )  result += e.SHIELD_3;

        if ( isPiece(WHITE, PAWN, J2) )  result += e.SHIELD_2;
        else if ( isPiece(WHITE, PAWN, J3) )  result += e.SHIELD_3;
    }

    /* king on the queenside */
    else if ( COL(b.king_loc[WHITE]) < COL_E ) {

        if ( isPiece(WHITE, PAWN, A2) )  result += e.SHIELD_2;
        else if ( isPiece(WHITE, PAWN, A3) )  result += e.SHIELD_3;

        if ( isPiece(WHITE, PAWN, B2) )  result += e.SHIELD_2;
        else if ( isPiece(WHITE, PAWN, B3) )  result += e.SHIELD_3;

        if ( isPiece(WHITE, PAWN, C2) )  result += e.SHIELD_2;
        else if ( isPiece(WHITE, PAWN, C3) )  result += e.SHIELD_3;
    }

    return result;
}

int bKingShield() {
    int result = 0;

    /* king on the kingside */
    if ( COL(b.king_loc[BLACK]) > COL_F ) {
        if ( isPiece(BLACK, PAWN, H7) )  result += e.SHIELD_2;
        else if ( isPiece(BLACK, PAWN, H6) )  result += e.SHIELD_3;

        if ( isPiece(BLACK, PAWN, I7) )  result += e.SHIELD_2;
        else if ( isPiece(BLACK, PAWN, I6) )  result += e.SHIELD_3;

        if ( isPiece(BLACK, PAWN, J7) )  result += e.SHIELD_2;
        else if ( isPiece(BLACK, PAWN, J6) )  result += e.SHIELD_3;
    }

    /* king on the queenside */
    else if ( COL(b.king_loc[BLACK]) < COL_E ) {
        if ( isPiece(BLACK, PAWN, A7) )  result += e.SHIELD_2;
        else if ( isPiece(BLACK, PAWN, A6) )  result += e.SHIELD_3;

        if ( isPiece(BLACK, PAWN, B7) )  result += e.SHIELD_2;
        else if ( isPiece(BLACK, PAWN, B6) )  result += e.SHIELD_3;

        if ( isPiece(BLACK, PAWN, C7) )  result += e.SHIELD_2;
        else if ( isPiece(BLACK, PAWN, C6) )  result += e.SHIELD_3;
    }
    return result;
}

/******************************************************************************
*                            Pawn structure evaluaton                         *
******************************************************************************/

int getPawnScore() {
    int result;

    /**************************************************************************
    *  This function wraps hashing mechanism around evalPawnStructure().      *
    *  Please note  that since we use the pawn hashtable, evalPawnStructure() *
    *  must not take into account the piece position.  In a more elaborate    *
    *  program, pawn hashtable would contain only the characteristics of pawn *
    *  structure,  and scoring them in conjunction with the piece position    *
    *  would have been done elsewhere.                                        *
    **************************************************************************/

    int probeval = ttpawn_probe();
    if (probeval != INVALID)
        return probeval;

    result = evalPawnStructure();
    ttpawn_save(result);
    return result;
}

int evalPawnStructure() {
    int result = 0;

    for (U8 row = 1; row < NUM_RANKS - 1; row++)
        for (U8 col = 0; col < NUM_FILES; col++) {

            U8 sq = SET_SQ(row, col);

            if (b.pieces[sq] == PAWN) {
                if (b.color[sq] == WHITE) result += EvalPawn(sq, WHITE);
                else                      result -= EvalPawn(sq, BLACK);
            }
        }

    return result;
}

int EvalPawn(U8 sq, S8 side) {
    int result = 0;
    int flagIsPassed = 1; // we will be trying to disprove that
    int flagIsWeak = 1;   // we will be trying to disprove that
    int flagIsOpposed = 0;
	
    /**************************************************************************
    *   We have only very basic data structures that do not update informa-   *
    *   tion about pawns incrementally, so we have to calculate everything    *
    *   here.  The loop below detects doubled pawns, passed pawns and sets    *
    *   a flag on finding that our pawn is opposed by enemy pawn.             *
    **************************************************************************/

	if (b.pawn_ctrl[!side][sq]) // if a pawn is attacked by a pawn, it is not
		flagIsPassed = 0;       // passed (not sure if it's the best decision)

	U8 nextSq = sq + stepFwd[side];

    while (IS_SQ(nextSq)) {

        if (b.pieces[nextSq] == PAWN) { // either opposed by enemy pawn or doubled
            flagIsPassed = 0;
            if (b.color[nextSq] == side)
                result -= 20;       // doubled pawn penalty
            else
                flagIsOpposed = 1;  // flag our pawn as opposed
        }

		if (b.pawn_ctrl[!side][nextSq])
			flagIsPassed = 0;

        nextSq += stepFwd[side];
    }

    /**************************************************************************
    *   Another loop, going backwards and checking whether pawn has support.  *
    *   Here we can at least break out of it for speed optimization.          *
    **************************************************************************/

    nextSq = sq+stepFwd[side]; // so that a pawn in a duo will not be considered weak

    while (IS_SQ(nextSq)) {

		if (b.pawn_ctrl[side][nextSq]) {
			flagIsWeak = 0;
			break;
		}

        nextSq += stepBck[side];
    }

    /**************************************************************************
    *  Evaluate passed pawns, scoring them higher if they are protected       *
    *  or if their advance is supported by friendly pawns                     *
    **************************************************************************/

    if ( flagIsPassed ) {
        if ( isPawnSupported(sq, side) ) result += e.protected_passer[side][sq];
        else							 result += e.passed_pawn[side][sq];
    }

    /**************************************************************************
    *  Evaluate weak pawns, increasing the penalty if they are situated       *
    *  on a half-open file                                                    *
    **************************************************************************/

    if ( flagIsWeak ) {
        result += e.weak_pawn[side][sq];
        if (!flagIsOpposed)
            result -= 4;
    }

    return result;
}

int isPawnSupported(U8 sq, S8 side) {
    int step;
    if (side == WHITE) step = SOUTH;
    else               step = NORTH;

    if ( IS_SQ(sq+WEST) && isPiece(side,PAWN, sq + WEST) ) return 1;
    if ( IS_SQ(sq+EAST) && isPiece(side,PAWN, sq + EAST) ) return 1;
    if ( IS_SQ(sq+step+WEST) && isPiece(side,PAWN, sq + step+WEST ) ) return 1;
    if ( IS_SQ(sq+step+EAST) && isPiece(side,PAWN, sq + step+EAST ) ) return 1;

    return 0;
}

/******************************************************************************
*                             Pattern detection                               *
******************************************************************************/

void blockedPieces(int side) {

	int oppo = !side;

    // central pawn blocked, bishop hard to develop
    if (isPiece(side, BISHOP, REL_SQ(side,C1)) 
	&& isPiece(side, PAWN, REL_SQ(side,D2)) 
	&& b.color[REL_SQ(side,D3)] != COLOR_EMPTY)
       v.blockages[side] -= e.P_BLOCK_CENTRAL_PAWN;

	if (isPiece(side, BISHOP, REL_SQ(side,H1)) 
	&& isPiece(side, PAWN, REL_SQ(side,G2)) 
	&& b.color[REL_SQ(side,G3)] != COLOR_EMPTY)
	   v.blockages[side] -= e.P_BLOCK_CENTRAL_PAWN;

	// trapped knight
	 if (isPiece(side, KNIGHT, REL_SQ(side,A8) ) 
	 && (isPiece(oppo, PAWN, REL_SQ(side,A7) ) || isPiece(oppo, PAWN, REL_SQ(side,C7)))) 
	 v.blockages[side] -= e.P_KNIGHT_TRAPPED_A8;

	 if (isPiece(side, KNIGHT, REL_SQ(side,J8))
	 && (isPiece(oppo, PAWN, REL_SQ(side,J7)) || isPiece(oppo, PAWN, REL_SQ(side,H7)))) 
	     v.blockages[side] -= e.P_KNIGHT_TRAPPED_A8;
 
	 if (isPiece(side, KNIGHT, REL_SQ(side, A7))
	 &&  isPiece(oppo, PAWN, REL_SQ(side,A6)) 
	 &&  isPiece(oppo, PAWN, REL_SQ(side,B7))) 
	     v.blockages[side] -= e.P_KNIGHT_TRAPPED_A7;

	 if (isPiece(side, KNIGHT, REL_SQ(side, J7))
	 && isPiece (oppo, PAWN, REL_SQ(side, J6))
	 && isPiece (oppo, PAWN, REL_SQ(side, I7))) 
	    v.blockages[side] -= e.P_KNIGHT_TRAPPED_A7;

	 // knight blocking queenside pawns
//	 if (isPiece(side, KNIGHT, REL_SQ(side, C3))
//	 && isPiece(side, PAWN, REL_SQ(side, C2))
//	 && isPiece(side, PAWN, REL_SQ(side, D4))
//	 && !isPiece(side, PAWN, REL_SQ(side, E4)) ) 
//	    v.blockages[side] -= e.P_C3_KNIGHT;

	 // trapped bishop
	 if (isPiece(side, BISHOP, REL_SQ(side,A7)) 
	 &&  isPiece(oppo, PAWN,   REL_SQ(side,B6))) 
	     v.blockages[side] -= e.P_BISHOP_TRAPPED_A7;

	 if (isPiece(side, BISHOP, REL_SQ(side, J7))
	 && isPiece(oppo, PAWN, REL_SQ(side, I6))) 
	    v.blockages[side] -= e.P_BISHOP_TRAPPED_A7;

	 if (isPiece(side, BISHOP, REL_SQ(side, B8))
	 && isPiece(oppo, PAWN, REL_SQ(side, C7))) 
	    v.blockages[side] -= e.P_BISHOP_TRAPPED_A7;

	 if (isPiece(side, BISHOP, REL_SQ(side, I8))
	 && isPiece(oppo, PAWN, REL_SQ(side, H7))) 
	    v.blockages[side] -= e.P_BISHOP_TRAPPED_A7;

	 if (isPiece(side, BISHOP, REL_SQ(side, A6))
	 && isPiece(oppo, PAWN, REL_SQ(side, B5))) 
	    v.blockages[side] -= e.P_BISHOP_TRAPPED_A6;

	 if (isPiece(side, BISHOP, REL_SQ(side, J6))
	 && isPiece(oppo, PAWN, REL_SQ(side, I5))) 
	    v.blockages[side] -= e.P_BISHOP_TRAPPED_A6;

	 // bishop on initial sqare supporting castled king
	 if (isPiece(side, BISHOP, REL_SQ(side, F1))
	 && isPiece(side, KING, REL_SQ(side, G1))) 
	    v.positionalThemes[side] += e.RETURNING_BISHOP;

	 if (isPiece(side, BISHOP, REL_SQ(side, C1))
	 && isPiece(side, KING, REL_SQ(side, B1))) 
	    v.positionalThemes[side] += e.RETURNING_BISHOP;

    // uncastled king blocking own rook
//  if ( ( isPiece(side, KING, REL_SQ(side,F1)) || isPiece(side, KING, REL_SQ(side,G1) ) )
//	&&   ( isPiece(side, ROOK, REL_SQ(side,H1)) || isPiece(side, ROOK, REL_SQ(side,G1) ) ) )
//      v.blockages[side] -= e.P_KING_BLOCKS_ROOK;

//	if ((isPiece(side, KING, REL_SQ(side,C1)) || isPiece(side, KING, REL_SQ(side,B1)))
//	&&  (isPiece(side, ROOK, REL_SQ(side,A1)) || isPiece(side, ROOK, REL_SQ(side,B1))) )
//		v.blockages[side] -= e.P_KING_BLOCKS_ROOK;
}

int isPiece(U8 color, U8 piece, U8 sq) {
    return ( (b.pieces[sq] == piece) && (b.color[sq] == color) );
}

/******************************************************************************
*                             Printing eval results                           *
******************************************************************************/

void printEval() {
    printf("------------------------------------------\n");
    printf("Total value (for side to move): %d \n", eval(-INF,INF, 0) );
    printf("Material balance       : %d \n", b.piece_material[WHITE] + b.pawn_material[WHITE] - b.piece_material[BLACK] - b.pawn_material[BLACK] );
    printf("Material adjustement   : ");
	printEvalFactor(v.adjustMaterial[WHITE], v.adjustMaterial[BLACK]);
    printf("Mg Piece/square tables : ");
    printEvalFactor(b.pcsq_mg[WHITE], b.pcsq_mg[BLACK]);
    printf("Eg Piece/square tables : ");
    printEvalFactor(b.pcsq_eg[WHITE], b.pcsq_eg[BLACK]);
    printf("Mg Mobility            : ");
    printEvalFactor(v.mgMob[WHITE], v.mgMob[BLACK]);
    printf("Eg Mobility            : ");
    printEvalFactor(v.egMob[WHITE], v.egMob[BLACK]);
    printf("Mg Tropism             : ");
    printEvalFactor(v.mgTropism[WHITE], v.mgTropism[BLACK]);
    printf("Eg Tropism             : ");
    printEvalFactor(v.egTropism[WHITE], v.egTropism[BLACK]);
    printf("Pawn structure         : %d \n", evalPawnStructure() );
    printf("Blockages              : ");
    printEvalFactor(v.blockages[WHITE], v.blockages[BLACK]);
    printf("Positional themes      : ");
    printEvalFactor(v.positionalThemes[WHITE], v.positionalThemes[BLACK]);
    printf("King Shield            : ");
    printEvalFactor(v.kingShield[WHITE], v.kingShield[BLACK]);
    printf("Tempo                  : ");
    if ( b.stm == WHITE ) printf("%d", e.TEMPO);
    else printf("%d", -e.TEMPO);
    printf("\n");
    printf("------------------------------------------\n");
}

void printEvalFactor(int wh, int bl) {
    printf("white %4d, black %4d, total: %4d \n", wh, bl, wh - bl);
}

int getTropism(int sq1, int sq2) {

	return 7 - (abs(ROW(sq1) - ROW(sq2)) + abs(COL(sq1) - COL(sq2)));
}
