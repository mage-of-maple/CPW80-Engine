
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#pragma once

struct svariant
{
	const char* startFEN;
	char archbishop;
	char chancellor;
	U8 kingStartSq[NUM_PLAYERS];
	U8 rookStartSq[NUM_PLAYERS][2];
	char castlingPrivChars[NUM_PLAYERS][2];
	int minCastlingStepsEast;
	int maxCastlingStepsEast;
	int minCastlingStepsWest;
	int maxCastlingStepsWest;
	U8 castlingPrivsSquareMask[NUM_SQUARES];
	int num_promote_types;
	char promote_types[6] = { QUEEN, CHANCELLOR, ARCHBISHOP, ROOK, BISHOP, KNIGHT };
	const char* variants = "capablanca,gothic,moderncarrera,bird,carrera,embassy,schoolbook,grotesque,ladorean,univers,opti,victorian,garamond,baskerville,helvetica,janus,newchancellor";
	const char* variant_display[3] = {
		"    capablanca, bird, carrera, gothic, moderncarrera, embassy, \n",
		"    schoolbook, grotesque, ladorean, univers, opti, victorian, \n",
		"    baskerville, garamond, helvetica, janus, newchancellor, hannibal \n" };

	void FinalizeInit()
	{
		// set the values in the castlingPrivsSquareMask.
		// every move the available castling privs will be &'ed 
		// with the value in both the from and to squares of 
		// this array.
		for (int sq = 0; sq < NUM_SQUARES; sq++) {
			castlingPrivsSquareMask[sq] = 255;
		}
		castlingPrivsSquareMask[kingStartSq[0]] = ~CASTLE_WK & ~CASTLE_WQ;
		castlingPrivsSquareMask[rookStartSq[0][0]] = ~CASTLE_WQ;
		castlingPrivsSquareMask[rookStartSq[0][1]] = ~CASTLE_WK;
		castlingPrivsSquareMask[kingStartSq[1]] = ~CASTLE_BK & ~CASTLE_BQ;
		castlingPrivsSquareMask[rookStartSq[1][0]] = ~CASTLE_BQ;
		castlingPrivsSquareMask[rookStartSq[1][1]] = ~CASTLE_BK;
	}

	void StandardCastling(bool mirror)
	{
		// does the king start on F1 or E1?  we support both.
		// Janus and Embassy start with the king on E1.
		if (!mirror) {
			castlingPrivChars[0][0] = 'Q';
			castlingPrivChars[0][1] = 'K';
			castlingPrivChars[1][0] = 'q';
			castlingPrivChars[1][1] = 'k';
			kingStartSq[0] = F1;
			kingStartSq[1] = F8;
		} else {
			castlingPrivChars[0][0] = 'A';
			castlingPrivChars[0][1] = 'J';
			castlingPrivChars[1][0] = 'a';
			castlingPrivChars[1][1] = 'j';
			kingStartSq[0] = E1;
			kingStartSq[1] = E8;
		}
		rookStartSq[0][0] = A1;
		rookStartSq[0][1] = J1;
		rookStartSq[1][0] = A8;
		rookStartSq[1][1] = J8;
		minCastlingStepsWest = 3;
		maxCastlingStepsWest = 3;
		minCastlingStepsEast = 3;
		maxCastlingStepsEast = 3;
	}

	void FlexibleCastling()
	{
		bool mirror = startFEN[4] == 'k';
		StandardCastling(mirror);
		minCastlingStepsWest = 2;
		minCastlingStepsEast = 2;
		maxCastlingStepsWest = mirror ? 3 : 4;
		maxCastlingStepsEast = mirror ? 4 : 3;
	}

	void LongCastling()
	{
		bool mirror = startFEN[4] == 'k';
		StandardCastling(mirror);
		minCastlingStepsWest = mirror ? 3 : 4;
		minCastlingStepsEast = mirror ? 4 : 3;
		maxCastlingStepsWest = mirror ? 3 : 4;
		maxCastlingStepsEast = mirror ? 4 : 3;
	}

	void CloseRookCastling()
	{
		bool mirror = startFEN[4] == 'k';
		StandardCastling(mirror);
		rookStartSq[0][0] = B1;
		rookStartSq[0][1] = I1;
		rookStartSq[1][0] = B8;
		rookStartSq[1][1] = I8;
		minCastlingStepsWest = 2;
		minCastlingStepsEast = 2;
		maxCastlingStepsWest = 2;
		maxCastlingStepsEast = 2;
		castlingPrivChars[0][0] = 'B';
		castlingPrivChars[0][1] = 'I';
		castlingPrivChars[1][0] = 'b';
		castlingPrivChars[1][1] = 'i';
	}

	void VariantBasic(const char* fen)
	{
		startFEN = fen;
		archbishop = 'a';
		chancellor = 'c';
		StandardCastling(fen[4] == 'k');
		num_promote_types = 6;
	}

	void VariantCapablanca()
	{
		VariantBasic("rnabqkbcnr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNABQKBCNR w KQkq - 0 1");
	}

