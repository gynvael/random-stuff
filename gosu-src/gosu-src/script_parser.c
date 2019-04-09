/* GoSu Script Parser
 * Ver.   : 0.0.2
 * Author : gynvael.coldwind//vx
 * Date   : 10.12.2003
 * Desc.  : like the sign says...
 */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"string_ext.h"
#include"varlst.h"
#include"config.h"
#include"plugins.h"
#include"memory.h"

/* types
 */
typedef void srcparser;
 
typedef struct def_prsinfo
{
  config *cfg;
  plugins *plg;
  varlst *prm;
  char *outbuf;
} prsinfo;

/* 64k should do */
#define BUFFER_SIZE 0x10000
#ifndef OUTPROC
  #define OUTPROC(a) ((void(*)(char*,int))(a))
#endif


/* open */
srcparser*
srcparser_open( config *cfg, plugins *plg )
{
  prsinfo *new_prs;
  
  /* is there a cfg file and plg ? */
  if( !cfg ) 
  {
    #ifdef DEBUG
    puts( "no config file" );
    #endif
    return NULL;
  }
  if( !plg )
  {
    #ifdef DEBUG
    puts( "no plugins" );
    #endif
    return NULL;
  }
  
  /* allocate memory */
  new_prs = (prsinfo*)malloc( sizeof( prsinfo ) );
  if( !new_prs ) 
  {
    #ifdef DEBUG
    puts( "could not alloc mem (1)" );
    #endif
    return NULL;
  }
  
  new_prs->outbuf = (char*)malloc( BUFFER_SIZE );
  if( !new_prs->outbuf ) 
  {
    #ifdef DEBUG
    puts( "could not alloc mem (2)" );
    #endif
    free( new_prs );
    return NULL;
  }
  
  /* set config info */
  new_prs->cfg = cfg;
  
  /* create param list */
  new_prs->prm = varlst_create ( );
  if( !new_prs->prm )
  {
    #ifdef DEBUG
    puts( "could not create var list" );
    #endif
    free( new_prs->outbuf );
    free( new_prs );
    return NULL;
  }
  
  /* set plugins pack */
  new_prs->plg = plg;

  /* exit */
  return new_prs;
}

/* close */
void 
srcparser_close( srcparser *prs )
{
  prsinfo *parser = (prsinfo*)prs;
  
  /* do we have a prs ? */
  if( !parser ) return;
  
  /* free memory */
  free( parser->outbuf );
  free( prs );
}

