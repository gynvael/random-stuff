/* Replace "dll.h" with the name of your header */
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"..\..\plugin_api\gosuapi.h"

/* global */
static char tempbuf[ 256 ];

/* plugin command list */
char *plugin_command[ ] =
{
  "cmd_date",      "basic-date",
  "cmd_time",      "basic-time",
  "cmd_paramtest", "basic-paramtest",
  "cmd_echo",      "basic-echo"
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
  _snprintf( buffer, n, "GoSu basic.dll" );
}

/* commands */
PROTOTYPE(cmd_date)
{
  _strdate( tempbuf );
  OUTPROC(output)( tempbuf, strlen( tempbuf ) );
  return 0;
}

PROTOTYPE(cmd_time)
{
  _strtime( tempbuf );
  OUTPROC(output)( tempbuf, strlen( tempbuf ) );
  return 0;
}

PROTOTYPE(cmd_paramtest)
{
  int i;
  
  i = _snprintf( tempbuf, 256, 
                 "<b>a</b> = <b>%s</b><BR>\n",
                 varlst_getvalue( params, "a") );
  OUTPROC(output)( tempbuf, i );
  
  i = _snprintf( tempbuf, 256, 
                 "<b>b</b> = <b>%s</b><BR>\n",
                 varlst_getvalue( params, "b") );
  OUTPROC(output)( tempbuf, i );
  
  i = _snprintf( tempbuf, 256, 
                 "<b>c</b> = <b>%s</b><BR>\n",
                 varlst_getvalue( params, "c") );
  OUTPROC(output)( tempbuf, i );  
  
  return 0;
}

PROTOTYPE(cmd_echo)
{
  int i;
  
  i = _snprintf( tempbuf, 256, 
                 "%s",
                 varlst_getvalue( params, "text") );
  OUTPROC(output)( tempbuf, i );
  
  return 0;
}


