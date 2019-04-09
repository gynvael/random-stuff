/* Linked List Lib
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 08.12.2003
 * Desc.  : just a simple linked list
 */
#ifndef GYNV_LINKED_LIST
#define GYNV_LINKED_LIST

/* list type 
 */
typedef void lnklist;

/* functions
 */
 
/* creates a linked list 
 * returns: poitner to lnklist
 *        : NULL on error
 */
lnklist* lnklist_create( void ); 

/* destroys a linked list */
void lnklist_destroy( lnklist* lst );

/* sets the list at the begining 
 * returns: 0 - all ok
 *        : !0 - no elements in list
 */
int lnklist_first( lnklist *lst );

/* sets the list at the end 
 * returns: 0 - all ok
 *        : !0 - no elements in list
 */
int lnklist_last( lnklist *lst );

/* moves to the next element 
 * returns: 0 - all ok
 *        : !0 - no next element to move to 
 */
int lnklist_next( lnklist *lst );

/* moves to the previous element 
 * returns: 0 - all ok
 *        : !0 - no previous element to move to 
 */
int lnklist_prev( lnklist *lst );

/* returns the number of elements 
 * returns: no. of elements in list
 *        : -1 on error
 */
int lnklist_count( lnklist *lst );

/* adds an element to the end of the list
 * returns: 0 - all on
 *        : !0 - some wierd error
 */
int lnklist_add( lnklist *lst, void *data_pointer );

/* deletes the current element
 * returns: no. of elements left in list
 */
int lnklist_del( lnklist *lst );

/* returns current pointer to data 
 * returns: pointer
 */
void* lnklist_data( lnklist *lst );

/* cleans the list, but doesn't destroy it */
void lnklist_clean( lnklist *lst );

#endif

