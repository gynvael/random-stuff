/* Replace "dll.h" with the name of your header */
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<io.h>
#include"..\..\plugin_api\gosuapi.h"
#include"..\..\gosu-src\config.h"

/* defines */
#define GETVAL(a,b,c) if( !(a = varlst_getvalue( b, c ) ) ) a = "";

/* global */
static char buffer[ 2048 ];
static unsigned int proj_num, total_code, total_size;

/* structs */
typedef struct def_systemtime 
{
	short year;
	short month;
	short dayofweek;
	short day;
	short h;
	short min;
	short sec;
	short ms;
} systemtime;

/* constants */
const char msk_vinfo[ ] =
 "<table width=\"100%%\"><tr><td align=\"right\" width=20%%\">\r\n"
 "Name:<BR>Author(s):<BR>Version:<BR>"
 "State:<BR>Directory:<BR>Short Description:<BR>"
 "Code Size:<BR>Total Size\r\n"
 "</td><td style=\"font-weight: bold; color: blue\">\r\n"
 "%s<BR>%s<BR>%s<BR>%s<BR>%s<BR>%s<BR>%u<BR>%u\r\n"
 "</td></tr></table><BR>";
 /* name, author, ver, state, dir, desc, codecnt, allcnt */

const char msk_vhead[ ] =
 "<div class=\"qmborder\">%s</div>\r\n";
 /* section name */
 
const char msk_vret[ ] = 
 "<a href=\"%s?%s\" class=\"sth\">"
 "%sreturn to main project page%s</a>"; 
 /* req, adddata, sel_left, sel_right */

const char msk_entry[ ] =
 "<tr class=\"projenum%u\">\r\n"
 "<td class=\"projleft\">"
 "<a href=\"%s?verbose=%s\\%s&%s\">%s</a></td>\r\n"
 "<td>%s</td>\r\n"
 "<td>%s</td>\r\n"
 "<td>%u</td>\r\n"
 "<td>%u</td>\r\n"
 "<td class=\"projright\">%s</td>\r\n"
 "</tr>\r\n\r\n";
 /* bg, req, src, path, adddata, name, version, state, code, total, desc */

const char msk_bottom[ ] =
 "<tr class=\"projlistfoot\">\r\n"
 "<td class=\"projmleft\" colspan=\"4\" style=\"text-align: right\">\r\n"
 "number of projects:<BR>\r\ntotal code / total size:\r\n"
 "</td><td class=\"projmright\" colspan=\"2\">\r\n"
 "<B><font color=\"blue\">%u<BR>\r\n%u</font></b> / "
 "<b><font color=\"blue\">%u</b>\r\n"
 "</td></tr></table>\r\n\r\n";
 /* proj_num, total_code, total_size */
 
const char stats_chart_none[ ] = 
"&nbsp\r\n";
/* none */

const char stats_chart_neg[ ] =
"<div class=\"statsred\" style=\"width: %upx;\">%i</div>\r\n";
/* width <10;250>, change */

const char stats_chart_pos[ ] =
"<div class=\"statsblue\" style=\"width: %upx;\">+%i</div>\r\n";
/* width <10;250>, change */

const char stats_item[ ] =
"<tr class=\"projenum%u\">\r\n"
"<td class=\"projleft\">%.2u.%.2u.%.4u</td>\r\n"
"<td>%u</td>\r\n"
"<td align=\"right\" width=\"30%%\">%s</td>\r\n"
"<td align=\"left\" width=\"30%%\" class=\"projright\">%s</td>\r\n"
"</tr>\r\n\r\n";
/* day, month, year, neg, pos */
 
const char txt_statstop[ ] =
 "<table width=\"100%%\" cellspacing=\"0\">\r\n"
 "<tr class=\"projlist\">\r\n"
 "<td width=\"20%%\" class=\"projmleft\">Date</td>\r\n"
 "<td width=\"20%%\" class=\"projm\">Total</td>\r\n"
 "<td class=\"projmright\" colspan=\"2\">Chart</td>\r\n"
 "</tr>\r\n\r\n";

