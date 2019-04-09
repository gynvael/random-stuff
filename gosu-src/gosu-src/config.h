/* Config Lib
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 08.12.2003
 * Desc.  : simple config parser + stuff
 */
#ifndef GYNV_CONFIG
#define GYNV_CONFIG

/* types 
 */
typedef void config;

/* functions
 */

/* creates config and assosiates a file with it (the file MUST exist)
 * returns: pointer
 *        : NULL on error 
 */
config* config_open( const char *filename );

/* closes the config (destroys all data) */
void config_close( config *cfg );

/* parses the config, filling all the data
 * returns: number of successfuly loaded items
 *        : -1 on error
 */
int config_parse( config *cfg );

/* returns a pointer to data from the specified item
 * returns: item data (string)
 */
const char* config_getstr( config *cfg, const char *sect_name, const char *item_name );

#endif 


