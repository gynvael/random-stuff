/* Replace "dll.h" with the name of your header */
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"..\..\plugin_api\gosuapi.h"
#include<windows.h>

/* global */
static char tempbuf[ 256 ];

/* plugin command list */
char *plugin_command[ ] =
{
  "cmd_index",      "index",
};

/* uploads the plugin command list to GoSu
 * works good, but u can modify it if u want too
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
  _snprintf( buffer, n, "GoSu index.dll" );
}

/* commands */
PROTOTYPE(cmd_index)
{
  char buffer[1024];
  HANDLE          search;
  WIN32_FIND_DATA data;
  char *p;

  if(!varlst_getvalue(env, "REALFILENAME"))
    return 0;
  
  strcpy(buffer, varlst_getvalue(env, "REALFILENAME"));
  p = strrchr(buffer, '\\');
  if(!p) p = strrchr(buffer, '/');
  if(!p) 
    return 0;
  *p = '\0';
  strcat(p, "\\*");

  search = FindFirstFile( buffer, &data );
  if( search == INVALID_HANDLE_VALUE )
  {
    return 0;
  }
  
  do
  {
    if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {      
      _snprintf(buffer, 1024, "<a href=\"%s/\">[DIR] %s</a><br/>\n", data.cFileName, data.cFileName);
      OUTPROC(output)( buffer, strlen( buffer ) );
    }
    else
    {
      _snprintf(buffer, 1024, "<a href=\"%s\">[---] %s</a><br/>\n", data.cFileName, data.cFileName);
      OUTPROC(output)( buffer, strlen( buffer ) );
    }   

  }
  while( FindNextFile( search, &data ) );
  
  FindClose( search );
    
  
  return 0;
}