const char txt_statsbottom[ ] =
 "<tr><td colspan=\"4\" align=\"center\" class=\"blackborder\">---"
 "</td></tr></table>\r\n\r\n";

const char txt_top[ ] =
 "<table width=\"100%%\" cellspacing=\"0\">\r\n"
 "<tr class=\"projlist\">\r\n"
 "<td width=\"20%%\" class=\"projmleft\">Name</td>\r\n"
 "<td width=\"10%%\" class=\"projm\">Version</td>\r\n"
 "<td width=\"10%%\" class=\"projm\">State</td>\r\n"
 "<td width=\"10%%\" class=\"projm\">Code Size</td>\r\n"
 "<td width=\"10%%\" class=\"projm\">Total</td>\r\n"
 "<td width=\"35%%\" class=\"projmright\">Short Desc</td>\r\n"
 "</tr>\r\n\r\n";
 
const char txt_notfound[ ] =
 "<center>Directory not found!</center>";
 
const char txt_pre[ ] =
 "<pre>";
 
const char txt_prex[ ] =
 "</pre>";
 
const char txt_br[ ] =
 "<br/>";
 
const char txt_na[ ] =
 "<center>n/a!</center>";
 
const char txt_sp[ ] =
 "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | |-";

/* plugin command list */
char *plugin_command[ ] =
{
  "cmd_reset",     "proj_reset",
  "cmd_head",      "proj_head",
  "cmd_enum",      "proj_enum",
  "cmd_foot",      "proj_foot",
  "cmd_verbose",   "proj_verbose",
  "cmd_stats",     "proj_stats"
};

/* uploads the plugin command list to GoSu
 */
void init( plugins *plg )
{
  int s = sizeof( plugin_command ) / ( sizeof( char* ) * 2 );
  int i;
  for( i = 0; i < s; i++ )
  {
    gosuapi_addcmd( plg, plugin_command[ i * 2 ], plugin_command[ i * 2 + 1 ] ); 
  }
}

/* info stuff */
void info( char *buffer, int n )
{
  _snprintf( buffer, n, "GoSu Project Manager" );
}

/* some functions */
void 
countfiles( const char *path, unsigned int *codecnt, unsigned int *allcnt )
{
  long handle;
  struct _finddata_t data;
  char seekme[ 256 ], seekhim[ 256 ];
  
  _snprintf( seekme, 256, "%s\\*.*", path );
    
  handle = _findfirst( seekme, &data );
  if( handle == -1 ) return;
  
  do
  {
    if( *data.name == '.' ) continue;
    if( data.attrib & _A_SUBDIR ) 
    {
      if( data.name[ 0 ] == '.' && 
          ( data.name[ 1 ] == '\0' || 
            ( data.name[ 1 ] == '.' && data.name[ 2 ] == '\0' ) ) )
        continue;
        
      _snprintf( seekhim, 256, "%s\\%s", path, data.name );
      countfiles( seekhim, codecnt, allcnt );
      continue;      
    }
    if( strstr( data.name, ".c" ) ) *codecnt += data.size;
    else if( strstr( data.name, ".h" ) ) *codecnt += data.size;
    else if( strstr( data.name, "makefile" ) ) *codecnt += data.size;
    
    *allcnt += data.size;
    
  } 
  while( !_findnext( handle, &data ) );
  
  _findclose( handle );
}

