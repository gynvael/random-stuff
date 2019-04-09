/* Dynamic Segment Data Bank
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 05-01-04
 * Desc.  : see doc/dsdb/index.html for desc
 */
#ifndef GYNV_DSDB
#define GYNV_DSDB

/* include config 
 */
#include"dsdb_config.h"

/* types
 */
typedef void dsdb;

/*
 * functions (CREATE/DESTROY) 
 */

/* creates main data bank struct and allocates one default segment 
 * this realy is a just a macro
 * returns: pointer to dsdb
 *        : NULL on error
 */
#define dsdb_create() dsdb_createex( DSDB_DEFAULT_SEG_SIZE, 1 )

/* creates main data bank struct, sets segment size, and allocated segments
 * returns: pointer to dsdb
 *        : NULL on error
 */
dsdb* dsdb_createex( unsigned int segsize, unsigned int segcnt );

/* cleans and destroys bank 
 * returns: void
 */
void dsdb_destroy( dsdb *bank );

/*
 * functions (INPUT/OUTPUT) 
 */

/* reads data from bank and writes it to the specified buffer
 * returns: number of elements read
 */
int dsdb_read( void *data, unsigned int s_size, unsigned int s_cnt, dsdb *bank );

/* writes data to bank
 * returns: number of elements written
 */
int dsdb_write( const void *data, unsigned int s_size, unsigned int s_cnt, dsdb *bank );

/* writes string to bank
 * returns: number of bytes written
 */
int dsdb_puts( dsdb *bank, const char *data );

/*
 * functions (MOVEMENT/SIZE) 
 */

/* moves the bank pointer to specified location
 * returns: 0 on success
 *        : !0 on error
 */
int dsdb_seek( dsdb *bank, int offset, int type );

/* gets the current bank position
 * returns: current bank position
 */
unsigned int dsdb_tell( dsdb *bank );

/* cuts the bank, setting it's end in the current position
 * current byte is also deleted
 * returns: void
 */
void dsdb_cut( dsdb *bank );

/*
 * functions (INFO) 
 */

/* gets size of data stored in specified bank
 * returns: data size
 */
unsigned int dsdb_datasize( dsdb *bank );

/* gets number of segments used by the bank; good for debug & optimalization
 * returns: number of segments
 */
unsigned int dsdb_segcount( dsdb *bank );

/* gets segment size of the specified bank
 * return: segment size
 */
unsigned int dsdb_segsize( dsdb *bank );

/* gets mem usage of the specified bank (not the same as data size!)
 * returns: mem usage 
 */
unsigned int dsdb_memsize( dsdb *bank );

/* gets mem usage of DSDB module
 * returns: mem usage 
 */
unsigned int dsdb_memusage( void );

/*
 * functions (PREALLOC/CLEAN) 
 */

/* allocates default-size segments for later usage (good for optimalization)
 * returns: 0 on success
 *        : 1 on error
 */
int dsdb_pre_alloc( unsigned int seg_cnt );

/* destroys pre-allocated segments (it's good that you destroy dsdb banks earlier)
 * returns: 0 on success
 *        : number of segments "lost" on error
 */
int dsdb_pre_free( void );

#endif