	void VariantGothic()
	{
		VariantBasic("rnbqckabnr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBQCKABNR w KQkq - 0 1");
	}

	void VariantModernCarrera()
	{
		VariantBasic("ranbqkbncr/pppppppppp/10/10/10/10/PPPPPPPPPP/RANBQKBNCR w KQkq - 0 1");
		printf("setup (PNBRQ..ACKpnbrq..aCk) 10x8+0_capablanca ranbqkbncr/pppppppppp/10/10/10/10/PPPPPPPPPP/RANBQKBNCR w KQkq - 0 1\n");
	}

	void VariantBird()
	{
		VariantBasic("rnbgqkebnr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBGQKEBNR w KQkq - 0 1");
		printf("setup (PNBRQ..E=AG=CKpnbrq..e=ag=ck) 10x8+0_capablanca rnbgqkebnr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBGQKEBNR w KQkq - 0 1\n");
		// set the Archbishop to 'e' for Equerry
		archbishop = 'e';
		// set the Chancellor to 'g' for Guard
		chancellor = 'g';
	}

	void VariantCarrera()
	{
		VariantBasic("rcnbkqbnar/pppppppppp/10/10/10/10/PPPPPPPPPP/RCNBKQBNAR w - - 0 1");
		printf("setup (PNBRQ..ACKpnbrq..ack) 10x8+0_capablanca rcnbkqbnar/pppppppppp/10/10/10/10/PPPPPPPPPP/RCNBKQBNAR w - - 0 1\n");
		// We retain the notations 'c' and 'a' because the names for these pieces, 
		// Champion and Centaur, both begin with 'c'.  Since 'c' is typically the Chancellor, 
		// the 'c' will remain assigned to Champion and 'a' will represent the Centaur.
	}

	void VariantEmbassy()
	{
		VariantBasic("rnbqkmcbnr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBQKMCBNR w AJaj - 0 1");
		printf("setup (PNBRQ..CMKpnbrq..cmk) 10x8+0_fairy rnbqkmcbnr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBQKMCBNR w AJaj - 0 1\n");
		printf("piece K& KisO3\n");
		// set the Archbishop to 'c' for Cardinal
		archbishop = 'c';
		// set the Chancellor to 'm' for Marshall
		chancellor = 'm';
	}

	void VariantSchoolbook()
	{
		VariantBasic("rqnbakbncr/pppppppppp/10/10/10/10/PPPPPPPPPP/RQNBAKBNCR w KQkq - 0 1");
		printf("setup (PNBRQ..AC=MKpnbrq..ac=mk) 10x8+0_fairy rqnbakbnmr/pppppppppp/10/10/10/10/PPPPPPPPPP/RQNBAKBNMR w KQkq - 0 1\n");
		printf("piece K KisO2isO3ilO4\n");
		printf("piece k KisO2isO3irO4\n");
		FlexibleCastling();
	}

	void VariantGrotesque()
	{
		VariantBasic("rbqnkgnebr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBQNKGNEBR w AJaj - 0 1");
		printf("setup (PNBRQ..E=AG=CKpnbrq..e=ag=ck) 10x8+0_fairy rbqnkgnebr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBQNKGNEBR w AJaj - 0 1\n");
		printf("piece K KisO2isO3irO4\n");
		printf("piece k KisO2isO3ilO4\n");
		// set the Archbishop to 'e' for Equerry
		archbishop = 'e';
		// set the Chancellor to 'g' for Guard
		chancellor = 'g';
		FlexibleCastling();
	}

	void VariantLadorean()
	{
		VariantBasic("rbqnkcnmbr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBQNKCNMBR w AJaj - 0 1");
		printf("setup (PNBRQ..CMKpnbrq..cmk) 10x8+0_fairy rbqnkcnmbr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBQNKCNMBR w AJaj - 0 1\n");
		printf("piece K KisO2isO3irO4\n");
		printf("piece k KisO2isO3ilO4\n");
		// set the Archbishop to 'c' for Cardinal
		archbishop = 'c';
		// set the Chancellor to 'm' for Marshall
		chancellor = 'm';
		FlexibleCastling();
	}

	void VariantUnivers()
	{
		VariantBasic("rbnmqkanbr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBNMQKANBR w KQkq - 0 1");
		printf("setup (PNBRQ..AM=CKpnbrq..am=ck) 10x8+0_fairy rbnmqkanbr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBNMQKANBR w KQkq - 0 1\n");
		printf("piece K KisO2isO3ilO4\n");
		printf("piece k KisO2isO3irO4\n");
		// set the Chancellor to 'm' for Marshall
		chancellor = 'm';
		// the Archbishop will retain notaion 'a' since the name in this variant 
		// is Paladin and the letter 'p' is already used for Pawn.
		FlexibleCastling();
	}

	void VariantOpti()
	{
		VariantBasic("nrcbqkbarn/pppppppppp/10/10/10/10/PPPPPPPPPP/NRCBQKBARN w BIbi - 0 1");
		printf("setup (PNBRQ..ACKpnbrq..ack) 10x8+0_fairy nrcbqkbarn/pppppppppp/10/10/10/10/PPPPPPPPPP/NRCBQKBARN w BIbi - 0 1\n");
		printf("piece K& KisjO2\n");
		CloseRookCastling();
	}

