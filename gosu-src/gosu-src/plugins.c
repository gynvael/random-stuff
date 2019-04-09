/* Plugins Lib
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 09.12.2003
 * Desc.  : Plugin loader and handler
 */
#include<stdio.h>
#include<windows.h>
#include<string.h>
#include"string_ext.h"
#include"config.h"
#include"linked_list.h"
#include"memory.h"

/* types
 */
typedef void plugins;

typedef struct def_plginfo
{
  HINSTANCE handle;
  char filename[ 256 ];
  char name[ 128 ];
} plginfo;

typedef struct def_command
{
  char name[ 64 ];
  int (*proc)( plugins*, lnklist*, lnklist*, lnklist*, void* );
} command;

typedef struct def_pluginsinfo
{
  lnklist *dlls;
  lnklist *commands;
  config *cfg;
  char sect_name[ 32 ];
} pluginsinfo;

/* functions 
*/


/* open */
plugins* 
plugins_open( void *cfg, const char *section )
{
  pluginsinfo *new_plugins;
  char parsbuf[ 256 ];
  char dllname[ 256 ];
  plginfo *new_dll;
  char *partemp;
  char *pardll;
  const char *c_dir, *c_dlls;
  #ifdef DEBUG
  int i,j;  
  
  puts("plg: starting!");
  #endif
  
  /* is the data ok ? */
  if( !cfg ) return NULL;
  if( !section || !(*section) ) return NULL;
  
  /* allocate memory */
  new_plugins = (pluginsinfo*)malloc( sizeof( pluginsinfo ) );
  if( !new_plugins ) return NULL;
  memset( new_plugins, 0, sizeof( pluginsinfo ) );
  
  /* set up cfg file and section name */
  strncpy( new_plugins->sect_name, section, 31 );
  new_plugins->cfg = cfg;
  
  #ifdef DEBUG
  printf("plg %u\n", __LINE__);
  #endif
  
  /* create linked lists */
  new_plugins->dlls = lnklist_create( );
  if( !new_plugins->dlls )
  {
    #ifdef DEBUG
    puts( "plg: could not create linked list (1)" );
    #endif
    free( new_plugins );
    return NULL;
  }
  new_plugins->commands = lnklist_create ( );
  if( !new_plugins->commands )
  {
    #ifdef DEBUG
    puts( "plg: could not create linked list (2)" );
    #endif
    free( new_plugins );
    return NULL;
  }
  
  #ifdef DEBUG
  printf("plg %u\n", __LINE__);
  #endif

  
  /* get config items */
  c_dir = config_getstr( cfg, section, "plugindir" );
  c_dlls = config_getstr( cfg, section, "plugins" );
  if( !c_dir )
  {
    c_dir = ".";
  }
  
  /* was there the plugin list ? */
  if( !c_dlls ) 
  {
    #ifdef DEBUG
    puts( "plg: no plugins in the list!" );
    #endif  
    /* huh? ... nvm.. return */
    free( new_plugins );
    return new_plugins;
  }
  
  #ifdef DEBUG
  printf("plg %u\n", __LINE__);
  #endif
  
  /* copy c_dlls to a work buffer */
  memset( parsbuf, 0, 256 );
  strncpy( parsbuf, c_dlls, 255 );
  
  /* parse dll names and insert them into dlls list */
  partemp = parsbuf;
  
  while( *partemp )
  {
    #ifdef DEBUG
    printf("plg %u\n", __LINE__);
    #endif
    
    /* clear buffer, and reset pardll */
    memset( dllname, 0, 256 );
    pardll = dllname;
    
    /* jump over spaces and tabs at the begining */
    partemp = jump_over_blank( partemp );
  
    while( *partemp && ( *partemp != ';' ) 
           && ( *partemp != ' ' ) && ( *partemp != '\t' ) )
    {
      *(pardll++) = *(partemp++);
    }
    
    /* does the partemp point to ; or free space ? */
    if( *partemp == ';' )
    {
      /* jump over ; */
      partemp++;
    }
    else if( *partemp )
    {
      /* jump over free space */
      partemp = jump_over_blank( partemp );
      if( *partemp == ';' )
      {
        /* jump over ; */
        partemp++;
      }
    }
    
    #ifdef DEBUG
    printf("plg %u\n", __LINE__);
    #endif
  
    /* did we copy anything to the name buf ? */
    if( *dllname )
    {
      /* allocate memory */
      new_dll = (plginfo*)malloc( sizeof( plginfo ) );
      if( !new_dll ) continue;
      memset( new_dll, 0, sizeof( plginfo ) );
      
      /* copy filename */
      _snprintf( new_dll->filename, 255, "%s\\%s", c_dir, dllname );
      
      /* load lib */
      new_dll->handle = LoadLibrary( new_dll->filename );
      if( !new_dll->handle )
      {
        /* temp log */
        #ifdef DEBUG
        printf( "error loading plugin: %s\n", new_dll->filename );
        #endif
        
        free( new_dll );
        continue;
      }
      
      #ifdef DEBUG
      printf("plg %u\n", __LINE__);
      #endif
      
      /* download dll info to new_dll->name */
      ((void(*)(char*,int))GetProcAddress( new_dll->handle, "info" ))
      ( new_dll->name, 127 );
      
      #ifdef DEBUG
      printf("plg %u\n", __LINE__);
      #endif
      
      /* add dll to list */
      lnklist_add( new_plugins->dlls, new_dll );

      #ifdef DEBUG
      printf("plg %u\n", __LINE__);      
      #endif
      
      /* download plugin commands */
      ((void(*)(plugins*))GetProcAddress( new_dll->handle, "init" ))
      ( new_plugins );
            
      /* temp log */
      #ifdef DEBUG
      printf("plg %u\n", __LINE__);
      printf( "plugin loaded: (%s) \"%s\"\n", new_dll->filename, new_dll->name );
      #endif
    }
  }
  
  /* temp log */
  #ifdef DEBUG
  printf( "End of plugin init.\nPLUGINS LOADED:\n");
  j = lnklist_count( new_plugins->dlls );
  lnklist_first( new_plugins->dlls );
  for( i = 0; i < j; i++ )
  {
    puts( ((plginfo*)lnklist_data( new_plugins->dlls ))->filename );    
    lnklist_next( new_plugins->dlls );
  }
  
  printf( "COMMANDS LOADED:\n");
  j = lnklist_count( new_plugins->commands );
  lnklist_first( new_plugins->commands );
  for( i = 0; i < j; i++ )
  {
    puts( ((command*)lnklist_data( new_plugins->commands ))->name );    
    lnklist_next( new_plugins->commands );
  }
  #endif
  
  return new_plugins;
}

