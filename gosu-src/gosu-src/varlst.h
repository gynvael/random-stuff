/* Variable List
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 10.12.2003
 * Desc.  : variable list...
 */
#ifndef GYNV_VARLST
#define GYNV_VARLST

/* types 
 */
typedef void varlst;

/* functions
 */
 
/* creates varlst info struct
 * returns: pointer on success
 *        : NULL on error
 */
varlst* varlst_create( void );

/* destroys the varlst */
void varlst_destroy( varlst *vls );

/* cleans varlst, but does not destroy it */
void varlst_clean( varlst *vls );

/* returns pointer to data 
 * returns: pointer to data
 *        : NULL on error
 */
const char* varlst_getvalue( varlst *vls, const char *itemname );

/* adds item with value, or changes the value of an existing item */
void varlst_setvalue( varlst *vls, const char *itemname, const char *value );

/* adds item with value, or changes the value of an existing item 
 * does not alocate item for data, and does not copy it
 * good for BIG values
 */
void varlst_setvalueex( varlst *vls, const char *itemname, char *data );

#endif

