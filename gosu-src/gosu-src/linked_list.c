/* Linked List Lib
 * Ver.   : 0.0.3
 * Author : gynvael.coldwind//vx
 * Date   : 08.12.2003
 * Desc.  : just a simple linked list
 */
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"memory.h"
 
/* list types
 */
typedef void lnklist;

struct def_lstitem
{
  void *data; /* pointer to data */
  struct def_lstitem *next; /* pointer to next item */
  struct def_lstitem *prev; /* pointer to prev item */
};

typedef struct def_lstdata 
{
  int count; /* number of elements in the list */
  struct def_lstitem *first; /* pointer to first item */
  struct def_lstitem *last; /* pointer to last item */
  struct def_lstitem *current; /* pointer to current item */
} lstdata;

/* functions 
 */
 
/* create */
lnklist* 
lnklist_create( void )
{
  lstdata *new_list;
  
  /* allocate memory */
  new_list = (lstdata*)malloc( sizeof( lstdata ) );
  if( !new_list )
  {
    #ifdef DEBUG
    puts( "lnklst: could not create!" );
    #endif
    return NULL;
  }
  
  /* clear */
  memset( new_list, 0, sizeof( lstdata ) );  
  return new_list;
}

/* destroy */
void 
lnklist_destroy( lnklist* lst )
{
  /* does the list exist ? */
  if( !lst ) return;

  /* is there something to clean ? */
  if( !((lstdata*)lst)->count ) goto destroy_list;
  
  /* clean the list */    
  ((lstdata*)lst)->current = ((lstdata*)lst)->first;
  do
  {
    ((lstdata*)lst)->current = ((lstdata*)lst)->current->next;
    free( ((lstdata*)lst)->first );
    ((lstdata*)lst)->first = ((lstdata*)lst)->current;
  }
  while( ((lstdata*)lst)->current );
  
  /* destroy the list */
destroy_list:
  free( lst );
}

/* first */
int 
lnklist_first( lnklist *lst )
{
  /* is the list ok and has something in it ? */
  if( !lst ) return 2;
  if( !((lstdata*)lst)->count ) return 1;
  
  ((lstdata*)lst)->current = ((lstdata*)lst)->first;
  
  return 0;
}

/* last */
int 
lnklist_last( lnklist *lst )
{
  /* is the list ok and has something in it ? */
  if( !lst ) return 2;
  if( !((lstdata*)lst)->count ) return 1;
  
  ((lstdata*)lst)->current = ((lstdata*)lst)->last;
  
  return 0;
}

/* next */
int 
lnklist_next( lnklist *lst )
{
  /* is the list ok and has something in it ? */
  if( !lst ) return 2;
  if( !((lstdata*)lst)->count ) return 1;
  if( !((lstdata*)lst)->current->next ) return 3;
  
  ((lstdata*)lst)->current = ((lstdata*)lst)->current->next;
  return 0;  
}

/* prev */
int 
lnklist_prev( lnklist *lst )
{
  /* is the list ok and has something in it ? */
  if( !lst ) return 2;
  if( !((lstdata*)lst)->count ) return 1;
  if( !((lstdata*)lst)->current->prev ) return 3;
  
  ((lstdata*)lst)->current = ((lstdata*)lst)->current->prev;
  return 0;  
}

/* count */
int 
lnklist_count( lnklist *lst )
{
  /* is the list ok ? */
  if( !lst ) return -1;
  
  return ((lstdata*)lst)->count;  
}

/* add */
int 
lnklist_add( lnklist *lst, void *data_pointer )
{
  struct def_lstitem *new_item;
  
  /* is the list ok ? */
  if( !lst ) return 1;
  
  /* alloc mem for the new item */
  new_item = (struct def_lstitem*) malloc( sizeof(struct def_lstitem) );
  if( !new_item ) return 2;
  
  /* write the info into new item */
  new_item->data = data_pointer;
  new_item->next = NULL;
  new_item->prev = ((lstdata*)lst)->last;
  
  /* check if we had already any element in the list or not */
  if( ((lstdata*)lst)->count )
  {
    /* set the new element as last, and correct the prev in last pointer */
    ((lstdata*)lst)->last->next = new_item;    
    ((lstdata*)lst)->last = new_item;
  }
  else
  {
    /* set the new element as first, last and current */
    ((lstdata*)lst)->first = 
    ((lstdata*)lst)->current = 
    ((lstdata*)lst)->last = new_item;
  }

  /* increase the count */  
  ((lstdata*)lst)->count++;
 
  return 0;  
}

/* del */
int 
lnklist_del( lnklist *lst )
{
  /* is the list ok and do we have current element set ? */
  if( !lst ) return -1;
  if( !((lstdata*)lst)->current ) return -1;
  
  /* see if we have next element */
  if( ((lstdata*)lst)->current->next )
  {
    /* set the next's element's prev to current prev */
    ((lstdata*)lst)->current->next->prev = ((lstdata*)lst)->current->prev;
  }
  else
  {
    /* set the prev element as last */
    ((lstdata*)lst)->last = ((lstdata*)lst)->current->prev;
  }
  
  /* see if we have prev element */
  if( ((lstdata*)lst)->current->prev )
  {
    /* set the prev's element's next to current next */
    ((lstdata*)lst)->current->prev->next = ((lstdata*)lst)->current->next;
  }
  else
  {
    /* set the next element as first */
    ((lstdata*)lst)->first = ((lstdata*)lst)->current->next;
  }
  
  /* destroy this and set the current element to first */
  free( ((lstdata*)lst)->current );
  ((lstdata*)lst)->current = ((lstdata*)lst)->first;  
  
  return --((lstdata*)lst)->count;
}

/* data */
void* 
lnklist_data( lnklist *lst )
{
  /* is the list ok and is the current element set ? */
  if( !lst ) return NULL;
  if( !((lstdata*)lst)->current ) return NULL;
  
  return ((lstdata*)lst)->current->data;
}

void 
lnklist_clean( lnklist *lst )
{
  /* does the list exist and is there something to clean ? */
  if( !lst ) return;
  if( !((lstdata*)lst)->count ) return;

  /* clean the list */
  ((lstdata*)lst)->current = ((lstdata*)lst)->first;
  do
  {
    ((lstdata*)lst)->current = ((lstdata*)lst)->current->next;
    free( ((lstdata*)lst)->first );
    ((lstdata*)lst)->first = ((lstdata*)lst)->current;
  }
  while( ((lstdata*)lst)->current );
  
  /* clean the list info */
  memset( (lstdata*)lst, 0, sizeof( lstdata ) );
}

