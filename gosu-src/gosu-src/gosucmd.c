/* GoSu @gosu Commands
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind/vx
 * Date   : 17.12.2003
 * Desc.  : @gosu commands
 */
#include<windows.h>
#include<stdio.h>
#include<string.h>
#include"config.h"
#include"varlst.h"
#include"linked_list.h"
#include"script_parser.h"
#include"string_ext.h"
#include"plugins.h"
#include"memory.h"

/* some defines */
int	_stricmp (const char*, const char*);
#define PROTOTYPE(a) int a( plugins *plg, varlst *params, varlst *env, varlst *client, void *output )
#define OUTPROC(a) ((void(*)(const char*,int))(a))

/* structs */
typedef struct def_command
{
  char name[ 64 ];
  int (*proc)( plugins*, lnklist*, void* );
} command;

typedef struct def_pluginsinfo
{
  lnklist *dlls;
  lnklist *commands;
  config *cfg;
  char sect_name[ 32 ];
} pluginsinfo;

typedef struct def_plginfo
{
  HINSTANCE handle;
  char filename[ 256 ];
  char name[ 128 ];
} plginfo;


/* get access to some of the values and functions */
extern varlst *http;
extern config *conf;
extern const char *GoSu_version;

/* statics */
#define BIGBUF_SIZE 4096
static char tempbuf[ 256 ];
static char bigbuf[ BIGBUF_SIZE ];

/* commands */
PROTOTYPE(cmd_insert);
PROTOTYPE(cmd_include);
PROTOTYPE(cmd_if);
PROTOTYPE(cmd_echo);
PROTOTYPE(cmd_echo_env);
PROTOTYPE(cmd_setenv);
PROTOTYPE(cmd_version);
PROTOTYPE(cmd_glist);
PROTOTYPE(cmd_diskinfo);
PROTOTYPE(cmd_httpinfo);

/* cmd list */
char *cmd_list[ ] =
{
  (char*)cmd_insert,    "gosu-insert",
  (char*)cmd_include,   "gosu-include",
  (char*)cmd_if,        "gosu-if", 
  (char*)cmd_echo,      "gosu-echo",
  (char*)cmd_echo_env,  "gosu-echo-env",
  (char*)cmd_httpinfo,  "http-info",
  (char*)cmd_setenv,    "gosu-setenv",
  (char*)cmd_version,   "gosu-version",
  (char*)cmd_glist,     "gosu-list",
  (char*)cmd_diskinfo,  "disk-info"
};

/* uploads the command list to GoSu
 */
void gosucmd_init( void *plg )
{
  command *newcmd;
  pluginsinfo *plug = (pluginsinfo*) plg;
  
  int s = sizeof( cmd_list ) / ( sizeof( char* ) * 2 );
  int i;
  for( i = 0; i < s; i++ )
  {
    /* allocate memory */
    newcmd = (command*)malloc( sizeof( command ) );
    memset( newcmd, 0, sizeof( command ) );
    
    /* set command info */
    strncpy( newcmd->name, cmd_list[ i * 2 + 1 ], 63 );
    newcmd->proc = (int(*)(plugins*, lnklist*, void*))cmd_list[ i * 2 ];
                   
    /* add to command list */
    lnklist_add( plug->commands, newcmd );
  }
}

/* 
 * ------------------------------------------------------------
 *                        C O M M A N D S 
 * ------------------------------------------------------------
 */ 
 
/* gosu-insert */
PROTOTYPE(cmd_insert)
{
  int i;
  char *pnt;
  FILE *f;
  
  /* heh ? */
  if( !varlst_getvalue( params, "SRC" ) ) return 0;
  
  /* get real file name */
  i = _snprintf( tempbuf, 256, 
                 "%s",
                 varlst_getvalue( env, "REALFILENAME") );
                 
  /* get rid of file name */
  pnt = &tempbuf[ i - 1 ];
  while( *pnt != '\\' ) pnt--;
  *(pnt+1) = '\0';
  
  /* add file path */
  strncat( tempbuf, varlst_getvalue( params, "SRC" ), 255 );
  tempbuf[ 255 ] = '\0';
  
  /* change / to \ */
  pnt = tempbuf;
  while( *pnt )
  {
    if( *pnt == '/' ) *pnt = '\\';
    pnt++;
  }
  
  /* open file */
  f = fopen( tempbuf, "rb" );
  if( !f )
  {
    /* send error sth */
    OUTPROC(output)( "<B>Could not open file ", 23 );
    OUTPROC(output)( tempbuf, strlen( tempbuf ) );
    OUTPROC(output)( "!</B>", 5 );
    return 0; 
  }
  
  /* send teh file */
  do
  {
    i = fread( bigbuf, 1, BIGBUF_SIZE, f );
    if( i ) OUTPROC(output)( bigbuf, i );
  }
  while( i == BIGBUF_SIZE );
  
  fclose( f );
  return 0;
}