/* close */
void 
plugins_close( plugins *plg )
{
  int j, i;
  void *sth;
  pluginsinfo *plug = (pluginsinfo*)plg;
  
  /* are the data ok ? */
  if( !plug ) return;
  
  /* free the libs */
  j = lnklist_count( plug->dlls );
  lnklist_first( plug->dlls );
  for( i = 0; i < j; i++ )
  {
    /* free the lib */
    sth = lnklist_data( plug->dlls );
    FreeLibrary( ((plginfo*)sth)->handle );
    
    /* free the info */
    free( sth );
    
    /* jump to next lib */
    lnklist_next( plug->dlls );
  }
  
  /* free the commands */
  j = lnklist_count( plug->commands );
  lnklist_first( plug->commands );
  for( i = 0; i < j; i++ )
  {
    /* free command data */
    free( lnklist_data( plug->commands ) );
    
    /* jump to next command */
    lnklist_next( plug->commands );
  }
  
  /* free lists */
  lnklist_destroy( plug->dlls );
  lnklist_destroy( plug->commands );
  
  /* free info */
  free( plug );  
}

/* run */
int 
plugins_run( plugins *plg, 
             void *output, 
             const char *command_name, 
             void *lst_params,
             void *lst_env,
             void *lst_client )
{
  pluginsinfo *plug = (pluginsinfo*)plg;
  command *cmd;
  int i, j, ret;
  
  /* is there a command ? */
  if( !plug ) return -1;
  
  /* seek the command */
  j = lnklist_count( plug->commands );
  lnklist_first( plug->commands );
  for( i = 0; i < j; i++ )
  {
    /* get the current command */
    cmd = (command*)lnklist_data( plug->commands );
    
    /* is this the command we're looking for ? */
    if( strncmp( cmd->name, command_name, 63 ) == 0 )
    {
      /* execute the command */
      ret = cmd->proc( plg, lst_params, lst_env, lst_client, output );
      
      /* leave the proc */
      return ret;
    }
    
    /* jump to next cmd */
    lnklist_next( plug->commands );
  }
  
  return -1;  
}

