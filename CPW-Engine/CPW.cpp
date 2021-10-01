
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/

#include "stdafx.h"
#include "transposition.h"
#include "0x88_math.h"
#include "variant.h"

#ifdef __linux__
#define Sleep _sleep
#else
#include <windows.h>
#endif

int com_init();
void setDefaultEval();
int tt_init();
int tt_setsize(int size);

s_options options;
etask task;

void time_uci_go(char * command);

int main() {
    com_init();
    setDefaultEval();
    tt_init();

	// minimum sizes, in case we don't receive the memory command:
    tt_setsize(0x0400000);     //  4m
    ttpawn_setsize(0x0100000); //  1m
    tteval_setsize(0x0100000); //  1m

	// opening book from CPW - not currently supported for variants
//  initBook();

	cv.VariantCapablanca();
    board_loadFromFen(cv.startFEN);

    for(;;) {

        if (task == etask::TASK_NOTHING || task == etask::TASK_FORCE || task == etask::TASK_WAIT || task == etask::TASK_GAMEOVER) {
            com();
			if (task == etask::TASK_WAIT)
				Sleep(10);
        } else {
            search_run();
            task = etask::TASK_WAIT;
        }
    }
}
