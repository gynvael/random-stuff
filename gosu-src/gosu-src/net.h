/* TCP/IP Simple Network Module 
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 13.12.2003
 * Desc.  : like the sign says...
 */
#ifndef GYNV_NET
#define GYNV_NET

/* types
 */
typedef void netsock;

enum
{
  NETSOCK_LISTEN,
  NETSOCK_CONNECT
};

enum
{
  NETSOCK_SYNCHRONIC,
  NETSOCK_ASYNCHRONIC
};

/* functions
 */

/* creates socket and connects to/listens on a specified ip/port
 * returns: pointer
 *        : NULL on error
 */
netsock *netsock_create( unsigned long ip, unsigned short port, int type );

/* destroys socket / closes connection */
void netsock_destroy( netsock *sock );

/* sets socket mode (default is SYNCHRONIC) */
void netsock_setmode( netsock *sock, int mode );

/* checks for new connection on listen socket (works in specified mode) 
 * returns: pointer to client socket
 *        : NULL on no connections
 */
netsock *netsock_listen( netsock *sock );

/* reads data from socket
 * returns: data length on success
 *        : 0 on connection closed
 *        : -1 on no data in asynchronic mode
 *        : -2>= on error
 */
int netsock_read( netsock *sock, char *buffer, int n );

/* write data to socket */
void netsock_write( netsock *sock, char *buffer, int n);

/* get socket info 
 * returns values in big endian encoding!
 */
void netsock_info( netsock *sock, int *ip, int *port );

#endif