/* gosu-include */
PROTOTYPE(cmd_include)
{
  int i;
  char *pnt;
  FILE *f;
  char *lpbuf;
  srcparser *tempprs;
  
  /* heh ? */
  if( !varlst_getvalue( params, "SRC" ) ) return 0;
  
  /* get real file name */
  i = _snprintf( tempbuf, 256, 
                 "%s",
                 varlst_getvalue( env, "REALFILENAME") );
                 
  /* get rid of file name */
  pnt = &tempbuf[ i - 1 ];
  while( *pnt != '\\' ) pnt--;
  *(pnt+1) = '\0';
  
  /* add file path */
  strncat( tempbuf, varlst_getvalue( params, "SRC" ), 255 );
  tempbuf[ 255 ] = '\0';
  
  /* change / to \ */
  pnt = tempbuf;
  while( *pnt )
  {
    if( *pnt == '/' ) *pnt = '\\';
    pnt++;
  }
  
  /* open file */
  f = fopen( tempbuf, "rb" );
  if( !f )
  {
    /* send error sth */
    OUTPROC(output)( "<B>Could not open file ", 23 );
    OUTPROC(output)( tempbuf, strlen( tempbuf ) );
    OUTPROC(output)( "!</B>", 5 );
    return 0; 
  }
  
  /* get file size */
  fseek( f, 0, SEEK_END );
  i = ftell( f );
  fseek( f, 0, SEEK_SET );
  
  /* allocate memory */
  lpbuf = (char*)malloc( i );
  if( !lpbuf )
  {
    /* send error sth */
    OUTPROC(output)( "<B>Could not allocate memory!</B>", 33 );
    fclose( f );
    return 0;
  }
  
  /* read and close the file */
  fread( lpbuf, 1, i, f );  
  fclose( f );
  
  /* allocate parser */
  tempprs = srcparser_open( conf, plg );
  if( !tempprs )
  {
    /* send error sth */
    OUTPROC(output)( "<B>Could not allocate parser!</B>", 33 );
    free( lpbuf );
    return 0;    
  }
  
  srcparser_parse( tempprs, lpbuf, i, env, client, output );
  
  /* close and clean */
  srcparser_close( tempprs );
  free( lpbuf );  
  return 0;
}

/* gosu-if */
PROTOTYPE(cmd_if)
{
  const char *l, *r, *th, *el, *tp, *executeme;
  int itp = 0, what;
  srcparser *tempprs;
  
  /* get params */
  l = varlst_getvalue( params, "left");
  r = varlst_getvalue( params, "right");
  th = varlst_getvalue( params, "then");
  el = varlst_getvalue( params, "else");
  tp = varlst_getvalue( params, "insens");
 
  /* check if we have everything */
  if( !l || !r || !( th || el ) ) return 0;
  
  /* get type */
  if( tp ) itp = atoi( tp );
  
  /* compare */
  if( !itp )
  {
    if( strcmp( l, r ) == 0 ) what = 0;
    else what = 1;    
  }
  else
  {
    if( _stricmp( l, r ) == 0 ) what = 0;
    else what = 1;    
  }
  
  /* execute */
  if( what == 0 )
  {
    /* try to execute "then" */
    if( !th ) return 0;
    executeme = th;
  }
  else
  {
    /* try to execute "else" */
    if( !el ) return 0;
    executeme = el;
  }
  
  /* allocate parser */
  tempprs = srcparser_open( conf, plg );
  if( !tempprs )
  {
    /* send error sth */
    OUTPROC(output)( "<B>Could not allocate parser!</B>", 33 );
    return 0;    
  }
  
  /* execute */
  srcparser_parse( tempprs, executeme, strlen( executeme ), env, client, output );
  
  /* close and clean */
  srcparser_close( tempprs );
  
  return 0;
}

/* gosu-echo */
PROTOTYPE(cmd_echo)
{
  int i;
  
  i = _snprintf( tempbuf, 256, 
                 "%s",
                 varlst_getvalue( params, "text") );
  OUTPROC(output)( tempbuf, i );
  
  return 0;
}

/* gosu-echo */
PROTOTYPE(cmd_echo_env)
{
  const char *what, *where;
  
  where = varlst_getvalue( params, "name" );
  if( !where ) return 0;
  
  what = varlst_getvalue( env, where );
  if( !what ) return 0;
  
  OUTPROC(output)( what, strlen( what ) );
  
  return 0;
}