void 
listfiles( void *output, const char *path, int lvl )
{
  long handle;
  struct _finddata_t data;
  char seekme[ 256 ], seekhim[ 256 ];
  int ret;
  
  _snprintf( seekme, 256, "%s\\*.*", path );
    
  handle = _findfirst( seekme, &data );
  if( handle == -1 ) return;
  
  do
  {
    if( *data.name == '.' ) continue;
    if( data.attrib & _A_SUBDIR ) 
    {
      if( data.name[ 0 ] == '.' && 
          ( data.name[ 1 ] == '\0' || 
            ( data.name[ 1 ] == '.' && data.name[ 2 ] == '\0' ) ) )
        continue;
        
      _snprintf( seekhim, 256, "%s\\%s", path, data.name );
      ret = _snprintf( buffer, 1024, "%s\\ <font color=\"green\">%s</font>\r\n", 
      &txt_sp[ sizeof( txt_sp ) - lvl*2 - 1 ], data.name );
      OUTPROC(output)( buffer, ret );
      listfiles( output, seekhim, lvl+1 );
      continue;      
    }
    
    ret = _snprintf( buffer, 1024, "%s <font color=\"blue\">%s</font>\r\n", 
      &txt_sp[ sizeof( txt_sp ) - lvl*2 - 1 ], data.name );
    OUTPROC(output)( buffer, ret );
    
  } 
  while( !_findnext( handle, &data ) );
  
  _findclose( handle );
}

void
sendfile( void *where, FILE *f )
{
  unsigned int i;
  
  /* send teh file */
  do
  {
    i = fread( buffer, 1, 1024, f );
    if( i ) OUTPROC(where)( buffer, i );
  }
  while( i == 1024 );
}

void
filesect( void *output, const char *src, const char *fname, char *namebuf,
          const char *name )
{
  int ret;
  FILE *f;
  
  ret = _snprintf( buffer, 1024, msk_vhead, name );
  OUTPROC(output)( buffer, ret );
  
  if( !fname )
  {
    OUTPROC(output)( txt_br, sizeof( txt_br ) - 1 );
    OUTPROC(output)( txt_na, sizeof( txt_na ) - 1 );
    OUTPROC(output)( txt_br, sizeof( txt_br ) - 1 );
  }
  else
  {
    _snprintf( namebuf, 1024, "%s\\%s", src, fname );
    f = fopen( namebuf, "r" );
    if( !f )
    {
      OUTPROC(output)( txt_br, sizeof( txt_br ) - 1 );
      OUTPROC(output)( txt_na, sizeof( txt_na ) - 1 );
      OUTPROC(output)( txt_br, sizeof( txt_br ) - 1 );
    }
    else
    {
      OUTPROC(output)( txt_pre, sizeof( txt_pre ) - 1 );
      sendfile( output, f );
      fclose(f);
      OUTPROC(output)( txt_prex, sizeof( txt_prex ) - 1 );
      OUTPROC(output)( txt_br, sizeof( txt_br ) - 1 );
    }    
  }
}

/* commands */
PROTOTYPE( cmd_reset )
{
  proj_num = total_code = total_size = 0;
  return 0;
}

PROTOTYPE( cmd_head )
{
  OUTPROC(output)( txt_top, sizeof( txt_top ) - 1 );
  return 0;
}

PROTOTYPE( cmd_foot )
{
  int ret;
  
  ret = _snprintf( buffer, 1024, msk_bottom, proj_num, total_code, total_size );
  OUTPROC(output)( buffer, ret );
  return 0;
}

