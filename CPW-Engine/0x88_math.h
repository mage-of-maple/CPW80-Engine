
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/


/* column identifiers */

#define COL_A   ( A1 >> 4 )
#define COL_B   ( B1 >> 4 )
#define COL_C   ( C1 >> 4 )
#define COL_D   ( D1 >> 4 )
#define COL_E   ( E1 >> 4 )
#define COL_F   ( F1 >> 4 )
#define COL_G   ( G1 >> 4 )
#define COL_H   ( H1 >> 4 )
#define COL_I   ( I1 >> 4 )
#define COL_J   ( J1 >> 4 )

/* row identifiers */

#define ROW_1  ( A1 & 7 )
#define ROW_2  ( A2 & 7 )
#define ROW_3  ( A3 & 7 )
#define ROW_4  ( A4 & 7 )
#define ROW_5  ( A5 & 7 )
#define ROW_6  ( A6 & 7 )
#define ROW_7  ( A7 & 7 )
#define ROW_8  ( A8 & 7 )

/* vectors */

#define NORTH   1
#define SOUTH  -1
#define EAST   16
#define WEST  -16
#define NE     17
#define SW    -17
#define NW    -15
#define SE     15

/* generate square number from row and column */
#define SET_SQ(row,col) (col * 16 + row)

/* does a given number represent a square on the board? */
#define IS_SQ(x)  ( (U8) ((((x) & 0x08) * 20) | (x)) >= 160 ) ? (0) : (1)

/* get board row that a square is part of */
#define ROW(sq)  ( (sq) & 7 )

/* get board column that a square is part of */
#define COL(sq)  ( (sq) >> 4 )

/* determine if two squares lie on the same column */
#define SAME_COL(sq1,sq2) ( ( COL(sq1) == COL(sq2) ) ? (1) : (0) )

/* determine if two squares lie in the same row */
#define SAME_ROW(sq1,sq2) ( ( ROW(sq1) == ROW(sq2) ) ? (1) : (0) )
