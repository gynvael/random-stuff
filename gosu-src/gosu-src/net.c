/* TCP/IP Simple Network Module 
 * Ver.   : 0.0.2
 * Author : gynvael.coldwind//vx
 * Date   : 13.12.2003
 * Desc.  : like the sign says...
 */
#include<winsock.h>
#include<stdio.h>
#include<string.h>
#include"memory.h"

/* types
 */
typedef void netsock;
typedef struct def_netinfo
{
  int sock;
  int ip;
  short mode;
  short type;
  short port;
} netinfo;

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

/* create */
netsock*
netsock_create( unsigned long ip, unsigned short port, int type )
{
  netinfo *ninfo;
  struct sockaddr_in sock_info;
  int ret, rt = 5;
  
  /* allocate memory */
  ninfo = (netinfo*)malloc( sizeof( netinfo ) );
  if( !ninfo )
  {
    return NULL;
  }  
  
  /* create socket */
  ninfo->sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if( ninfo->sock == INVALID_SOCKET )
  {
    free( ninfo );
    return NULL;
  }
  
  /* set socket info */
  sock_info.sin_family = AF_INET;
  ninfo->ip = sock_info.sin_addr.s_addr = ip;
  ninfo->port = sock_info.sin_port = htons( port );
  ninfo->mode = NETSOCK_SYNCHRONIC;
  ninfo->type = type;
   
  /* connect or listen */
  if( type == NETSOCK_CONNECT )
  {
    /* connect */
    ret = connect( ninfo->sock, (struct sockaddr*)&sock_info, 
                   sizeof( struct sockaddr ) );
    if( ret == SOCKET_ERROR )
    {
      /* close socket */
      closesocket( ninfo->sock );
      
      /* free mem */
      free( ninfo );
      return NULL;
    }
  }
  else
  { 
    /* ..and in darkness bind them.. ;p */
    while( 1 )
    {
      ret = bind( ninfo->sock, (struct sockaddr*)&sock_info,
                  sizeof( struct sockaddr ) );
      if( ret == SOCKET_ERROR && rt )
      {
        /* retry */
        Sleep( 500 );
        rt--;
      }
      else
      {
        break;
      }
    }
    
    /* did we have an error ? */    
    if( !rt )
    {
      printf("%p\r\n", (void*)WSAGetLastError( ) );
        
      /* close socket */
      closesocket( ninfo->sock );
      
      /* free mem */
      free( ninfo );
    }
        
    /* listen */
    ret = listen( ninfo->sock, SOMAXCONN );
    if( ret == SOCKET_ERROR )
    {
      printf("%p\r\n", (void*)WSAGetLastError( ) );
      
      /* close socket */
      closesocket( ninfo->sock );
      
      /* free mem */
      free( ninfo );
      return NULL;
    }
  }

  return ninfo;
}

/* destroy */
void 
netsock_destroy( netsock *sock )
{
  /* is there a sock ? */
  if( !sock ) return;
  
  /* clean up time */
  closesocket( ((netinfo*)sock)->sock );
  free( sock );
  }

/* set */
void netsock_setmode( netsock *sock, int mode )
{
  /* is there a sock ? */
  if( !sock ) return;
  
  /* check for current mode */
  if( ((netinfo*)sock)->mode == mode ) return;
  
  /* set the mode */
  ioctlsocket( ((netinfo*)sock)->sock, FIONBIO, (unsigned long*)&mode );
  ((netinfo*)sock)->mode = mode;
}

/* check */
netsock*
netsock_listen( netsock *sock )
{
  int newconn;
  netinfo *newsock;
  static struct sockaddr_in newaddr;
  static int addrsize = sizeof( struct sockaddr_in );
  
  /* is there a sock ? */
  if( !sock ) return NULL;
  
  /* check for conn */
  newconn = accept(((netinfo*)sock)->sock,
                   (struct sockaddr*)&newaddr,
                   &addrsize );
  
  /* check for mode */
  if( ((netinfo*)sock)->mode == NETSOCK_ASYNCHRONIC )
  {
    /* did something connect */
    if( newconn == -1 )
    { 
      return NULL;
    }
  }
  
  /* allocate mem */
  newsock = (netinfo*)malloc( sizeof( netinfo ) );
  if( !newsock )
  {
    closesocket( newconn );
    return NULL;
  }
  
  /* set info */
  newsock->sock = newconn;
  newsock->mode = NETSOCK_SYNCHRONIC;
  newsock->type = NETSOCK_CONNECT;
  newsock->ip = newaddr.sin_addr.s_addr;
  newsock->port = newaddr.sin_port;

  /* exit */
  return newsock;
}

/* reads data from socket
 * returns: data length on success
 *        : 0 on connection closed
 *        : -1 on no data in asynchronic mode
 *        : -2>= on error
 */
int 
netsock_read( netsock *sock, char *buffer, int n )
{
  int ret;
  
  /* is there a sock ? */
  if( !sock ) return 0;
  
  /* read data */
  ret  = recv( ((netinfo*)sock)->sock, buffer, n, 0 );
  return ret; 
}

/* write */
void 
netsock_write( netsock *sock, char *buffer, int n)
{
  /* is there a sock ? */
  if( !sock ) return;
  
  /* write data */
  send( ((netinfo*)sock)->sock, buffer, n, 0 );
}

/* info */
void 
netsock_info( netsock *sock, int *ip, int *port )
{
  /* is there a sock ? */
  if( !sock ) return;
  
  /* write the ip and port */
  *ip = ((netinfo*)sock)->ip;
  *port = ((netinfo*)sock)->port;
}