PROTOTYPE( cmd_enum )
{
  const char *req; /* the place where the link vault is (on www) */
  const char *adddata; /* additional get data */
  const char *src; /* source */
  const char *c_name = NULL, *c_version = NULL, *c_state = NULL, *c_desc = NULL;
  config *ini;
  unsigned int bg = 0;
  unsigned int codecnt, allcnt; 
  int ret;
  long handle;
  struct _finddata_t data;
  static char namebuf[ 1024 ];
  
  /* get source dir */
  src = varlst_getvalue( params, "src" );
  if( !src ) return 0;
  
  /* get req and adddata */
  GETVAL( req, client, "request" );
  GETVAL( adddata, params, "adddata" );
  
  /* get all projects */
  _snprintf( namebuf, 1024, "%s\\*.*", src );
  handle = _findfirst( namebuf, &data ); 
  if( handle == -1L ) return 0;
  
  do
  {
    /* continue on special dir names ( . (current dir) and .. (parent dir) ) */
    if( data.name[ 0 ] == '.' && 
        ( data.name[ 1 ] == '\0' || 
          ( data.name[ 1 ] == '.' && data.name[ 2 ] == '\0' ) ) )
      continue;
      
    /* check if what we found is a dir */
    if( !( data.attrib & _A_SUBDIR ) ) continue;
    
    /* zero */
    c_name = NULL;
    c_version = NULL;
    c_state = NULL;
    c_desc = NULL;
    
    /* ok, get the ini file */
    _snprintf( namebuf, 1024, "%s\\%s\\info.ini", src, data.name );
    ini = config_open( namebuf );
    if( !ini ) continue;
    
    /* try it */
    ret = config_parse( ini );
    if( ret > 0 )
    {
      c_name = config_getstr( ini, "default", "title" );
      c_version = config_getstr( ini, "default", "ver" );
      c_state = config_getstr( ini, "default", "state" );
      c_desc = config_getstr( ini, "default", "desc" );    
    }
  
    if( !c_name ) c_name = data.name;
    if( !c_version ) c_version = "?";
    if( !c_state ) c_state = "?";
    if( !c_desc ) c_desc = "n/a";
    
    /* get dir size */
    codecnt = 0;
    allcnt = 0;
    _snprintf( namebuf, 1024, "%s\\%s", src, data.name );
    
    countfiles( namebuf, &codecnt, &allcnt );
    
    total_code += codecnt;
    total_size += allcnt;
 
    /* fill the table */
    ret = _snprintf( buffer, 1024, msk_entry,
          bg, req, src, data.name, adddata,
          c_name, c_version, c_state, codecnt, allcnt, 
          c_desc );
         
    OUTPROC(output)( buffer, ret );        
  
    /* most important line ;p */
    bg = !bg; 
    config_close( ini );
                 
  }
  while( !_findnext( handle, &data ) );
  
  _findclose( handle );
  
  return 0;
}



PROTOTYPE( cmd_verbose )
{
  const char *req; /* the place where the link vault is (on www) */
  const char *adddata; /* additional get data */
  const char *src; /* source */
  const char *sel_left, *sel_right;
  const char *c_name = NULL,
             *c_version = NULL, 
             *c_state = NULL,
             *c_desc = NULL,
             *c_author = NULL,
             *c_dir = NULL,
             *c_longdesc = NULL,
             *c_history = NULL,
             *c_todolist = NULL,
             *c_licence = NULL;
                          
  config *ini;
  unsigned int bg = 0;
  unsigned int codecnt, allcnt; 
  int ret;
  long handle;
  struct _finddata_t data;
  static char namebuf[ 1024 ];
  FILE *f;
  
  /* get source dir */
  src = varlst_getvalue( params, "src" );
  if( !src ) return 0;
  
  /* get req and adddata */
  GETVAL( req, client, "request" );
  GETVAL( adddata, params, "adddata" );
  GETVAL( sel_left, env, "l" );
  GETVAL( sel_right, env, "r" );
  
  /* check dir */
  handle = _findfirst( src, &data ); 
  if( handle == -1L ) return 0;
  if( ! ( data.attrib & _A_SUBDIR ) )
  {
    _findclose( handle );
    OUTPROC(output)( txt_notfound, sizeof( txt_notfound ) - 1 ) ;
    ret = _snprintf( buffer, 1024, msk_vret, req, adddata, sel_left, sel_right );
    OUTPROC(output)( buffer, ret );
    return 0;
  }
  
  /* open and parse ini file */
  _snprintf( namebuf, 1024, "%s\\info.ini", src );
  ini = config_open( namebuf );
  ret = config_parse( ini );
  if( ret > 0 )
  { 
     c_name = config_getstr( ini, "default", "title" );
     c_version = config_getstr( ini, "default", "ver" );
     c_state = config_getstr( ini, "default", "state" );
     c_desc = config_getstr( ini, "default", "desc" );
     c_author = config_getstr( ini, "default", "author" );
     c_longdesc = config_getstr( ini, "default", "longdesc" );
     c_history = config_getstr( ini, "default", "history" );
     c_todolist = config_getstr( ini, "default", "todolist" );
     c_licence = config_getstr( ini, "default", "licence" );
  }
  
  c_dir = src;
  if( !c_name ) c_name = src;
  if( !c_version ) c_version = "?";
  if( !c_state ) c_state = "?";
  if( !c_desc ) c_desc = "n/a";
  if( !c_author ) c_author = "?";
  
  codecnt = 0;
  allcnt = 0;
  countfiles( src, &codecnt, &allcnt );
  
  /* the info sect */
  ret = _snprintf( buffer, 1024, msk_vhead, "Project Info" );
  OUTPROC(output)( buffer, ret );
  ret = _snprintf( buffer, 1024, msk_vinfo,
  c_name, c_author, c_version, c_state, c_dir, c_desc, codecnt, allcnt );
  OUTPROC(output)( buffer, ret );
  
  /* long desc */
  filesect( output, src, c_longdesc, namebuf, "Description" );
  
  /* to do */
  filesect( output, src, c_todolist, namebuf, "To Do List" );
  
  /* history */
  filesect( output, src, c_history, namebuf, "Changes & History" );
  
  /* licence */
  filesect( output, src, c_licence, namebuf, "Licence" );
  
   /* file list */
  ret = _snprintf( buffer, 1024, msk_vhead, "File List" );
  OUTPROC(output)( buffer, ret );
  OUTPROC(output)( txt_pre, sizeof( txt_pre ) - 1 );
  ret = _snprintf( buffer, 1024, "<font color=\"green\">%s</font>\r\n", src );
  OUTPROC(output)( buffer, ret );
  listfiles( output, src, 1 );
  OUTPROC(output)( txt_prex, sizeof( txt_prex ) - 1 );
  OUTPROC(output)( txt_br, sizeof( txt_br ) - 1 );
  
  /* return */
  ret = _snprintf( buffer, 1024, msk_vret, req, adddata, sel_left, sel_right );
  OUTPROC(output)( buffer, ret );
  
  config_close( ini );

  return 0;
}

