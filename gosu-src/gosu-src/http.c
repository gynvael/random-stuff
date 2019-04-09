/* HTTP 1.1 praser
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 15.12.2003
 * Desc.  : http parser, nothing else
 * RFC    : RFC-2616 Hypertext Transfer Protocol -- HTTP/1.1
 *        : RFC-1123 Requirements for Internet Hosts -- Application and Support
 * Notes  : HA! Uwielbiam pisac wg. rfc! Porzadna dokumentacja =^^=
 */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include"varlst.h"
#include"string_ext.h"
#include"memory.h"
int	_stricmp (const char*, const char*);

/* functions
 */
 
int
count_unicode( char term_char, const char *what )
{
  const char *r = what;
  int sth = 0;
  
  while( *r && *r != term_char )
  {
    if( *r == '%' )
    {
      r+=3;
      sth++;
      continue;
    }
    sth++;
    r++;
  }
  
  return sth;  
}
 
void
make_unicode( char *what )
{
  char *r = what, *m = what;
  int sth;
  
  while( *r )
  {
    if( *r == '+' )
    {
      *m = ' ';
      m++;
      r++;
      continue;
    }
    else if( *r == '%' )
    {
      r++;
      sscanf( r, "%2x", (unsigned int*)&sth );
      *(m++) = sth;
      r+=2;      
      continue;
    }
    *m = *r;
    m++;
    r++;
  }
  *m = '\0';
}

const char*
copy_unicode( char *where, const char *what, char term )
{
  const char *r = what;
  char *m = where;
  int sth;
  
  while( *r && *r != term )
  {
    if( *r == '+' )
    {
      *m = ' ';
      m++;
      r++;
      continue;
    }
    else if( *r == '%' )
    {
      r++;
      sscanf( r, "%2x", (unsigned int*)&sth );
      *(m++) = sth;
      r+=2;      
      continue;
    }
    *m = *r;
    m++;
    r++;
  }
  *m = '\0';
  return r;
}

void
parse_post_data( const char *where, void *env )
{
  char *pvalbuf;
  char valbuf[ 256 ], namebuf[ 32 ];
  const char *pwh = where;
  int sz;
  
  /*
  puts( where );
  printf( "US -> %i %i\n", count_unicode( '\0', where ), strlen( where ) );
  1. dotrzec do = albo & albo \0
   =. policzyc ile znakow do & albo \0
      jesli wieksze niz 255, zaalokowac nowy buffer
      skopiowac dane do nowego lub tymczasowego bufforku
      wrzucic na liste
   &. skopiowac nazwe do bufforka
      wrzucic na liste
      kontynuowac
   0. skopiowac nazwe do bufforka
      wrzucic na liste
      wyjsc z petli
  */
  
  while( *pwh )
  {
    /* 1 */
    sz = 0;
    while( *pwh && *pwh != '=' && *pwh != '&' ) 
    {
      if( sz < 31 )
      {
        namebuf[ sz ] = *pwh;
        sz++;
      }
      pwh++;
    }
    namebuf[ sz ] = '\0';
    
    /* switcheeee swiitcheeee */
    switch( *pwh )
    {
      case '=':
        pwh++;
        sz = count_unicode( '&', pwh );
        if( sz > 255 )
        {
          pvalbuf = (char*)malloc( sz+1 );
          if( !pvalbuf ) return;
        }
        else
        {
          pvalbuf = valbuf;
        }
        pwh = copy_unicode( pvalbuf, pwh, '&' );
        if( *pwh == '&' ) pwh++;
        
        if( sz > 255 )
        {
          varlst_setvalueex( env, namebuf, pvalbuf );
        }
        else
        {
          varlst_setvalue( env, namebuf, pvalbuf );
        }
      break;
      
      case '&':
        varlst_setvalue( env, namebuf, "" );
      break;
      
      case '\0':
        varlst_setvalue( env, namebuf, "" );
      break;
    }
  }
}
 