/* http-info */
PROTOTYPE(cmd_httpinfo)
{
  const char *what, *where;
  
  where = varlst_getvalue( params, "name" );
  if( !where ) return 0;
  
  what = varlst_getvalue( http, where );
  if( !what ) return 0;
  
  OUTPROC(output)( what, strlen( what ) );
  
  return 0;
}

/* gosu-setenv */
PROTOTYPE(cmd_setenv)
{
  const char *name, *value;
  
  name = varlst_getvalue( params, "name");
  value = varlst_getvalue( params, "value");
  
  /* do we have all ? */
  if( !name || !value ) return 0;
  
  /* set value */
  varlst_setvalue( env, name, value );
  
  return 0;
}

/* gosu-version */
PROTOTYPE(cmd_version)
{
  OUTPROC(output)( GoSu_version, strlen( GoSu_version ) );
  return 0;
}

/* gosu-version */
PROTOTYPE(cmd_glist)
{
  const char* what;
  pluginsinfo *plug = (pluginsinfo*) plg;
  plginfo *pinfo;
  command *cinfo;
  int i, j, ret;
  
  /* get 'what' */
  what = varlst_getvalue( params, "what" );
  if( !what )
  {
    OUTPROC(output)( "<b>Missing param!</b>\r\n", 21 );
    return 0;
  }
  
  /* what do i have to list */
  if( _stricmp( what, "plugins" ) == 0 )
  {
    /* list plugins */
    j = lnklist_count( plug->dlls );
    if( !j )
    {
      OUTPROC(output)( "No plugins loaded.\r\n", 20 );
      return 0;
    }

    lnklist_first( plug->dlls);
    for( i = 0; i < j; i++ )
    {
      pinfo = (plginfo*)lnklist_data( plug->dlls );
      ret = snprintf( tempbuf, 256, 
            "<b>%s</b> [<font style=\"color: blue;\">%s</font>]<br>\r\n",
            pinfo->filename,
            pinfo->name );
      OUTPROC(output)( tempbuf, ret );            
      lnklist_next( plug->dlls );
    }
  }
  else if( _stricmp( what, "functions" ) == 0 )
  {
    /* list functions */
    j = lnklist_count( plug->commands );
    if( !j )
    {
      /* HUH?! then what the hell is this ?! */
      OUTPROC(output)( "No functions.\r\n", 15 );
      return 0;
    }

    lnklist_first( plug->commands );
    for( i = 0; i < j; i++ )
    {
      cinfo = (command*)lnklist_data( plug->commands );
      ret = snprintf( tempbuf, 256, 
            "@<font style=\"color: blue;\">%s</font>\r\n",
            cinfo->name );
      OUTPROC(output)( tempbuf, ret );
      lnklist_next( plug->commands );
    }
  }
  else
  {
    OUTPROC(output)( "<b>Invalid param!</b>\r\n", 21 );
  }
  
  return 0;
}

PROTOTYPE(cmd_diskinfo)
{
  long unsigned int free, total;
  int ret;
  const char *dir, *what, *in;
  
  dir = varlst_getvalue( params, "disk" );
  what = varlst_getvalue( params, "what" );
  in = varlst_getvalue( params, "in" );
  
  if( !dir || !what ) 
  {
    OUTPROC(output)( "<b>Missing param!</b>\r\n", 21 );
    return 0;
  }
  
  if( !GetDiskFreeSpaceEx( dir, (void*)&free, (void*)&total, NULL ) )
  {
    OUTPROC(output)( "<b>Invalid param!</b>\r\n", 21 );
    return 0;
  }
  
  if( _stricmp( what, "free" ) == 0 )
  {
    if( in && ( _stricmp( in, "kb" ) == 0 ) )
    {
      free = free/1024;
    }
    else if( in && ( _stricmp( in, "mb" ) == 0 ) )
    {
      free = free/(1024*1024);
    }

    
    ret = snprintf( tempbuf, 256, "%lu", free );
  }
  else if( _stricmp( what, "total" ) == 0 )
  {
    if( in && ( _stricmp( in, "kb" ) == 0 ) )
    {
      total = total/1024;
    }
    else if( in && ( _stricmp( in, "mb" ) == 0 ) )
    {
      total = total/(1024*1024);
    }
    
    ret = snprintf( tempbuf, 256, "%lu", total );
  }
  else
  {
    ret = snprintf( tempbuf, 256, "<b>Invalid param!</b>\r\n" );
  }
  
  OUTPROC(output)( tempbuf, ret );  
  return 0;  
}

