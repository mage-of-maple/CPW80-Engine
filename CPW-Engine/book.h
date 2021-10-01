
/*  CPW-80 by Greg Strong
*
*   Derived from CPW by Pawel Koziol and Edmund Moshammer and
*   other members of the Chess Programming Wiki.
*/


#ifdef NO_BOOK_SUPPORT_AT_THIS_TIME 


void readBookFile();
void book_addline( int line_no, const char * movestring);
int book_add( char * movestring);
int book_present( char * movestring );
int book_getMaxFreq();
void book_loadInternal();


#endif
