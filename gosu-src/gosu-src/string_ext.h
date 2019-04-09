/* String Stuff That Helps Parsing =^^=
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 08.12.2003
 * Desc.  : like the sign says...
 */
#ifndef GYNV_STRINGEXT
#define GYNV_STRINGEXT

/* functions
 */
char* jump_over_blank( char *where );
char* end_of_name( char *where );
void strip_eoln( char *where );
void strip_end_blank( char *where );
char* upper(char *what);
const char* copy_line( char *where, const char *what, int n );
char *end_of_name_sp( char *where );

#endif
