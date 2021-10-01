Overview

CPW-80 is an adaptation of the CPW Chess Engine to an 80 square 10-by-8 board. The original CPW is an open-source didactic chess engine developed by members of the Chess Programming Wiki, especially Pawel Koziol and Edmund Moshammer. CPW-80 aims to retain the clarity of code and educational value of CPW as much as possible, and will hopefully by useful to people wanting to study or expirement with 10x8 chess variants.

CPW-80 can play fifteen different Capablanca variants, as well as Janus Chess and New Chancellor Chess. It supports Standard, Long, Flexible, and Close-Rook castling rules. Playing strength is respectable.
Technical Details

CPW has a 0x88 board representation. The biggest question for adapting to an 80 square, 10-by-8 board was the board representation to use. 0x88 is a neat trick that works because the chess board has 8 ranks and 8 files. The 80 square board does still have 8 ranks, though, so it is possible to still gain some of the benefit. I rotated the board representation to Little-Endian File-Rank Mapping, meaning the squares are ordered a1, a2, a3 … rather than a1, b1, c1. Like 0x88, we have a second "phantom board" that is never occupied stored after the real board, but since the representation is rotated, our phantom board is conceptually to the right the main board rather than above it. 
