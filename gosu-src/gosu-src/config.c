/* Config Lib
 * Ver.   : 0.0.2
 * Author : gynvael.coldwind//vx
 * Date   : 08.12.2003
 * Desc.  : simple config parser + stuff
 */
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"linked_list.h"
#include"string_ext.h"
#include"memory.h"
int	_strnicmp (const char*, const char*, size_t);

/* types
 */
typedef void config;

/* config tree:
 * cfginfo
 * |-section1
 * | |-item1
 * | |-item2
 * | \-item3
 * |-section2
 * | \-item1
 * \-default
 */
 
typedef struct def_item
{
  char name[ 32 ];
  char value[ 256 ];
} item;

typedef struct def_section
{
  char name[ 32 ];
  lnklist *items;
} section;

typedef struct def_cfginfo
{
  char file[ 256 ];
  lnklist *sections;
} cfginfo;

/* functions 
*/

/* open */
config*
config_open( const char *filename )
{
  cfginfo *new_cfg;
    
  /* allocate memory */
  new_cfg = (cfginfo*)malloc( sizeof( cfginfo ) );
  if( !new_cfg ) return NULL;
  
  /*
  NOT USED
  def = (section*)malloc( sizeof( section ) );
  if( !def )
  {
    free( new_cfg );
    return NULL;
  }
  */
  
  /* copy config filename */
  strncpy( new_cfg->file, filename, 256 );
  
  /* create section list */
  new_cfg->sections = lnklist_create( );
  
  return new_cfg;  
}

/* close */
void 
config_close( config *cfg )
{
  cfginfo *conf = (cfginfo*)cfg;
  section *sect;
  int i, j;
  int no_sect, no_itms;
  
  /* is there a config ? */
  if( !conf ) return;
  
  /* clean the items */
  no_sect = lnklist_count( conf->sections );
  if( ( no_sect ) && ( no_sect != -1 ) )
  {
    /* enumerate all the sections */
    lnklist_first( conf->sections );
    for( j = 0; j < no_sect; j++ )
    {
      sect = (section*)lnklist_data( conf->sections );
      
      /* are there any items in this section ? */
      no_itms = lnklist_count( sect->items );
      if( ( no_itms ) && ( no_itms != -1 ) )
      {
        /* enumerate all the items in this section */
        lnklist_first( sect->items );
        for( i = 0; i < no_itms; i++ )
        {
          /* free the data */
          free( lnklist_data( sect->items ) );
          
          /* go to the next item */
          lnklist_next( sect->items );
        }
      }
      
      /* free the section */
      lnklist_destroy( sect->items );
      free( sect );
      
      /* go to the next section */
      lnklist_next( conf->sections );
    }
  }
  
  /* free the section list */
  lnklist_destroy( conf->sections );
  
  /* free the conf info */
  free( conf );  
}

