/* GoSu API
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 09.12.2003
 * Desc.  : gosu api...
 */
#include<windows.h>
#include<string.h>
#include"../../gosu-src/linked_list.h"

typedef void config, plugins;

typedef struct def_pluginsinfo
{
  lnklist *dlls;
  lnklist *commands;
  config *cfg;
  char sect_name[ 32 ];
} pluginsinfo;

typedef struct def_command
{
  char name[ 64 ];
  int (*proc)( plugins*, lnklist*, lnklist*, lnklist*, void* );
} command;



void 
gosuapi_addcmd( plugins *plg, const char *real_name, 
                const char *use_name )
{
  command *newcmd;
  pluginsinfo *plug = (pluginsinfo*) plg;
  HINSTANCE hinst;
  
  /* get instance */
  lnklist_last( plug->dlls );
  hinst = *((HINSTANCE*)lnklist_data( plug->dlls ));
  
  /* allocate memory */
  newcmd = (command*)malloc( sizeof( command ) );
  memset( newcmd, 0, sizeof( command ) );
  
  /* set command info */
  strncpy( newcmd->name, use_name, 63 );
  newcmd->proc = (int(*)(plugins*, lnklist*, lnklist*, lnklist*, void*)) 
                 GetProcAddress( hinst, real_name );
                 
  /* add to command list */
  lnklist_add( plug->commands, newcmd );
}

