/* Dynamic Segment Data Bank config
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 05-01-04
 * Desc.  : see doc/dsdb/index.html for desc
 */

/* enums & defines
 */
enum
{
  DSEEK_SET,
  DSEEK_CUR,
  DSEEK_END
};

/* default segment size
 * 1024 is a good value for small data (like HTTP requests)
 * use larger segment size if you will use larger data more offten
 */
#define DSDB_DEFAULT_SEG_SIZE 65536

/* uncomment this if you wan't to be extra careful..
 * but i don't think you're ever gonna need this
 */
/* #define EXTRA_SAFE */ 