PROTOTYPE( cmd_stats )
{
  const char *src;
  systemtime ct;
  FILE *stats;
  int ret;
  int s, d, i, j, total_code, diff, width;
  int entry_size = ( sizeof( systemtime ) + sizeof( int ) );
  char chart_neg[ 128 ], chart_pos[ 128 ];
  
  src = varlst_getvalue( params, "src" );
  if( !src ) return 0;
  
  stats = fopen( src, "rb" );
  if( !stats ) return 0;
  
  fseek( stats, 0, SEEK_END );
  s = ftell( stats );
  fseek( stats, 0, SEEK_SET );
  if( !s )
  {
    fclose( stats );
    return 0;
  }
  
  OUTPROC(output)( txt_statstop, sizeof( txt_statstop ) - 1 );
  
  j = s / entry_size;
  
  for( i = 0; i < j; i++ )
  {
    fread( &ct, sizeof( systemtime ), 1, stats );
    fread( &total_code, sizeof( int ), 1, stats );
    if( i )
    {
      diff = total_code - d;
      if( diff < 0 )
      {
        width = ( -diff ) / 10;
        if( width < 30 ) width = 30;
        if( width > 200 ) width = 200;
          
        strcpy( chart_pos, stats_chart_none );
        sprintf( chart_neg, stats_chart_neg, width, diff );
          
      }
      else if( diff > 0 )
      {
        width = ( diff ) / 100;
        if( width < 30 ) width = 30;
        if( width > 200 ) width = 200;
          
        strcpy( chart_neg, stats_chart_none );
        sprintf( chart_pos, stats_chart_pos, width, diff );
      }
      else
      {
        strcpy( chart_neg, stats_chart_none );
        strcpy( chart_pos, stats_chart_none );
      }
      
      
      /* day, month, year, neg, pos */
      ret = _snprintf( buffer, 2048, stats_item, 
                 (i-1)%2, ct.day, ct.month, ct.year,
                 total_code, chart_neg, chart_pos );
      OUTPROC(output)( buffer, ret );
    }
    
    d = total_code;
  }
  
  OUTPROC(output)( txt_statsbottom, sizeof( txt_statsbottom ) - 1 );
  fclose( stats ); 
  
  
  return 0;
}

