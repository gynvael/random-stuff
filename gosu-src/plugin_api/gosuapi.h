/* GoSu Plugin Api
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 09.12.2003
 * Desc.  : plugin api, just include this...
 *        : for more info on these func. look in the 
 *        : config.h and varlst.h
 */

#ifndef GYNV_PLUGIN_API
#define GYNV_PLUGIN_API

/* types */
typedef void plugins, varlst;
typedef struct def_pluginsinfo
{
  void *dlls;
  void *commands;
  void *cfg;
  char sect_name[ 32 ];
} pluginsinfo;

/* prototype:
 * int (*proc)( plugins*, lnklist*, void* );
 */
#define PROTOTYPE(a) int a( plugins *plg, varlst *params, varlst *env, varlst *client, void *output )
#define OUTPROC(a) ((void(*)(const char*,int))(a))

/* gosu api */
void gosuapi_addcmd( plugins *plg, const char *real_name, 
                     const char *use_name );

/* config */
const char* config_getstr( void *cfg, const char *sect_name, const char *item_name );

/* varlst */
const char* varlst_getvalue( varlst *vls, const char *itemname );
void varlst_setvalue( varlst *vls, const char *itemname, const char *value );
void varlst_setvalueex( varlst *vls, const char *itemname, char *data );


#endif