	void VariantVictorian()
	{
		VariantBasic("crnbakbnrq/pppppppppp/10/10/10/10/PPPPPPPPPP/CRNBAKBNRQ w BIbi - 0 1");
		printf("setup (PNBRQ..ACKpnbrq..ack) 10x8+0_fairy crnbakbnrq/pppppppppp/10/10/10/10/PPPPPPPPPP/CRNBAKBNRQ w BIbi - 0 1\n");
		printf("piece K& KisjO2\n");
		printf("choice QCA\n");
		CloseRookCastling();
		num_promote_types = 3;
	}

	void VariantJanus()
	{
		VariantBasic("rjnbkqbnjr/pppppppppp/10/10/10/10/PPPPPPPPPP/RJNBKQBNJR w AJaj - 0 1");
		LongCastling();
		num_promote_types = 5;
		// set the Archbishop to 'j' for Janus
		archbishop = 'j';
		// remove Chancellor from the promotion lineup
		for (int x = 1; x < 5; x++)
			promote_types[x] = promote_types[x + 1];
	}

	void VariantNewChancellor()
	{
		VariantBasic("crnbqkbnrc/pppppppppp/10/10/10/10/PPPPPPPPPP/CRNBQKBNRC w BIbi - 0 1");
		printf("setup (PNBRQ...CKpnbrq...ck) 10x8+0_fairy crnbqkbnrc/pppppppppp/10/10/10/10/PPPPPPPPPP/CRNBQKBNRC w BIbi - 0 1\n");
		printf("piece K& KisjO2\n");
		CloseRookCastling();
		num_promote_types = 5;
		// remove Archbishop from the promotion lineup
		for (int x = 2; x < 5; x++)
			promote_types[x] = promote_types[x + 1];
	}

	void VariantGaramond()
	{
		VariantBasic( "rbnaqkcnbr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBNAQKCNBR w KQkq - 0 1" );
		printf( "setup (PNBRQ..ACKpnbrq..ack) 10x8+0_fairy rbnaqkcnbr/pppppppppp/10/10/10/10/PPPPPPPPPP/RBNAQKCNBR w KQkq - 0 1\n" );
		printf( "piece K KisO2isO3ilO4\n" );
		printf( "piece k KisO2isO3irO4\n" );
		FlexibleCastling();
	}

	void VariantBaskerville()
	{
		VariantBasic( "rannckbbqr/pppppppppp/10/10/10/10/PPPPPPPPPP/RANNCKBBQR w KQkq - 0 1" );
		printf( "setup (PNBRQ..ACKpnbrq..ack) 10x8+0_fairy rannckbbqr/pppppppppp/10/10/10/10/PPPPPPPPPP/RANNCKBBQR w KQkq - 0 1\n" );
		printf( "piece K KisO2isO3ilO4\n" );
		printf( "piece k KisO2isO3irO4\n" );
		FlexibleCastling();
	}

	void VariantHelvetica()
	{
		VariantBasic( "rnqbckbanr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNQBCKBANR w KQkq - 0 1" );
		printf( "setup (PNBRQ..ACKpnbrq..ack) 10x8+0_fairy rnqbckbanr/pppppppppp/10/10/10/10/PPPPPPPPPP/RNQBCKBANR w KQkq - 0 1\n" );
		printf( "piece K KisO2isO3ilO4\n" );
		printf( "piece k KisO2isO3irO4\n" );
		FlexibleCastling();
	}

	void VariantByName(char* name)
	{
		if (!strcmp(name, "capablanca"))
			VariantCapablanca();
		else if (!strcmp(name, "gothic"))
			VariantGothic();
		else if (!strcmp(name, "moderncarrera"))
			VariantModernCarrera();
		else if (!strcmp(name, "bird"))
			VariantBird();
		else if (!strcmp(name, "carrera"))
			VariantCarrera();
		else if (!strcmp(name, "embassy"))
			VariantEmbassy();
		else if (!strcmp(name, "schoolbook"))
			VariantSchoolbook();
		else if (!strcmp(name, "grotesque"))
			VariantGrotesque();
		else if (!strcmp(name, "ladorean"))
			VariantLadorean();
		else if (!strcmp(name, "univers"))
			VariantUnivers();
		else if (!strcmp(name, "opti"))
			VariantOpti();
		else if (!strcmp(name, "victorian"))
			VariantVictorian();
		else if (!strcmp(name, "janus"))
			VariantJanus();
		else if (!strcmp(name, "newchancellor"))
			VariantNewChancellor();
		else if( !strcmp( name, "garamond" ) )
			VariantGaramond();
		else if( !strcmp( name, "baskerville" ) )
			VariantBaskerville();
		else if( !strcmp( name, "helvetica" ) )
			VariantHelvetica();
	}
};

extern svariant cv;
