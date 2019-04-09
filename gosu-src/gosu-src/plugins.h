/* Plugins Lib
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 09.12.2003
 * Desc.  : Plugin loader and handler
 */
#ifndef GYNV_PLUGINS
#define GYNV_PLUGINS

/* types
 */
typedef void plugins;

/* functions
 */
 
/* creates a new plugin handler, loads the plugins, etc
 * returns: pointer
 *        : NULL on error
 */
plugins* plugins_open( void *cfg, const char *section );

/* destroys plugin handles, unloads all the dll's and cleans all */
void plugins_close( plugins *plg );

/* runs the command from the plugin
 * returns: 0 on success
 *        : !0 on error
 */
int plugins_run( plugins *plg, void *output, 
                 const char *command_name, void *lst_params,
                 void *lst_env, void *lst_client );

#endif
