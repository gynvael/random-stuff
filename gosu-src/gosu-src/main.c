/* GoSu
 * Ver.   : 0.1.6 (14-01-04)
 * Author : gynvael.coldwind//vx
 * Date   : 16.12.2003
 * Desc.  : http server + managment plugins
 */
#include<windows.h>
#include<stdio.h>
#include<string.h>
#include"config.h"
#include"varlst.h"
#include"linked_list.h"
#include"net.h"
#include"http.h"
#include"script_parser.h"
#include"plugins.h"
#include"string_ext.h"
#include"gosucmd.h"
#include"dsdb.h"
#include"memory.h"

/* defines
 */
#define MAIN_CONFIG "gosu.conf"
#define MAIN_Log "gosu.Log"
#define GOSU_RESTART -5
#define GOSU_SHUTDOWN -4
#define GOSU_VERSION "0.1.6"
#define NETSEND(a,b,c)  net_output( a, _snprintf( a, b, c ) )
#define PARSEBUF_SIZE 262144 /* 256 kb */
#define SENDFILE_PACKET_SIZE 1024 /* 1 kb */
int	_stricmp (const char*, const char*);
const char *GoSu_version = GOSU_VERSION" ("__TIME__" "__DATE__")";

/* media types 
 */
#define TYPE_GOSU       0 /* .gosu               * text/html  */
#define TYPE_TEXTHTML   1 /* .htm .html          * text/html  */
#define TYPE_TEXTPLAIN  2 /* .txt                * text/plain */
#define TYPE_IMAGEJPEG  3 /* .jpg .jpeg .jpe     * image/jpeg */
#define TYPE_IMAGEPNG   4 /* .png                * image/png  */
#define TYPE_IMAGEGIF   5 /* .gif                * image/gif  */
#define TYPE_IMAGEOTHER 6 /* .ico .bmp .tga .pcx * image/ *   */
#define TYPE_OTHER      7 /* .*                  * *//*       */

/* structures 
 */
enum
{
  /* listening socket (main) */
  GTSK_LISTEN,

  /* waiting for/reciving data */
  GTSK_READ,
  
  /* data to be parsed */
  GTSK_HTTP,
  
  /* send data from dsdb, then send file */
  GTSK_SENDING_BOTH,
 
  /* sending file */
  GTSK_SENDING_FILE,

  /* sending data from dsdb */
  GTSK_SENDING_DSDB,

  /* awaiting to be destroyed */
  GTSK_DESTROY
};

typedef struct gosutask_def
{
  int type;
  int mode;
  netsock *sock;
  dsdb *in;
  dsdb *data;
  FILE *file;
  unsigned int add;
} gosutask;

/* functions
 */
/* creates etherything! */
int gosu_init( void );

/* cleans etherything */
int gosu_clean( void );

/* main loop */
int gosu_main( void );
void handle_http( gosutask *tsk );
void dsdb_output( const char *data, int n );
int get_real_file( char *file, int n, FILE **f, int *ftype );
int get_file_type( const char *file );
void send_file( FILE *f );
void send_ok_header( dsdb *dt );
void send_notfound_header( dsdb *dt );
void send_ok_headerex( dsdb *dt, const char *content_type, int size );

/* globals
 */
config    *conf; /* gosu.conf */
plugins   *plug; /* plugins */
varlst    *env;  /* env values */
varlst    *http; /* http fields */
srcparser *prs;  /* main parser */
netsock   *msck; /* main socket */
lnklist   *csck; /* list of client sockets */
FILE      *Log;  /* main Log file */
char      *pbuf; /* parse buffer */
gosutask  *currtsk; /* current task */
int       state;

/* the main function
 */
int
main( int argc, char **argv )
{
  int ret;
  
  /* main program loop
   */
  do
  {
    /* try to init gosu
     */
    if( gosu_init( ) )
    {
      break;
    }
    fflush( Log );
    
    /* go to the main server loop
     */
    ret = gosu_main( );
  
    /* clean gosu
     */
    gosu_clean( );
  } 
  while( ret == GOSU_RESTART ); /* restart or exit */
  
  
  return 0;
}

/* gosu_init
 */
