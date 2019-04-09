/* HTTP 1.1 praser
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 15.12.2003
 * Desc.  : http parser, nothing else
 */
#ifndef GYNV_HTTP
#define GYNV_HTTP

/* functions
 */
 
/* fills out the varlst http and env, adds some items like:
 *  method (OPTIONS/GET/POST/HEAD/etc)
 *  request (file requested)
 *  http (version of http)
 *  error (filled only on error with error type)
 * returns: 0
 *        : !0 on error
 */
int http_parse( const char *what, int n, void *http, void *env );

/* returns pointer to static buffer with current date in rfc1123-date format
 * returns: pointer
 */
const char* http_now( void );

#endif

