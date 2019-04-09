/* GoSu Script Parser
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 10.12.2003
 * Desc.  : like the sign says...
 */
#ifndef GYNV_SCRIPT_PARSER
#define GYNV_SCRIPT_PARSER

/* types
 */
typedef void srcparser;

/* functions
 */
 
/* creates parser info struct
 * returns: pointer
 *        : NULL on error
 */
srcparser* srcparser_open( void *cfg, void *plg );

/* destroys parser info struct, nothing unusual is done here */
void srcparser_close( srcparser *prs );

/* parses the script, output is done via the output proc 
 * prototype of the output proc:
 * void proc( char *data, int count );
 */
void srcparser_parse( srcparser *prs,
                      const char *data, int n,
                      void *env, void *client, void *output );

#endif