int 
gosu_init( void )
{
  int ret;
  int ip;
  short port;
  unsigned int pre;
  WSADATA wsdat;

  /* open Log file for appending in text mode */
  Log = fopen( MAIN_Log, "a" );
  if( !Log )
  {
    #ifdef DEBUG
    puts("error opening Log file!");
    #endif
    goto error;
  }
  fprintf( Log, "[%s] GoSu v"GOSU_VERSION" init started.\n", http_now( ) );
  fflush( Log );
  
  /* open and parse config */
  fprintf( Log, "[%s] allocating config... ", http_now( ) );
  fflush( Log );
  conf = config_open( MAIN_CONFIG );
  if( !conf )
  {
    #ifdef DEBUG
    puts("error allocating config!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  
  ret = config_parse( conf );
  if( ret == -1 )
  {
    #ifdef DEBUG
    puts("error parsing config!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  fflush( Log );
  
  /* pre-allocate DSDB segments */
  if( config_getstr( conf, "advanced", "preDSDBno" ) )
  {
    pre = atoi( config_getstr( conf, "advanced", "preDSDBno" ) );
  }
  else
  {
    pre = 10;
  }
  fprintf( Log, "[%s] preallocating %u DSDB segments... ", http_now( ), pre );
  fflush( Log );
  ret = dsdb_pre_alloc( pre );
  if( ret )
  {
    #ifdef DEBUG
    puts("error preallocating DSDB!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  fflush( Log );
    
  /* allocate plugins & commands */
  fprintf( Log, "[%s] loading plugins... ", http_now( ) );
  fflush( Log );
  plug = plugins_open( conf, "gosuscript" );
  if( !plug )
  {
    #ifdef DEBUG
    puts("error loading plugins!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  gosucmd_init( plug );
  fprintf( Log, "OK\n" );
  fflush( Log );
  
  /* allocate var lists */
  fprintf( Log, "[%s] allocating variable lists... ", http_now( ) );
  fflush( Log );
  env = varlst_create( );
  http = varlst_create( );
  if( !env || !http )
  {
    #ifdef DEBUG
    puts("error allocating varlst!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  fflush( Log );
  
  /* allocate main script parser */
  fprintf( Log, "[%s] allocating main parser... ", http_now( ) );
  fflush( Log );
  prs = srcparser_open( conf, plug );
  pbuf = (char*)malloc( PARSEBUF_SIZE );
  if( !prs || !pbuf )
  {
    #ifdef DEBUG
    puts("error allocating main parser!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  
  /* allocate client socket list */
  fprintf( Log, "[%s] allocating task list... ", http_now( ) );
  fflush( Log );
  csck = lnklist_create( );
  if( !csck )
  {
    #ifdef DEBUG
    puts("error allocating task list!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  fflush( Log );
  
  /* init winsock */
  fprintf( Log, "[%s] initializing winsock... ", http_now( ) );
  fflush( Log );
  if( WSAStartup( 0x0101, &wsdat ) )
	{
    #ifdef DEBUG
    puts("error initializing winsock!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  fprintf( Log, "[%s] -> %s\n[%s] -> %s\n", http_now( ),
           wsdat.szDescription, http_now( ), wsdat.szSystemStatus );
  fflush( Log );           

  /* create main socket */
  fprintf( Log, "[%s] creating main socket... ", http_now( ) );
  fflush( Log );
  ip = inet_addr( config_getstr( conf, "default", "listenip" ) );
  port = (short)atoi( config_getstr( conf, "default", "listenport" ) );
  msck = netsock_create( ip, port, NETSOCK_LISTEN );
  if( !msck )
 	{
    #ifdef DEBUG
    puts("error creating main socket!");
    #endif
    fprintf( Log, "ERROR\n" );
    goto error;
  }
  fprintf( Log, "OK\n" );
  fprintf( Log, "[%s] -> listening on %s:%u\n", http_now( ),
           config_getstr( conf, "default", "listenip" ),
           port );
  fflush( Log );           
  
  /* ok */
  fprintf( Log, "[%s] GoSu init finished succesfuly.\n", http_now( ) );  
  fflush( Log );
  return 0;

  /* on error */
error:
  if( Log  )  fclose( Log );
  if( plug )  plugins_close( plug );
  if( env  )  varlst_destroy( env );
  if( http )  varlst_destroy( http );
  if( prs  )  srcparser_close( prs );
  if( msck )  netsock_destroy( msck );
  if( csck )  lnklist_destroy( csck );
  if( conf )  config_close( conf );
  if( pbuf )  free( pbuf );
  dsdb_pre_free( );
  WSACleanup( ); 

  return -1;
}

int 
gosu_clean( void )
{
  int i, j, g = 0, k = 0, l = 0;
  gosutask *tsk;
  
  fprintf( Log, "[%s] GoSu shutdown procedure started.\n", http_now( ) );
  
  /* shut down tasks */
  fprintf( Log, "[%s] ending tasks...", http_now( ) );
  j = lnklist_count( csck );
  lnklist_first( csck );
  do
  {
    tsk = (gosutask*)lnklist_data( csck );
    if( !tsk ) continue;
    if( tsk->sock ) { netsock_destroy( tsk->sock ); l++; }
    if( tsk->in ) { dsdb_destroy( tsk->in ); k++; }
    if( tsk->data ) { dsdb_destroy( tsk->data ); k++; }
    if( tsk->file ) { fclose( tsk->file ); g++; }
    free( tsk );       
  }
  while( !lnklist_next( csck ) );
  lnklist_destroy( csck );
  WSACleanup( );
  fprintf( Log, "OK\n" );
  fprintf( Log, "[%s] -> %u sockets closed\n", http_now( ), l );
  fprintf( Log, "[%s] -> %u files closed\n", http_now( ), g );
  fprintf( Log, "[%s] -> %u DSDBs closed\n", http_now( ), k );
  fprintf( Log, "[%s] -> winsock closed\n", http_now( ) );
  fprintf( Log, "[%s] -> task list destroyed\n", http_now( ) );
  fflush( Log );
  
  /* close parser */
  fprintf( Log, "[%s] closing parser...", http_now( ) ); fflush( Log );
  srcparser_close( prs );
  free( pbuf );
  fprintf( Log, "OK\n" ); fflush( Log );
  
  /* destroy variable lists */
  fprintf( Log, "[%s] cleaning variable lists...", http_now( ) ); fflush( Log );
  varlst_destroy( env );
  varlst_destroy( http );
  fprintf( Log, "OK\n" ); fflush( Log );
  
  /* close plugins */  
  fprintf( Log, "[%s] closing plugins...", http_now( ) ); fflush( Log );
  plugins_close( plug );
  fprintf( Log, "OK\n" ); fflush( Log );
  
  /* free DSDB segments */  
  fprintf( Log, "[%s] freeing DSDB segments...", http_now( ) ); fflush( Log );
  i = dsdb_pre_free( );
  if( i )
  {
    fprintf( Log, "ERROR (%u segments lost)\n", i ); fflush( Log );
  }
  else
  { 
    fprintf( Log, "OK\n" ); fflush( Log );
  }
  
  /* close config */
  fprintf( Log, "[%s] freeing config...", http_now( ) ); fflush( Log );
  config_close( conf );
  fprintf( Log, "OK\n" ); fflush( Log );
  
  fprintf( Log, "[%s] GoSu shutdown finished.\n", http_now( ) );
  fclose( Log );
  
  /* clean */
  conf = plug = env = http = prs = msck = csck = NULL;
  Log = NULL;
  
  return 0;
}

/*
 * main server loop 
 */
int 
gosu_main( void )
{
  static char buffer[ 0x10000 ];
  char *sth;
  int i = 0, j = 0, ret;
  unsigned int uret;
  netsock *newconn;
  gosutask *tsk;
  int cno = 0; /* client no. */
  
  /* add the main socket to the list */
  tsk = (gosutask*) malloc( sizeof( gosutask ) );
  if( !tsk )
  {
    #ifdef DEBUG
    puts("not enough mem to alloc main task");
    #endif
    fprintf( Log, "[%s] couldn't alloc mem for main task\n", http_now( ) );
    fflush( Log );
    return GOSU_SHUTDOWN;
  }
  
  /* setup main task */
  tsk->type = GTSK_LISTEN;
  tsk->mode = NETSOCK_ASYNCHRONIC;
  tsk->sock = msck;
  tsk->in =
  tsk->data =
  tsk->file = NULL;
  
  /* add main task to list */
  lnklist_add( csck, tsk );
  tsk = NULL;
  
  /* reset list and state */    
  state = 0;
  lnklist_first( csck );
  
  while( 1 )
  {
    /* get current task */
    currtsk = (gosutask*)lnklist_data( csck );
    if( currtsk )
    {
      /* swiiiitchhee.. swiiitchee... 
       * (yeah, i know, to much Chobits ;p)
       */
      switch( currtsk->type )
      {
        /* listen to what i have to say */
        case GTSK_LISTEN:
          #ifdef DEBUG
          printf("GTSK_LISTEN\n");
          #endif
        
          /* check for socket mode */
          if( currtsk->mode == NETSOCK_SYNCHRONIC && cno )
          {
            netsock_setmode( currtsk->sock, NETSOCK_ASYNCHRONIC );
            currtsk->mode = NETSOCK_ASYNCHRONIC;
          }
          else if( currtsk->mode == NETSOCK_ASYNCHRONIC && !cno )
          {
            netsock_setmode( currtsk->sock, NETSOCK_SYNCHRONIC );
            currtsk->mode = NETSOCK_SYNCHRONIC;          
          }
        
          /* check for new connection */
          newconn = netsock_listen( currtsk->sock );
          if( !newconn ) 
          {
            #ifdef DEBUG
            puts(" (left break)");
            #endif
            break;
          }
          
          /* set asynchronic mode */
          netsock_setmode( newconn, NETSOCK_ASYNCHRONIC );
          
          /* Log the info */
          netsock_info( newconn, &i, &j );
          #ifdef DEBUG
            printf( "connection from %s:%u\n",
            inet_ntoa( *(struct in_addr*)&i ), 
            htons( j ) );
          #endif
          fprintf( Log, "[%s] connection from %s:%u\n", http_now( ),
                   inet_ntoa( *(struct in_addr*)&i ),
                   htons( j ) );
          fflush( Log );
          
          /* create new task */
          tsk = (gosutask*) malloc( sizeof( gosutask ) );
          if( !tsk )
          {
            #ifdef DEBUG
            puts("not enough mem to alloc new task");
            #endif
            fprintf( Log, "[%s] couldn't alloc mem for new task!\n", http_now( ) );
            fflush( Log );
            netsock_destroy( newconn );
            #ifdef DEBUG
            puts(" (left break)");
            #endif
            break;
          }
          
          /* setup task */
          tsk->type = GTSK_READ;
          tsk->mode = NETSOCK_ASYNCHRONIC;
          tsk->sock = newconn;
          tsk->in = dsdb_create( );
          tsk->data = dsdb_create( );
          tsk->file = NULL;
          tsk->add = 0;
          
          /* did everything go ok ? */
          if( !tsk->in || !tsk->data )
          {
            #ifdef DEBUG
            puts("could not allocate DSDB");
            #endif
            fprintf( Log, "[%s] could not allocate DSDB for new task!\n", http_now( ) );
            fflush( Log );
            if( tsk->in ) dsdb_destroy( tsk->in );
            if( tsk->data ) dsdb_destroy( tsk->data );
            netsock_destroy( newconn );
            free( tsk );
            #ifdef DEBUG
            puts(" (left break)");
            #endif
            break;
          }
          
          /* add task to list and inc client no. */
          lnklist_add( csck, tsk );
          tsk = NULL;
          cno++;
          #ifdef DEBUG
          puts(" (left)");
          #endif
        break;
        
        /* get data i'm sending you */
        case GTSK_READ:
          #ifdef DEBUG
          printf("GTSK_READ\n");
          #endif
          /* check if the sock is are in good mode 
           * (although it allways should be)
           */
          if( currtsk->mode != NETSOCK_ASYNCHRONIC )
          {
            netsock_setmode( currtsk->sock, NETSOCK_ASYNCHRONIC );
            currtsk->mode = NETSOCK_ASYNCHRONIC;
          }
        
          /* recive data */
          ret = netsock_read( currtsk->sock, buffer, 0x10000 );
          
          /* did it get any data ? */
          if( ret == -1 ) 
          {
            #ifdef DEBUG
            puts(" (left break)");
            #endif
            break;
          }
          
          /* did it disconnect ? */
          if( ret == 0 )
          {
            currtsk->type = GTSK_DESTROY;
            
            /* Log */
            #ifdef DEBUG
            printf( "client disconnected\n" );
            #endif
            fprintf( Log, "[%s] client disconnected [%u clients left]\n", 
                          http_now( ), cno );
            fflush( Log );
          }
          
          /* put data in the DSDB */
          dsdb_write( buffer, 1, ret, currtsk->in );
          
          /* check for the \r\n\r\n & POST sequence */
          uret = dsdb_datasize( currtsk->in );
          if( ( !currtsk->add ) && ( uret > 3 ) )
          {
            /* check if POST */
            dsdb_seek( currtsk->in, 0, DSEEK_SET );
            dsdb_read( buffer, 1, 4, currtsk->in );
            
            if( *((int*)buffer) == *((int*)"POST") )
            {
            
              if( uret > 0x10000 ) uret = 0x10000;
              dsdb_seek( currtsk->in, 0, DSEEK_SET );
              dsdb_read( buffer, 1, uret, currtsk->in );
              
              
              for( i = 0; (unsigned int)i < uret - 3; i++ )
              {
                if( *((int*)&buffer[i]) == *((int*)"\r\n\r\n") )
                {
                  break;
                }
              }
                
              if( (unsigned int)i < uret - 3 )
              {
                             
                /* get header size */
                currtsk->add = (unsigned int)i + 4;
                
                /* find content length */
                buffer[ currtsk->add ] = '\0';
                sth = strstr( buffer, "Content-Length:" );
                
                /* is it there ? */
                if( !sth )
                { 
                  /* quit if not */
                  currtsk->type = GTSK_DESTROY;
                }
                
                /* read Content-Length */
                sth += sizeof( "Content-Length: " ) - 1;
                if( sscanf( sth, "%u", &uret ) != 1 )
                {
                   /* quit if not */
                  currtsk->type = GTSK_DESTROY;
                }
                
                currtsk->add += uret;
              }
            }            
            else
            { 
              /* check for \r\n\r\n */
              
              /* get data */
              dsdb_seek( currtsk->in, 0, DSEEK_END );
              dsdb_seek( currtsk->in, -4, DSEEK_CUR );
              dsdb_read( buffer, 1, 4, currtsk->in );
            
              /* check */
              if( *((int*)buffer) == *((int*)"\r\n\r\n") )
              {
                currtsk->type = GTSK_HTTP;
              }
            }
          }
          
          /* check if post ended */
          if( currtsk->add && currtsk->add == dsdb_datasize( currtsk->in ) )
          {
            currtsk->type = GTSK_HTTP;
          }
      
          dsdb_seek( currtsk->in, 0, DSEEK_END );
               
          #ifdef DEBUG
          puts(" (left)");
          #endif     
        break;
        
        /* try to understand what i have to say */
        case GTSK_HTTP:
          #ifdef DEBUG
          printf("GTSK_HTTP\n");
          #endif
          /* parse http request and decide what to do next */
          handle_http( currtsk );
          dsdb_seek( currtsk->data, 0, DSEEK_SET );
          #ifdef DEBUG
          puts(" (left)");
          #endif
        break;
        
        /* send me the DSDB and then the file */
        case GTSK_SENDING_BOTH:
          #ifdef DEBUG
          printf("GTSK_SENDING_BOTH\n");
          #endif
          /* check if the sock is are in good mode */
          if( currtsk->mode != NETSOCK_SYNCHRONIC )
          {
            netsock_setmode( currtsk->sock, NETSOCK_SYNCHRONIC );
            currtsk->mode = NETSOCK_SYNCHRONIC;
          }
          
          /* get the packet and send it */
          ret = dsdb_read( buffer, 1, SENDFILE_PACKET_SIZE, currtsk->data );
          netsock_write( currtsk->sock, buffer, ret );
          
          /* was it the last one ? */
          if( ret != SENDFILE_PACKET_SIZE )
          {
            currtsk->type = GTSK_SENDING_FILE;
          }
          #ifdef DEBUG
          puts(" (left)");
          #endif
        break;
        
        /* send me that file! */
        case GTSK_SENDING_FILE:
          #ifdef DEBUG
          printf("GTSK_SENDING_FILE\n");
          #endif
          /* check if the sock is are in good mode
           * (although it allways should be)
           */
          if( currtsk->mode != NETSOCK_SYNCHRONIC )
          {
            netsock_setmode( currtsk->sock, NETSOCK_SYNCHRONIC );
            currtsk->mode = NETSOCK_SYNCHRONIC;
          }
          
          /* check for file */
          if( !currtsk->file )
          {
            currtsk->type = GTSK_DESTROY;
            #ifdef DEBUG
            puts(" (left break)");
            #endif
            break;
          }
          
          /* get the packet and send it */
          ret = fread( buffer, 1, SENDFILE_PACKET_SIZE, currtsk->file );
          netsock_write( currtsk->sock, buffer, ret );
          
          /* was it the las one ? */
          if( ret != SENDFILE_PACKET_SIZE )
          {
            currtsk->type = GTSK_DESTROY;
          }
          #ifdef DEBUG
          puts(" (left)");
          #endif
        break;
        
        /* i want the DSDB! */
        case GTSK_SENDING_DSDB:
          #ifdef DEBUG
          printf("GTSK_SENDING_DSDB\n");
          #endif
          /* check if the sock is are in good mode
           * (although it allways should be)
           */
          if( currtsk->mode != NETSOCK_SYNCHRONIC )
          {
            netsock_setmode( currtsk->sock, NETSOCK_SYNCHRONIC );
            currtsk->mode = NETSOCK_SYNCHRONIC;
          }
          
          /* get the packet and send it */
          ret = dsdb_read( buffer, 1, SENDFILE_PACKET_SIZE, currtsk->data );
          netsock_write( currtsk->sock, buffer, ret );
          
          /* was it the las one ? */
          if( ret != SENDFILE_PACKET_SIZE )
          {
            currtsk->type = GTSK_DESTROY;
          }
          #ifdef DEBUG
          puts(" (left)");
          #endif
        break;
        
        /* no.. noo.. what are you doing?! noooooo.. */
        case GTSK_DESTROY:
          #ifdef DEBUG
          printf("GTSK_DESTROY\n");
          #endif
          /* clean it */
          netsock_destroy( currtsk->sock );
          dsdb_destroy( currtsk->in );
          dsdb_destroy( currtsk->data );
          if( currtsk->file ) fclose( currtsk->file );
          free( currtsk );
          lnklist_del( csck );
          cno--; 
          #ifdef DEBUG
          puts(" (left)");
          #endif
        break;
      }
      
    }
    
    /* special stuff */
    if( cno )
    {
      Sleep( 0 ); /* try with 0 later */
    }
    
    if( state && !cno ) 
    {
      return state;
    }
    
    /* go to the next task */
    if( lnklist_next( csck ) ) lnklist_first( csck );
  }
  
  return 0;
}

void 
send_ok_header( dsdb *db )
{
  static char temp[ 512 ];
  const char *dt;
  int ret;
  
  /* get time
   */  
  dt = http_now( );
  
  /* format data
   */
  ret = _snprintf( temp, 512, 
        "HTTP/1.1 200 OK\r\n"
        "Date: %s\r\n"
        "Expires: %s\r\n"
        "Connection: close\r\n"
        "Server: GoSu/"GOSU_VERSION" (31337-OS)\r\n"
        "Content-Type: text/html\r\n"
        "\r\n", dt, dt );
        
  /* write data to dsdb
   */
  dsdb_write( temp, 1, ret, db );
}

void 
send_ok_headerex( dsdb *db, const char *content_type, int size )
{
  static char temp[ 512 ];
  const char *dt;
  int ret;
    
  /* get time
   */
  dt = http_now( );
  
  /* format data
   */
  ret = _snprintf( temp, 512, 
        "HTTP/1.1 200 OK\r\n"
        "Date: %s\r\n"
        "Connection: close\r\n"
        "Server: GoSu/"GOSU_VERSION" (31337-OS)\r\n"
        "Content-Length: %u\r\n"
        "Content-Type: %s\r\n"
        "\r\n", dt, size, content_type );
  
  /* write data to dsdb
   */
  dsdb_write( temp, 1, ret, db );
}

void 
send_notfound_header( dsdb *db )
{
  static char temp[ 512 ];
  const char *dt;
  int ret;
    
  /* get time
   */
  dt = http_now( );
  
  /* format data
   */
  ret = _snprintf( temp, 512, 
        "HTTP/1.1 404 Not Found\r\n"
        "Date: %s\r\n"
        "Expires: %s\r\n"
        "Connection: close\r\n"
        "Content-Type: text/html\r\n"
        "\r\n", dt, dt );
  
  /* write data to dsdb
   */
  dsdb_write( temp, 1, ret, db );
}

void 
handle_http( gosutask *tsk )
{
  int ret, i, j, ftype = 0;
  static char temp[ 256 ];
  char *tbuf;
  STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
  /* get info */
  netsock_info( tsk->sock, &i, &j );  
  
  /* parse the http request */
  ret = dsdb_datasize( tsk->in ) + 1;
  if( ret > PARSEBUF_SIZE ) 
  {
    /* not good! */
    tbuf = (char*)malloc( ret );
    if( !tbuf ) 
    {
      /* VERY BAD! */
      fprintf(Log, "[%s] could not parse request from %s:%u\n", http_now( ),
            inet_ntoa( *(struct in_addr*)&i ), htons( j ) );
      fprintf(Log, 
              "[%s] -> the parse buf was not enough and i"
              " couldn't allocate more memory!\n",
              http_now( ) );
      fflush( Log );
      
      /* set type */
      tsk->type = GTSK_DESTROY;
      goto end;
    }
  }
  else
  {
    tbuf = pbuf;
  }
  
  /* read data */
  dsdb_seek( tsk->in, 0, DSEEK_SET );
  dsdb_read( tbuf, 1, ret - 1, tsk->in );
  tbuf[ ret - 1 ] = '\0';
  ret = http_parse( tbuf, ret, http, env );

  
  /* free the buf if allocated */
  if( tbuf != pbuf ) free( tbuf );
  
  if( ret )
  {
    #ifdef DEBUG
    printf("http parse error ( %s )!\n", varlst_getvalue( http, "ERROR") );
    #endif
    fprintf(Log, "[%s] invalid http request from %s:%u\n", http_now( ),
            inet_ntoa( *(struct in_addr*)&i ), htons( j ) );
    fprintf(Log, "[%s] -> %s\n", http_now( ), varlst_getvalue( http, "ERROR") );
    fflush( Log );
    return;
  }
  
  /* add some env variables */
  varlst_setvalue( env, "CLIENT_IP", inet_ntoa( *(struct in_addr*)&i ) );
  sprintf( temp, "%u", htons( j ) );
  varlst_setvalue( env, "CLIENT_PORT", temp );
  
  /* Log */
  fprintf(Log, "[%s] client %s:%s requests %s with %s\n", http_now( ),
          varlst_getvalue( env, "CLIENT_IP" ),
          varlst_getvalue( env, "CLIENT_PORT" ),
          varlst_getvalue( http, "REQUEST" ),
          varlst_getvalue( http, "METHOD" ) );
  fflush( Log );
  
  /* check for special commands */
  if( strcmp( varlst_getvalue( http, "REQUEST" ), "/-servercmd") == 0 )
  {
    /* now can be set the new task type
     */
    tsk->type = GTSK_SENDING_DSDB;
    
    /* write data to header
     */
    send_ok_header( tsk->data );
    dsdb_puts( tsk->data, 
          "<html><title>GoSu "GOSU_VERSION"</title><body>\r\n" );

    if( varlst_getvalue( env, "SHUTDOWN" ) )
    {
        /* write body */
        dsdb_puts( tsk->data, 
          "<B>Executing Server Command:</B> SHUTDOWN\r\n" );
        
        /* Log */
        fprintf( Log, "[%s] Shutdown Request!\n", http_now( ) );
        fflush( Log );
        
        /* set state */
        state = GOSU_SHUTDOWN;
    }
    else if ( varlst_getvalue( env, "RESTART" ) )
    {
        /* write body */
        dsdb_puts( tsk->data, 
          "<B>Executing Server Command:</B> RESTART\r\n" );
        
        /* Log */
        fprintf( Log, "[%s] Restart Request!\n", http_now( ) );
        fflush( Log );
        
        /* set state */
        state = GOSU_RESTART;
    }
    else if ( varlst_getvalue( env, "EXECUTE" ) )
    {
        /* clean mem */
        memset( &si, 0, sizeof( si ) );
        si.cb = sizeof( si );
        
        /* try to create process */
        CreateProcess( varlst_getvalue( env, "EXECUTE" ), 
                       NULL, NULL, NULL, FALSE, 0, NULL, 
                       varlst_getvalue( env, "WORKDIR" ),
                       &si, &pi );
        
        /* write data */
        dsdb_puts( tsk->data, 
                   "<script language=\"javascript\">history.back(1);</script>" );
    }
    else
    {
        /* write data */
        dsdb_puts( tsk->data, "<B>Unknown Server Command!</B>\r\n" );
        
        /* Log */
        fprintf( Log, "[%s] Unknown Request!\n", http_now( ) );
        fflush( Log );
    }
    
    /* write data */
    dsdb_puts( tsk->data, "</body></html>\r\n\r\n" );
    goto end;
  }
  /* server logo requested ? */
  else if( strcmp( varlst_getvalue( http, "REQUEST" ), "/-gosulogo") == 0 )
  {
    tsk->file = fopen( "logo.png", "rb" );
    if( !tsk->file )
    {
      /* now can be set the new task type
       */
      tsk->type = GTSK_SENDING_DSDB;
      
      /* not found header */
      send_notfound_header( tsk->data );
      goto end;
    }
   
    /* now can be set the new task type
     */
    tsk->type = GTSK_SENDING_BOTH;
    
    /* get file size 
     */
    fseek( tsk->file, 0, SEEK_END );
    ret = ftell( tsk->file );
    fseek( tsk->file, 0, SEEK_SET );
    
    /* write header */
    send_ok_headerex( tsk->data, "image/png", ret );
    
    goto end;
    
  }  
  /* not any special stuff */
  else
  {
    ret = get_real_file( temp, 256, &tsk->file, &ftype );
    if( !ret )
    {
      /* now can be set the new task type
       */
      tsk->type = GTSK_SENDING_DSDB;
      
      /* not found header */
      send_notfound_header( tsk->data );
      dsdb_puts( tsk->data, "<html><title>GoSu "GOSU_VERSION"</title><body>\r\n" );
      dsdb_puts( tsk->data, "<B>Requested Site Not Found!</B>\r\n" );
      dsdb_puts( tsk->data, "</body></html>\r\n\r\n" );
      goto end;
    }
    
    /* set some env values */
    varlst_setvalue( env, "FILENAME", varlst_getvalue( http, "REQUEST" ) );
    varlst_setvalue( env, "REALFILENAME", temp );
    
    /* set type to both */
    tsk->type = GTSK_SENDING_BOTH;
    
    /* switchiii switchiii... */
    switch( ftype )
    {
      /* .gosu */
      case TYPE_GOSU:
        tsk->type = GTSK_SENDING_DSDB;
        send_ok_header( tsk->data );
        fread( pbuf, 1, ret, tsk->file );

        srcparser_parse( prs, pbuf, ret, env, http, (void*)dsdb_output );
        
        fclose( tsk->file );
        tsk->file = NULL;
      break;
      
      /* .html .htm */
      case TYPE_TEXTHTML:
        send_ok_headerex( tsk->data, "text/html", ret );
      break;
      
      /* .txt */
      case TYPE_TEXTPLAIN:
        send_ok_headerex( tsk->data, "text/plain", ret );
      break;
      
      /* .jpg .jpe .jpeg */
      case TYPE_IMAGEJPEG:
        send_ok_headerex( tsk->data, "image/jpeg", ret );
      break;
      
      /* .png */
      case TYPE_IMAGEPNG:
        send_ok_headerex( tsk->data, "image/png", ret );
      break;
      
      /* .gif */
      case TYPE_IMAGEGIF:
        send_ok_headerex( tsk->data, "image/gif", ret );
      break;      
      
      /* .pcx .ico etc */
      case TYPE_IMAGEOTHER:
        send_ok_headerex( tsk->data, "image/*", ret );
      break;                  
      
      /* .gif */
      case TYPE_OTHER:
        send_ok_headerex( tsk->data, "*/*", ret );
      break;      
    };
    
    /* send the file if not .gosu */
    /*if( ftype != TYPE_GOSU )
    {
      send_file( f );
    }*/
    
    /* close the file */
    /*fclose( f );*/
    
  }
  
end:

  
  varlst_clean( http );
  varlst_clean( env );
}

/* send data */
void 
dsdb_output( const char *data, int n )
{
  dsdb_seek( currtsk->data, 0, DSEEK_END );
  dsdb_write( data, 1, n, currtsk->data );
}

/* gets real file name and open the file */
int 
get_real_file( char *file, int n, FILE **f, int *ftype )
{
  static char temp[ 256 ];
  const char *uri;
  char *pnt;
  int ret;
  
  /* get uri */
  uri = varlst_getvalue( http, "REQUEST" );
  
  /* check for virtual dir */
  if( uri[ 1 ] == '~' )
  {
    uri+=2;
    pnt = temp;
    
    /* copy dir name */
    while( *uri && ( *uri != '/' ) ) *(pnt++) = *(uri++);
    if( !(*uri) ) 
    {
      return 0;
    }
    *pnt = '\0';
    
    /* do we have such a virtual dir ? */
    if( !config_getstr( conf, "VIRTUALS", temp ) ) 
    {
      return 0;
    }
        
    /* copy real name */
    ret = _snprintf( file, n, "%s", config_getstr( conf, "VIRTUALS", temp ) );
    pnt = &file[ ret ];
  }
  else
  {
    ret = _snprintf( file, n, "%s", config_getstr( conf, "DEFAULT", "HOMEDIR" ) );
    pnt = &file[ ret ];
  }
  
  /* copy name to namebuf */
  while( *uri )
  {
    if( *uri == '/' )
      *pnt = '\\';
    else
      *pnt = *uri;
    pnt++;
    uri++;
  }
  
  /* ended by filename or dirname ? */
  if( *(pnt-1) == '\\' )
  {
    /* we need to try some options */
    
    /* index.gosu */
    strcpy( pnt, "index.gosu" );
    *f = fopen( file, "rb" );
    if( *f ) goto nextstep;
    
    /* index.html */
    strcpy( pnt, "index.html" );
    *f = fopen( file, "rb" );
    if( *f ) goto nextstep;
    
    /* index.htm */
    strcpy( pnt, "index.htm" );
    *f = fopen( file, "rb" );
    if( *f ) goto nextstep;
    
    /* blah, no file, so sorry */
    return 0;
  }
  else
  {
    /* open the file */
    *pnt = '\0';
    *f = fopen( file, "rb" );
    if( !(*f) ) return 0;
  }
  
nextstep:

  /* get file size */
  fseek( *f, 0, SEEK_END );
  ret = ftell( *f );
  fseek( *f, 0, SEEK_SET );
  
  /* get file type */
  *ftype = get_file_type( file );
  
  
  return ret;
}

/* gets file extension and tryies to determin file size */
int
get_file_type( const char *file )
{
  static char ext[ 8 ];
  const char *where;
  char *pnt = &ext[ 6 ];
  int s, i;
  ext[ 7 ] = '\0';
  
  /* get *file size */
  s = strlen( file );
  where = &file[ s - 1 ];
  
  /* copy ext or first 5 chars */
  for( i = 0; i < 5; i++ )
  {
    /* if the file got open, the name must be ok
     * so no need to check for errors 
     */
    if( ( *where != '\\' ) && ( *where != '.' ) )
    {
      *(pnt--) = *(where--);    
    }
    else break;    
  }
  
  pnt++;
  
  /* try to figure out file type by extention */  
  if( *where != '.' ) return TYPE_OTHER;
  
  /* SUBJECT TO CHANGE! */
  
  /* TYPE_GOSU .gosu */
  if( _stricmp( pnt, "gosu" ) == 0 ) return TYPE_GOSU;
  
  /* TYPE_TEXTHTML .htm .html */
  if( ( _stricmp( pnt, "htm" ) == 0 ) ||
      ( _stricmp( pnt, "html" ) == 0 ) ) return TYPE_TEXTHTML;
      
  /* TYPE_TEXTPLAIN .txt */
  if( _stricmp( pnt, "txt" ) == 0 ) return TYPE_IMAGEOTHER;//TYPE_TEXTPLAIN;
    
  /* TYPE_IMAGEJPEG .jpg .jpeg .jpe */
  if( ( _stricmp( pnt, "jpg" ) == 0 ) ||
      ( _stricmp( pnt, "jpe" ) == 0 ) ||
      ( _stricmp( pnt, "jpeg" ) == 0 ) ) return TYPE_IMAGEJPEG;
  
  /* TYPE_IMAGEPNG .png */
  if( _stricmp( pnt, "png" ) == 0 ) return TYPE_IMAGEPNG;
  
  /* TYPE_IMAGEGIF .gif */
  if( _stricmp( pnt, "gif" ) == 0 ) return TYPE_IMAGEGIF;
  
  /* TYPE_IMAGEOTHER .ico .bmp .tga .pcx */
  if( ( _stricmp( pnt, "ico" ) == 0 ) ||
      ( _stricmp( pnt, "bmp" ) == 0 ) ||
      ( _stricmp( pnt, "tga" ) == 0 ) ||
      ( _stricmp( pnt, "pcx" ) == 0 ) ) return TYPE_IMAGEOTHER;
  
  /* TYPE_OTHER .* */
  return TYPE_OTHER;
}