/* parse */
int 
config_parse( config *cfg )
{
  cfginfo *conf = (cfginfo*)cfg;
  section *current_section = NULL;
  section *new_section = NULL;
  item *new_item = NULL;
  FILE *parseme = NULL;
  static char line[ 512 ];
  int items = 0;
  char *partemp = NULL, *parname = NULL, *pardata = NULL;
  
  /* is there any config ? */
  if( !cfg ) return -1;
  
  /* open the file */
  parseme = fopen( conf->file, "r" );
  if( !parseme ) return -1;
  
  /* start the parsing */
  
  while( !feof( parseme ) )
  {
  
    /* read the line */
    fgets( line, 512, parseme );
    line[ 511 ] = '\0';
    
    /* strip \n */
    strip_eoln( line );
    
    /* strip blank space at the begining */
    partemp = jump_over_blank( line );
    
    /* check if we don't have a blank line */
    if( !(*partemp) ) continue;
    
    /* strip blank space at the end */
    strip_end_blank( partemp );
    
    /* check what do we have in line */
    if( *partemp == '#' )
    {
      /* it's a comment, don't mind it */
      continue;
    }
    else if( *partemp == '[' )
    {
      /* it's a section! */
      
      /* we have to check it's name */
      if( partemp[ strlen( partemp ) - 1 ] != ']' )
      {
        /* incorrect section, continue */
        continue;
      }
      else
      {
        /* strip [ ] */
        partemp++;
        partemp[ strlen( partemp ) - 1 ] = '\0';
      }
      
      /* create a new section */
      new_section = (section*)malloc( sizeof( section ) );
      memset( new_section, 0, sizeof( section ) );
      strncpy( new_section->name, partemp, 31 );
      new_section->items = lnklist_create( );
      lnklist_add( conf->sections, new_section );
      
      /* set the new section as current */
      current_section = new_section;
    }
    else
    {
      /* it's an item */
      parname = partemp;
      
      /* end the name */
      pardata = end_of_name( partemp );
      if( !(*pardata) )
      {
        /* incorrect item */
        continue;
      }
      
      if( *pardata == '=' )
      {
        *(pardata++) = '\0';
        
        /* jump over space */
        pardata = jump_over_blank( pardata );
      }
      else
      {
        *(pardata++) = '\0';
        
        /* jump over space */
        pardata = jump_over_blank( pardata );
        
        /* is everything ok ? */
        if( (*pardata) != '=' )
        {
          /* incorrect */
          continue;
        }
        
        pardata++;
        pardata = jump_over_blank( pardata );
      }
     
      /* is there any data ? */
      if( !(*pardata) ) 
      {
        /* incorrect */
        continue;
      }
     
      /* is there a section opened ? */
      if( !current_section )
      { 
        /* creates the default section */
        new_section = (section*)malloc( sizeof( section ) );
        memset( new_section, 0, sizeof( section ) );
        strncpy( new_section->name, "default", 31 );
        new_section->items = lnklist_create( );
        lnklist_add( conf->sections, new_section );
      
        /* set the new section as current */
        current_section = new_section;
      }
      
      /* create item entry */
      new_item = (item*)malloc( sizeof( item ) );
      memset( new_item, 0, sizeof( item ) );
      strncpy( new_item->name, parname, 31 );
      strncpy( new_item->value, pardata, 255 );
      lnklist_add( current_section->items, new_item );
      
      /* increase the counter */
      items++;
    }
  }
  
  fclose( parseme );
  return items;
}

/* get */
const char* 
config_getstr( config *cfg, const char *sect_name, const char *item_name )
{
  cfginfo *conf = (cfginfo*)cfg;
  section *sect = NULL;
  item *itm;
  int no, i, found = 0;
  
  /* is there a config ? */
  if( !conf )
  {
    return NULL;
  }
  
  /* are there any sections ? */
  no = lnklist_count( conf->sections );
  if( !no ) return NULL;
  
  /* search for the section */
  lnklist_first( conf->sections );
  for( i = 0; i < no; i++ )
  {
    sect = (section*)lnklist_data( conf->sections );
    
    /* should never happen */
    if( !sect ) continue;
    
    /* is this the section ? */
    if( _strnicmp( sect->name, sect_name, 31 ) == 0 )
    {
      /* escape the loop */
      found = 1;
      break;
    }
    
    lnklist_next( conf->sections );
  }
  
  /* have we found anything ? */
  if( !found ) return NULL;
  
  /* check the item list count */
  no = lnklist_count( sect->items );
  if( !no ) return NULL;
  
  /* search for item */
  lnklist_first( sect->items );
  for( i = 0; i < no; i++ )
  {
    itm = (item*)lnklist_data( sect->items );
    
    /* should never happen */
    if( !itm ) continue;
    
    /* is this the item ? */
    if( _strnicmp( itm->name, item_name, 31 ) == 0 )
    {
      /* return the value */
      return itm->value;
    }
    
    lnklist_next( sect->items );
  }
  
  return NULL;
}