void
uri_parse( char *uri, void *http, void *env )
{
  char *now = uri;
  static char *temp_name;
  static char *temp_value;
  
  /* go to ? */
  while( *now && ( *now!='?' ) ) now++;
  
  /* did we hit the question mark ? */
  if( !(*now) )
  {
    /* set file */
    make_unicode( uri );
    varlst_setvalue( http, "REQUEST", uri );
    return;    
  }
  
  /* set \0 and set file */
  *now = '\0';
  make_unicode( uri );
  varlst_setvalue( http, "REQUEST", uri );
  now++;
  
  /* begin variable parsing */
  while( *now )
  {
    temp_name = now;
    
    /* jump over all thouse leters ;p */
    while( (*now) && ( (*now != '&') && (*now != '=') ) ) now++;
    if( !(*now) || ( *now == '&' ) )
    {
      if( *now ) *(now++) = '\0';
      make_unicode( temp_name );
      varlst_setvalue( env, temp_name, "" );
      /*printf(" ENV: %s == \"\"\n", temp_name );*/
      continue;
    }
    
    /* get value */
    *(now++) = '\0';
    temp_value = now;
    
    /* jump over all thouse leters ;p */
    while( (*now) && (*now != '&') ) now++;
    
    if( *now ) *(now++) = '\0';
    make_unicode( temp_name );
    make_unicode( temp_value );
    varlst_setvalue( env, temp_name, temp_value );
    /* printf(" ENV: %s == \"%s\"\n", temp_name, temp_value ); */
    continue;
  }
  
}

/* parse */
int 
http_parse( const char *what, int n, void *http, void *env )
{
  static char line[ 1024 ];
  char *method, *uri, *httpver, *nm, *val;
  const char *where = what;
  int i;
  
  /* parse first line */
  /* copy line and get method */
  where = copy_line( line, where, 1024 );
  method = line;
  if( !(*method) ) 
  {
    varlst_setvalue( http, "ERROR", "invalid sequence" );
    return -1;
  }
   
  /* set \0 and set method */
  uri = end_of_name_sp( method );
  if( !(*uri) )
  {
    varlst_setvalue( http, "ERROR", "no URI found" );
    return -1;
  }
  *uri = '\0';
  varlst_setvalue( http, "METHOD", method );
  
  /* set \0 and get URI */
  uri ++;
  httpver = end_of_name_sp( uri );
  if( !(*httpver) )
  {
    varlst_setvalue( http, "ERROR", "invalid request" );
    return -1;
  }
  *httpver = '\0';
  httpver++;
  
  /* get HTTP version */
  i = strlen( httpver );
  httpver = strchr( httpver, '/' ) + 1;
  if( httpver == (char*)1 )
  {
    varlst_setvalue( http, "ERROR", "invalid version format" );
    return -1;
  }
  
  /* set HTTP version */
  varlst_setvalue( http, "HTTP", httpver );
  
  /* parse uri */
  uri_parse( uri, http, env );
  
  /* params parser */
  do
  {
    where = copy_line( line, where, 512 );
    if( ! (*line) ) break;
    
    /* find : */
    nm = line;
    val = strchr( line, ':' );
    if( !val ) continue;
    
    /* enter there \0 */
    *(val++) = '\0';
    
    /* jump spaces */
    val = jump_over_blank( val );
    if( ! (*val) ) continue;
    
    /* write them into http */
    varlst_setvalue( http, nm, val );
    
  }
  while( *where );
  
  if( *where && ( _stricmp( varlst_getvalue( http, "METHOD" ), "POST" ) == 0 ) )
  {
    parse_post_data( where, env );
  }
  
  
  return 0;
}

/* date
 */
const char wkday[ 7 ][ 4 ] =
{
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
const char month[ 12 ][ 4 ] = 
{
  "Jan", "Feb", "Mar", "Apr",
  "May", "Jun", "Jul", "Aug",
  "Sep", "Oct", "Nov", "Dec"
};
 
const char* 
http_now( void )
{
  static char date[ 32 ]; /* i know that 30 bytes would do ;p */
  time_t now;
  struct tm *snow;
  
  /* get the time */
  time( &now );
  snow = gmtime( &now );
  
  /* rfc1123-date */
  sprintf( date, "%3s, %.2u %3s %.4u %.2u:%.2u:%.2u GMT",
  /* wkday */ wkday[ snow->tm_wday ],
  /*   day */ snow->tm_mday,
  /* month */ month[ snow->tm_mon ],
  /*  year */ snow->tm_year + 1900,
  /*  hour */ snow->tm_hour,
  /*   min */ snow->tm_min,
  /*   sec */ snow->tm_sec );
  
  return (const char*)date;
}
