/* Variable List
 * Ver.   : 0.0.2
 * Author : gynvael.coldwind//vx
 * Date   : 10.12.2003
 * Desc.  : variable list...
 */
#include<string.h>
#include<stdlib.h>
#include"linked_list.h"
#include"memory.h"
int	_strnicmp (const char*, const char*, size_t); 

/* types
 */
typedef void varlst;
typedef struct def_item
{
  char name[ 32 ];
  int type;
  union def_value
  {
    char normal[ 256 ];  
    char *special;
  } value;
} item;

/* functions
 */

/* create */ 
varlst* 
varlst_create( void )
{
  return lnklist_create ( );
}

/* destroy */
void 
varlst_destroy( varlst *vls )
{
  item *itm;
  int i, j;
  
  /* is there a list ? */
  if( !vls ) return;
  
  /* clean the list data */
  j = lnklist_count( vls );
  lnklist_first( vls );
  for( i = 0; i < j; i++ )
  {
    itm = (item*)lnklist_data( vls );
    if( !itm ) continue;
    
    /* is this a special item ? */
    if( itm->type == 1 )
    {
      free( itm->value.special );      
    }
    
    free( itm );
    lnklist_next( vls );
  }
  
  /* free the list */
  lnklist_destroy( vls );
}

/* clean */
void varlst_clean( varlst *vls )
{
  item *itm;
  int i, j;
  
  /* is there a list ? */
  if( !vls ) return;
  
  /* clean the list data */
  j = lnklist_count( vls );
  lnklist_first( vls );
  for( i = 0; i < j; i++ )
  {
    itm = (item*)lnklist_data( vls );
    
    /* is this a special item ? */
    if( itm->type == 1 )
    {
      free( itm->value.special );      
    }
    
    free( itm );
    lnklist_next( vls );
  }
  
  /* clean the list */
  lnklist_clean( vls );
}

/* get */
const char* 
varlst_getvalue( varlst *vls, const char *itemname )
{
  int i, j;
  item *itm;
  
  /* is there a list ? */
  if( !vls ) return NULL;
  
  /* seek the item */
  j = lnklist_count( vls );
  lnklist_first( vls );
  for( i = 0; i < j; i++ )
  {
    itm = (item*)lnklist_data( vls );
    
    /* is this the item we seek ? */
    if( _strnicmp( itm->name, itemname, 31 ) == 0 ) 
    {
      /* is this a special or a normal item ? */
      if( itm->type == 1 )
      {
        return itm->value.special;
      }
      else
      {    
        return itm->value.normal;
      }
    }
    
    /* jump to next */
    lnklist_next( vls );
  }
  
  return NULL;
}

/* add */
void 
varlst_setvalue( varlst *vls, const char *itemname, const char *value )
{
  const char *sth;
  item *itm;
  
  /* is there a list ? */
  if( !vls ) return;

  /* is there an item with such name ? */
  sth = varlst_getvalue( vls, itemname );
  if( sth )
  {
    itm = (item*)lnklist_data( vls );
    
    /* is this a special item ? */
    if( itm->type == 1 )
    {
      /* free the item's current data */
      free( itm->value.special );
    }
  }
  else
  {
    /* alloc mem */
    itm = (item*)malloc( sizeof( item ) );
    if( !itm ) return;
    
    /* copy item name */
    memset( itm, 0, sizeof( item ) );
    strncpy( itm->name, itemname, 31 );
  
    /* add the item to the list */
    lnklist_add( vls, itm );
  }
  
  /* set the item data and exit */
  strncpy( itm->value.normal, value, 255 );
  itm->type = 0;
  
}

/* addex */
void 
varlst_setvalueex( varlst *vls, const char *itemname, char *data )
{
  const char *sth;
  item *itm;
  
  /* is there a list ? */
  if( !vls ) return;

  /* is there an item with such name ? */
  sth = varlst_getvalue( vls, itemname );
  if( sth )
  {
    itm = (item*)lnklist_data( vls );
    
    /* is this a special item ? */
    if( itm->type == 1 )
    {
      /* free the item's current data */
      free( itm->value.special );
    }
  }
  else
  {
    /* alloc mem */
    itm = (item*)malloc( sizeof( item ) );
    if( !itm ) return;
    
    /* copy item name */
    memset( itm, 0, sizeof( item ) );
    strncpy( itm->name, itemname, 31 );
    
    /* add the item to the list */
    lnklist_add( vls, itm );
  }
  
  /* set the item data and exit */
  itm->value.special = data;
  itm->type = 1;
} 