/* execute (private) */
void
srcparser_execute( prsinfo *parser, void *output, varlst *env, varlst *client )
{
  char cmdname[ 64 ];
  char temp_itemname[ 32 ];
  char temp_itemvalue[ 256 ];
  char temp_env[ 32 ];
  char *where;
  char *tmp;
  char *tmpenv;
  int i, j;
  int isvalue = 0;
  
  /* get rid of blank space */
  where = jump_over_blank( parser->outbuf );
  if( !(*where) ) return;
  
  /* copy cmd name till \0 or blank space */
  tmp = cmdname;
  i = 0;
  while( (*where) && (*where != ' ') && (*where != '\t') )
  {
    /* hehe sorry, no buffer overflow ;p */
    if( i == 64 ) break;
    i++;

    /* copy char */
    *(tmp++) = *(where++);
  }
  *tmp = '\0';
  
  /* get the param list */
  while( *where )
  {
    /* jump over blank */
    where = jump_over_blank( where );
    if( !(*where) ) break;
    
    /* get param name */
    tmp = temp_itemname;
    i = 0;
    while( (*where) && (*where != ' ') && (*where != '\t') && (*where != '=') )
    {
      /* hehe sorry, no buffer overflow ;p */
      if( i == 32 ) break;
      i++;

      /* copy char */
      *(tmp++) = *(where++);
    }
    *tmp = '\0';
    
    /* jump over blank */
    where = jump_over_blank( where );
    if( !(*where) ) break;
    
    /* jump over = */
    if( *where != '=' ) break;
    where++;
    
    /* jump over blank */
    where = jump_over_blank( where );
    if( !(*where) ) break;
    
    /* get param data */
    tmp = temp_itemvalue;
    i = 0;
    while( *where )
    {
      /* hehe sorry, no buffer overflow ;p */
      if( i == 256 ) break;
      i++;
      
      /* end if blank */
      if( !isvalue && ( ( *where == ' ' ) || ( *where == '\t' ) ) )
      {
        break;
      }
      
      /* set isvalue */
      if( ( *where == '\'' ) || ( *where == '\"' ) )
      {
        isvalue = !isvalue;
        where++;
        continue;
      }
      
      /* is it an env value ? */
      if( *where == '%' )
      {
        *where++;
        
        /* did we get to the end ? */
        if( !(*where) )
        {
          break;          
        }
        
        /* copy param name */
        tmpenv = temp_env;
        j = 0;
        while( *where && (*where != '%') )
        {
          /* no buf overflow! */
          if( j < 32 )
          {
            *(tmpenv++) = *(where++);
            j++;
          }
          else
          {
            where++;
          }
        }
        *tmpenv = '\0';
        
        /* jump over % */
        if( *where == '%' ) *where++;
        
        /* append env to buffer */
        tmpenv = ( char* ) varlst_getvalue( env, temp_env );
        if( !tmpenv ) continue;
        while( *tmpenv )
        {
          *(tmp++) = *(tmpenv++); 
           
          /* hehe sorry, no buffer overflow ;p */
          if( i == 256 ) break;
          i++;          
        }
        
        /* continue */
        continue;
      }
      
      /* check for specials \ */
      if( *where == '\\' )
      {
        where++;
        
        /* did we get to the end ? */
        if( !(*where) )
        {
          break;          
        }
        
        /* check for specials */
        if( *where == 'n' ) 
        {
          *(tmp++) = '\n';
          continue;
        } 
        else if( *where == 'r' ) 
        {
          *(tmp++) = '\r';
          continue;
        }
        else if( *where == 't' ) 
        {
          *(tmp++) = '\t';
          continue;
        }        
      }

      /* copy char */
      *(tmp++) = *(where++);
    }
    *tmp = '\0';
    
    /* add to varlst */
    varlst_setvalue( parser->prm, temp_itemname, temp_itemvalue );
  }

  /* run it */  
  plugins_run( parser->plg, output, cmdname, parser->prm, env, client );
  
  /* clean var lst */
  varlst_clean( parser->prm );
}

/* parse */
void srcparser_parse( srcparser *prs,
                      const char *data, int n,
                      varlst *env, varlst *client,
                      void *output )
{
  prsinfo *parser = (prsinfo*)prs;
  int i, outsize;
  char *where;
  char isvalue = 0;
  
  /* do we have a prs ? */
  if( !parser ) return;
  
  /* parsing loop */
  for( i = 0, outsize = 0, where = parser->outbuf; i < n; i++ )
  {
    /* do we have a command tag ? '<@' */
    if( ( data[ i ] == '<' ) && ( ( i+1 < n ) && ( data[ i + 1 ] == '@' ) ) )
    {
      
      /* send the data we have in the buffer */
      if( outsize )
      {
        OUTPROC(output)( parser->outbuf, outsize );
        outsize = 0;
        where = parser->outbuf;
      }
      
      /* jump over command tag */
      i += 2;
      if( i >= n ) return;
      
      /* copy the stuff until end of data or end of command tag */
      for( isvalue = 0; i < n; i++ )
      {
        /* was the command tag ended ? */
        if( (data[ i ] == '>') && !isvalue )
        {
          /* end the buf end escape from loop 
           * we don't have to jump over >, the main loop will do it
           */          
          *where = '\0';
          break;
        }
        
        /* did we find a " or ' ? */
        if( (data[ i ] == '\'') || (data[ i ] == '\"') )
        {
          isvalue = !isvalue ;
        }
        
        /* was there a \ ? */
        if( (data[ i ] == '\\') && isvalue )
        {
          *(where++) = data[ i ];
          
          /* jump over \ */
          i++;
          if( i >= n ) break;
        }
        
        /* strip eoln */
        if ( (data[ i ] == '\r') || (data[ i ] =='\n') )
        {
          i++;
          continue;
        }
        
        /* copy data to buf */
        *(where++) = data[ i ];
      }
      
      /* parse command */
      #ifdef DEBUG
      printf("command found: %s\n", parser->outbuf );
      #endif
      srcparser_execute( parser, output, env, client );
      
      /* return to normal mode */
      where = parser->outbuf;
    }
    else
    {
      /* write data to outbuf */
      *(where++) = data[ i ];
      outsize++;
    }
  }
  
  /* send the rest of data */
  if( outsize )
  {
    OUTPROC(output)( parser->outbuf, outsize );
  }
}

