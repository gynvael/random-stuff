/* GoSu Plug-In: Linkvault
 * Ver.   : 0.0.1
 * Author : gynvael.coldwind//vx
 * Date   : 12.01.2004
 * Desc.  : link vault plugin for GoSu http daemon
 */
#include<stdio.h>
#include<string.h>
#include"..\..\plugin_api\gosuapi.h"

/* defines */
#define GETVAL(a,b,c) if( !(a = varlst_getvalue( b, c ) ) ) a = "";

/* structs */
typedef struct def_lvfheader
{
  unsigned int sect_no; /* number of sections */
} lvfheader;

typedef struct def_lvfsection
{
  unsigned int index; /* index of section */
  unsigned int item_no; /* number of items in section */
  char name[ 64 ]; /* name of section */
} lvfsection;

typedef struct def_lvfitem
{
  char link[ 64 ]; /* link */
  char linkname[ 64 ]; /* link name */
  char desc[ 256 ]; /* description */
} lvfitem;

typedef struct def_lvsect
{
  unsigned int index; /* index of section */
  unsigned int item_no; /* number of items in section */
  char name[ 64 ]; /* name of section */
  lvfitem *items;
} lvsect;

typedef struct def_lvhead
{
  unsigned int sect_no; /* number of sections */
  lvsect *sections;
} lvhead;

/* functions */
void display_linkvault( varlst *params, varlst *env, varlst *client, void *output );
void display_addsection( varlst *params, varlst *env, varlst *client, void *output );
void display_editsection( varlst *params, varlst *env, varlst *client, void *output );

/* consts */
const char msk_chgsectname[ ] =
 "<br><center>\r\n"
 "<form action=\"%s?action=edit&todo=editname&section=%u&%s\" method=\"post\">\r\n"
 "New Section Name (max. 64):<BR>\r\n"
 "<input type=\"text\" name=\"sectname\" size=\"64\" class=\"lvinput\" value=\"%s\"/>"
 "<BR><BR>\r\n"
 "<input type=\"submit\" value=\"Change Section Name\" class=\"lvsubmit\"/>\r\n"
 "</form></center>\r\n";
 /* req, sect_no, adddata, value*/
 
const char msk_edititemdata[ ] =
 "<br><center>Edit item:<BR><BR>\r\n"
 "<form action=\"%s?action=edit&section=%u&itemno=%u&todo=edit&%s\" method=\"post\">\r\n"
 "Link (max. 64):<BR>\r\n"
 "http://<input type=\"text\" name=\"link\" size=\"64\" class=\"lvinput\" "
 "style=\"text-align: left\" value=\"%s\"/><BR><BR>\r\n"
 "Link Name (max. 64, if blank, Link is used as name):<BR>\r\n"
 "<input type=\"text\" name=\"linkname\" size=\"64\" class=\"lvinput\" value=\"%s\"/>"
 "<BR><BR>\r\nDescription (max. 256):<BR>\r\n"
 "<input type=\"text\" name=\"desc\" size=\"64\" class=\"lvinput\" value=\"%s\"/>"
 "<BR><BR>\r\n"
 "<input type=\"submit\" value=\"Change\" class=\"lvsubmit\"/>\r\n"
 "</form></center>\r\n";
 /* req, sect_no, item_no, adddata, link, linkname, desc */ 

const char msk_additemdata[ ] =
 "<br><center>Add new item:<BR><BR>\r\n"
 "<form action=\"%s?action=edit&section=%u&%s\" method=\"post\">\r\n"
 "Link (max. 64):<BR>\r\n"
 "http://<input type=\"text\" name=\"link\" size=\"64\" class=\"lvinput\" "
 "style=\"text-align: left\"/><BR><BR>\r\n"
 "Link Name (max. 64, if blank, Link is used as name):<BR>\r\n"
 "<input type=\"text\" name=\"linkname\" size=\"64\" class=\"lvinput\"/><BR><BR>\r\n"
 "Description (max. 256):<BR>\r\n"
 "<input type=\"text\" name=\"desc\" size=\"64\" class=\"lvinput\"/><BR><BR>\r\n"
 "<input type=\"submit\" value=\"Add Item\" class=\"lvsubmit\"/>\r\n"
 "</form></center>\r\n";
 /* req, sect_no, adddata */
 
const char msk_sectioned[ ] =
 "<table width=\"100%%\" cellspacing=\"0\">\r\n"
 "<tr><td class=\"lvsectleft\">\r\n"
 "%s\r\n"
 "</td><td width=\"40\" class=\"lvlinkitmtoolr\">\r\n"
 "<a href=\"%s?action=edit&todo=editname&section=%u&%s\">"
 "<img src=\"img/lv_edit.gif\" border=\"0\" width=\"20\" height=\"20\"></a>"
 "<a href=\"%s?action=delete&section=%u&%s\">"
 "<img src=\"img/lv_del.gif\" border=\"0\" width=\"20\" height=\"20\"></a>"
 "</td></tr></table><br>\r\n\r\n";
 /* sect_name, req, sect_no, adddata */ 
 
 const char msk_itemsected[ ] =
 "<table width=\"100%%\" class=\"lvlink\" cellspacing=\"0\"><tr>\r\n"
 "<td class=\"lvlinkitm\">"
 "<a href=\"%s?action=edit&todo=up&itemno=%u&section=%u&%s\">"
 "<img src=\"img/lv_up.gif\" class=\"lvlinkitmtooll\"></a></td>"
 "<td class=\"lvlinkitm\">"
 "<a href=\"%s?action=edit&todo=down&itemno=%u&section=%u&%s\">"
 "<img src=\"img/lv_down.gif\" class=\"lvlinkitmtoolc\"></a></td>"
 "<td class=\"lvlinkitm\">"
 "<a href=\"%s?action=edit&todo=edit&itemno=%u&section=%u&%s\">"
 "<img src=\"img/lv_edit.gif\" class=\"lvlinkitmtoolc\"></a></td>"
 "<td class=\"lvlinkitm\">"
 "<a href=\"%s?action=edit&todo=del&itemno=%u&section=%u&%s\">"
 "<img src=\"img/lv_del.gif\" class=\"lvlinkitmtoolr\"></a>"
 "</td>\r\n"
 "<td class=\"lvlinkleft\">%s</td>\r\n"
 "<td class=\"lvlinkright\">%s</td>\r\n"
 "</tr></table>\r\n\r\n";
 /* req, item_no, sect_no, adddata, sel_left, sel_right
  * req, item_no, sect_no, adddata, sel_left, sel_right
  * req, item_no, sect_no, adddata, sel_left, sel_right
  * req, item_no, sect_no, adddata, sel_left, sel_right
  * itemname, itemdesc
  */

const char msk_addsectdata[ ] =
 "<br><center>\r\n"
 "<form action=\"%s?action=addsection&%s\" method=\"post\">\r\n"
 "New Section Name (max. 64):<BR>\r\n"
 "<input type=\"text\" name=\"sectname\" size=\"64\" class=\"lvinput\"/><BR><BR>\r\n"
 "<input type=\"submit\" value=\"Add Section\" class=\"lvsubmit\"/>\r\n"
 "</form></center>\r\n";
 /* req, adddata */
 
const char msk_addsectret[ ] = 
 "<a href=\"%s?%s\" class=\"sth\">"
 "%sreturn to main linkvault page%s</a>"; 
 /* req, adddata, sel_left, sel_right */
 
const char msk_addsectok[ ] = 
 "<center>Section \"%s\" created succesfully!</center>";
 /* text */
 
const char msk_additemok[ ] = 
 "<center>Link \"%s\" added succesfully!</center><BR>";
 /* text */
 
const char msk_center[ ] = 
 "<center>%s</center>";
 /* text */

const char msk_addsect[ ] = 
 "<a href=\"%s?action=addsection&%s\" class=\"sth\">"
 "%sadd section%s</a>";
 /* req, adddata, sel_left, sel_right */
 
const char msk_section[ ] =
 "<table width=\"100%%\" cellspacing=\"0\">\r\n"
 "<tr><td class=\"lvsectleft\">\r\n"
 "%s\r\n"
 "</td><td width=\"60\" class=\"lvlinkitmtoolr\">\r\n"
 "<a href=\"%s?action=up&section=%u&%s\">"
 "<img src=\"img/lv_up.gif\" border=\"0\" width=\"20\" height=\"20\"></a>" 
 "<a href=\"%s?action=down&section=%u&%s\">"
 "<img src=\"img/lv_down.gif\" border=\"0\" width=\"20\" height=\"20\"></a>"
 "<a href=\"%s?action=edit&section=%u&%s\">"
 "<img src=\"img/lv_edit.gif\" border=\"0\" width=\"20\" height=\"20\"></a>"
 "</td></tr></table><br>\r\n\r\n";
 /* sect_name, req, sect_no, adddata */
 
const char msk_item[ ] =
 "<a href=\"http://%s\" class=\"sth\">\r\n"
 "%s<table width=\"100%%\" class=\"lvlink\"><tr>\r\n"
 "<td class=\"lvlinkleft\">%s</td>\r\n"
 "<td class=\"lvlinkright\">%s</td>\r\n"
 "</tr></table>%s</a>\r\n\r\n";
 /* link, sel_left, link_name, link_desc, sel_right */

const char txt_sectbreak[ ] =
 "<BR><BR>\r\n\r\n";
 
const char txt_nosrc[ ] =
 "No src param found. Please fill the src param with "
 "linkvault database file (it will be created if it "
 "doesn't exist. The path to this file starts at "
 "main GoSu directory.";

const char txt_srcerror[ ] =
 "Cannot access linkvault data file, or the data file "
 "is corrupted.";

/* buf */
static char buffer[ 2048 ];


/* cleans sections */
void
clean_sections( lvhead* head )
{
  unsigned int i;
  if( head->sect_no && head->sections )
  {
    for( i = 0; i < head->sect_no; i++ )
    {
      if( head->sections[ i ].items )
      {
        free( head->sections[ i ].items );
      }
    }
    free( head->sections );    
  }
  
  head->sect_no = 0;
  head->sections = NULL;  
}

/* save sections to file */
void
save_sections( const char *file, lvhead *lv, void *output )
{
  FILE *f;
  unsigned int i, j;
  
  /* write data into file */
  f = fopen( file, "wb" );
  if( !f )
  {
    OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
    return;
  }
  
  /* write in the lv struct */
  fwrite( lv, 1, sizeof( lvfheader ), f ); 
  for( i = 0; i < lv->sect_no; i++ )
  { 
    fwrite( &lv->sections[ i ], 1, sizeof( lvfsection ), f );
    for( j = 0; j < lv->sections[ i ].item_no; j++ )
    {
      fwrite( &lv->sections[ i ].items[ j ], 1, sizeof( lvfitem ), f );      
    }
  }

  /* done */
  fflush( f ); 
  fclose( f );
}

void
drop_sections( const char *file, lvhead *lv, void *output, 
               unsigned int sc, unsigned int it )
{
  FILE *f;
  unsigned int i, j;
  unsigned int sno, ino;
  
  /* write data into file */
  f = fopen( file, "wb" );
  if( !f )
  {
    OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
    return;
  }
  
  /* write in the lv struct */
  sno = lv->sect_no;
  if( sc != ~0 && it == ~0 )
  {
    lv->sect_no--;
  }
  
  fwrite( lv, 1, sizeof( lvfheader ), f ); 
  for( i = 0; i < sno; i++ )
  { 
    if( sc == i && it == ~0 ) continue;
    
    ino = lv->sections[ i ].item_no;
    if( sc == i && it != ~0 )
    {
      lv->sections[ i ].item_no--;
    }
    fwrite( &lv->sections[ i ], 1, sizeof( lvfsection ), f );
    for( j = 0; j < ino; j++ )
    {
      if( it == j && sc == i ) continue;
      fwrite( &lv->sections[ i ].items[ j ], 1, sizeof( lvfitem ), f );      
    }
  }

  /* done */
  fflush( f ); 
  fclose( f );
}


/* load sections from file */
lvhead*
load_sections( const char *file )
{
  FILE *f = NULL;
  static lvhead head;
  int ret;
  unsigned int i, j;
  
  head.sect_no = 0;
  head.sections = NULL;    
  
  /* try to open section file */
  f = fopen( file, "rb" );
  if( !f )
  {
    /* try to create file */
    f = fopen( file, "wb" );
    
    /* return with NULL if could not create */
    if( !f ) return NULL;
    
    /* write data */
    fwrite( &head, 1, sizeof( lvfheader ), f );
    fclose( f );
    return &head;    
  }
  
  /* read header */
  ret = fread( &head, 1, sizeof( lvfheader ), f );
  if( ret != sizeof( lvfheader ) )
  {
    goto error;
  }
  
  if( !head.sect_no ) 
  {
    /* nothing more to do */
    fclose( f );
    return &head;
  }
  
  /* allocate memory for sections */
  i = sizeof( lvsect ) * head.sect_no;
  head.sections = (lvsect*)malloc( i );
  if( !head.sections )
  {
    goto error;
  }
  
  memset( head.sections, 0, i );
  
  /* load sections */
  for( i = 0; i < head.sect_no; i++ )
  {
    /* read section header */
    ret = fread( &head.sections[ i ], 1, sizeof( lvfsection ), f );
    if( ret != sizeof( lvfsection ) )
    {
      goto error;
    }
    
    /* check if the section is empty */
    if( head.sections[ i ].item_no == 0 ) continue;
    
    /* allocate memory */
    j = sizeof( lvfitem ) * head.sections[ i ].item_no;
    head.sections[ i ].items = (lvfitem*)malloc( j );
    if( !head.sections[ i ].items )
    {
      goto error;
    }
    
    /* read items */
    ret = fread( head.sections[ i ].items, 1, j, f );
    if( ret != j )
    {
      goto error;
    }
  }  

  fclose( f );
  
  return &head;
  
/* on error */
error:
  fclose( f );
  if( head.sect_no && head.sections )
  {
    for( i = 0; i < head.sect_no; i++ )
    {
      if( head.sections[ i ].items )
      {
        free( head.sections[ i ].items );
      }
    }
    free( head.sections );    
  }
  return NULL;
}

/* Linkvault main function */
PROTOTYPE(linkvault)
{
  const char *action; /* action, what are we doing */
  const char *src; /* linkvault database file */
  const char *section; /* linkvault database file */
  unsigned int sec;
  lvhead *lv;  
  
  /* check for src */
  src = varlst_getvalue( params, "src" );
  if( !src )
  {
    OUTPROC(output)( txt_nosrc, sizeof( txt_nosrc ) - 1 );
    return 0;
  }
  
  /* get rest of values */
  section = varlst_getvalue( env, "section" );
  GETVAL( action, env, "action" );
  
  if( strcmp( action, "addsection" ) == 0 )
  {
    display_addsection( params, env, client, output );
  }
  else if( strcmp( action, "edit" ) == 0 )
  {
    display_editsection( params, env, client, output );
  }  
  else
  {
    if( strcmp( action, "delete" ) == 0 && section )
    {
      /* load list */
      lv = load_sections( src );
      if( !lv )
      {
        OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
        return;
      }
      
      sec = atoi( section );
      if( sec < lv->sect_no )
      {
        drop_sections( src, lv, output, sec, ~0 );
      }
      clean_sections( lv );
      
    }
    display_linkvault( params, env, client, output );
  }

  return 0;
}

/* link vault */
void 
display_linkvault( varlst *params, varlst *env, varlst *client, void *output )
{
  const char *req; /* the place where the link vault is (on www) */
  const char *adddata; /* additional get data */
  const char *sel_left, *sel_right; /* add selection info */
  const char *src; /* linkvault database file */
  const char *action;
  const char *section;
  lvhead *lv;
  int ret;
  unsigned int i, j, sec;
  static lvsect tempsect;
  
  /* get vals */
  src = varlst_getvalue( params, "src" );
  action = varlst_getvalue( env, "action" );
  section = varlst_getvalue( env, "section" );
  GETVAL( req, client, "request" );
  GETVAL( adddata, params, "adddata" );
  GETVAL( sel_left, env, "l" );
  GETVAL( sel_right, env, "r" );
  
  /* load list */
  lv = load_sections( src );
  if( !lv )
  {
    OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
    return;
  }
  
  /* check for up & downs */
  if( action && section )
  {
    sec = atoi( section );
    if( sec <= lv->sect_no )
    {
      if( strcmp( action, "up" ) == 0 && sec )
      {
        memcpy( &tempsect, &lv->sections[ sec ], sizeof( lvsect ) );
        memcpy( &lv->sections[ sec ], &lv->sections[ sec - 1 ], sizeof( lvsect ) );
        memcpy( &lv->sections[ sec - 1 ], &tempsect, sizeof( lvsect ) );
        save_sections( src, lv, output );
      }
      else if( strcmp( action, "down" ) == 0 && ( sec < lv->sect_no - 1 ) )
      {
        memcpy( &tempsect, &lv->sections[ sec ], sizeof( lvsect ) );
        memcpy( &lv->sections[ sec ], &lv->sections[ sec + 1 ], sizeof( lvsect ) );
        memcpy( &lv->sections[ sec + 1 ], &tempsect, sizeof( lvsect ) );
        save_sections( src, lv, output );
      }
    }
  }
  
  /* enum sections and links */
  for( i = 0; i < lv->sect_no; i++ )
  {
    /* display section header */
    ret = _snprintf( buffer, 2048, msk_section,
    lv->sections[ i ].name, req, i, adddata, req, i, adddata, req, i, adddata );
    OUTPROC(output)( buffer, ret );
    
    /* enum items */
    for( j = 0; j < lv->sections[ i ].item_no; j++ )
    {
      ret = _snprintf( buffer, 1024, msk_item,
      lv->sections[ i ].items[ j ].link,
      sel_left,
      lv->sections[ i ].items[ j ].linkname,
      lv->sections[ i ].items[ j ].desc,
      sel_right );
      OUTPROC(output)( buffer, ret );
    }
    
    /* section break */
    OUTPROC(output)( txt_sectbreak, sizeof( txt_sectbreak ) - 1 );
  }
  
  /* display "add section" */
  ret = _snprintf( buffer, 1024, msk_addsect,
                   req, adddata, sel_left, sel_right );
  OUTPROC( output )( buffer, ret );
  
  clean_sections( lv );  
}

/* add section */
void 
display_addsection( varlst *params, varlst *env, varlst *client, void *output )
{
  const char *req; /* the place where the link vault is (on www) */
  const char *adddata; /* additional get data */
  const char *sel_left, *sel_right; /* add selection info */
  const char *src; /* linkvault database file */
  const char *sectname; /* new section name */
  lvhead *lv;
  static lvfsection nsect;
  int ret;
  unsigned int i, j;
  FILE *f;
  
  /* get vals */
  src = varlst_getvalue( params, "src" );
  sectname = varlst_getvalue( env, "sectname" );
  GETVAL( req, client, "request" );
  GETVAL( adddata, params, "adddata" );
  GETVAL( sel_left, env, "l" );
  GETVAL( sel_right, env, "r" );
  
  /* display form or add section ? */
  if( !sectname )
  {
    /* display form */
    ret = _snprintf( buffer, 1024, msk_addsectdata,
    req, adddata );
    OUTPROC(output)( buffer, ret );
  }
  else
  {
    /* load list */
    lv = load_sections( src );
    if( !lv )
    {
      OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
      return;
    }
    
    /* clear new list */
    memset( &nsect, 0, sizeof( lvfsection ) );
    
    /* write in data */
    nsect.index = lv->sect_no++;
    strncpy( nsect.name, sectname, 64 );
    
    /* write data into file */
    f = fopen( src, "wb" );
    if( !f )
    {
      OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
      clean_sections( lv ); 
      return;
    }
    
    /* write in the lv struct */
    fwrite( lv, 1, sizeof( lvfheader ), f ); 
    for( i = 0; i < lv->sect_no - 1; i++ )
    { 
      fwrite( &lv->sections[ i ], 1, sizeof( lvfsection ), f );
      for( j = 0; j < lv->sections[ i ].item_no; j++ )
      {
        fwrite( &lv->sections[ i ].items[ j ], 1, sizeof( lvfitem ), f );      
      }
    }
    
    /* write in new section */
    fwrite( &nsect, 1, sizeof( lvfsection ), f ); 
    
    /* done */
    fflush( f ); 
    fclose( f );
    clean_sections( lv ); 
    
    ret = _snprintf( buffer, 1024, msk_addsectok,
    sectname );
    OUTPROC(output)( buffer, ret );
  }
    
  /* display "return..." */
  ret = _snprintf( buffer, 1024, msk_addsectret,
                   req, adddata, sel_left, sel_right );
  OUTPROC( output )( buffer, ret );
  
}

/* edit section */
void 
display_editsection( varlst *params, varlst *env, varlst *client, void *output )
{
  const char *req; /* the place where the link vault is (on www) */
  const char *adddata; /* additional get data */
  const char *sel_left, *sel_right; /* add selection info */
  const char *src; /* linkvault database file */
  const char *link, *linkname, *desc; /* guess */
  const char *section; /* section */
  const char *sectname; /* section */
  unsigned int sec;    /* same here */
  const char *itemno; /* item no */
  unsigned int no;    /* same here */
  const char *todo; /* do... */
  lvhead *lv;
  int ret;
  unsigned int i, j;
  static lvfitem nitem;
  FILE *f;
  
  /* get vals */
  src = varlst_getvalue( params, "src" );
  section = varlst_getvalue( env, "section" );
  todo = varlst_getvalue( env, "todo" );
  itemno = varlst_getvalue( env, "itemno" );
  sectname = varlst_getvalue( env, "sectname" );
  GETVAL( req, client, "request" );
  GETVAL( adddata, params, "adddata" );
  GETVAL( sel_left, env, "l" );
  GETVAL( sel_right, env, "r" );
  GETVAL( linkname, env, "linkname" );
  GETVAL( desc, env, "desc" );
  link = varlst_getvalue( env, "link" );

  if( !section )
  {
    /* display "return..." */
    ret = _snprintf( buffer, 1024, msk_addsectret,
                     req, adddata, sel_left, sel_right );
    OUTPROC( output )( buffer, ret );
    return;   
  }
  
  /* load list */
  lv = load_sections( src );
  if( !lv )
  {
    OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
    return;
  }  
 
  sscanf( section, "%u", &sec );
  if( sec >= lv->sect_no ) return;
  
  /* check for up and down */
  if( todo && itemno )
  {
    sscanf( itemno, "%u", &no );
    if( no < lv->sections[ sec ].item_no )
    {
      if( ( strcmp( todo, "up" ) == 0 ) && no )
      {
        memcpy( &nitem, &lv->sections[ sec ].items[ no ], sizeof( lvfitem ) );
        memcpy( &lv->sections[ sec ].items[ no ], 
                &lv->sections[ sec ].items[ no - 1 ], sizeof( lvfitem ) );
        memcpy( &lv->sections[ sec ].items[ no - 1 ], &nitem, sizeof( lvfitem ) );
        save_sections( src, lv, output );
      }
      else if( ( strcmp( todo, "down" ) == 0 ) &&
               ( no != lv->sections[ sec ].item_no - 1 ) )
      {
        memcpy( &nitem, &lv->sections[ sec ].items[ no ], sizeof( lvfitem ) );
        memcpy( &lv->sections[ sec ].items[ no ], 
                &lv->sections[ sec ].items[ no + 1 ], sizeof( lvfitem ) );
        memcpy( &lv->sections[ sec ].items[ no + 1 ], &nitem, sizeof( lvfitem ) );
        save_sections( src, lv, output );
      }
      else if( strcmp( todo, "del" ) == 0 &&
              ( sec < lv->sect_no ) ) 
      {
        drop_sections( src, lv, output, sec, no );
        clean_sections( lv ); 
        lv = load_sections( src );
        if( !lv )
        {
           OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
           return;
        }
      }
      else if( strcmp( todo, "edit" ) == 0 &&
              ( sec < lv->sect_no ) ) 
      {
        if( !link )
        {
          ret = _snprintf( buffer, 2048, msk_edititemdata,
                   req, sec, no, adddata,
                   lv->sections[ sec ].items[ no ].link,
                   lv->sections[ sec ].items[ no ].linkname,
                   lv->sections[ sec ].items[ no ].desc );
          OUTPROC( output )( buffer, ret );
          return;
        }
        else
        {
          memset( &lv->sections[ sec ].items[ no ], 0, sizeof( lvfitem ) );
          if( ! (*linkname) ) linkname = link;
          strncpy( lv->sections[ sec ].items[ no ].link, link, 64 );
          strncpy( lv->sections[ sec ].items[ no ].linkname, linkname, 64 );
          strncpy( lv->sections[ sec ].items[ no ].desc, desc, 256 );
          save_sections( src, lv, output );
        }
      }
    }
  } else if( todo )
  {
    if( strcmp( todo, "editname" ) == 0 &&
              ( sec < lv->sect_no ) ) 
    {
      if( sectname )
      {
        strncpy( lv->sections[ sec ].name, sectname, 64 ) ;
        save_sections( src, lv, output );
      }
      else
      {
        ret = _snprintf( buffer, 2048, msk_chgsectname,
                   req, sec, adddata, lv->sections[ sec ].name );
        OUTPROC( output )( buffer, ret );
        return;
      }
    }
  }

  /* add item ? */
  if( link && !todo )
  {
    /* setup item */
    memset( &nitem, 0, sizeof( lvfitem ) );
    if( ! (*linkname) ) linkname = link;
    strncpy( nitem.link, link, 64 );
    strncpy( nitem.linkname, linkname, 64 );
    strncpy( nitem.desc, desc, 256 );
    
    /* write file */
    f = fopen( src, "wb" );
    if( !f )
    {
      OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
      clean_sections( lv ); 
      return;
    }
    
    /* write in the lv struct */
    fwrite( lv, 1, sizeof( lvfheader ), f ); 
    for( i = 0; i < lv->sect_no; i++ )
    { 
      if( i != sec )
      {      
        fwrite( &lv->sections[ i ], 1, sizeof( lvfsection ), f );
        for( j = 0; j < lv->sections[ i ].item_no; j++ )
        {
          fwrite( &lv->sections[ i ].items[ j ], 1, sizeof( lvfitem ), f );      
        }
      }
      else
      {
        lv->sections[ i ].item_no++;
        fwrite( &lv->sections[ i ], 1, sizeof( lvfsection ), f );
        for( j = 0; j < lv->sections[ i ].item_no - 1; j++ )
        {
          fwrite( &lv->sections[ i ].items[ j ], 1, sizeof( lvfitem ), f );
        }
        fwrite( &nitem, 1, sizeof( lvfitem ), f );      
      }
    }

    /* done */
    fflush( f ); 
    fclose( f );
  
    /* reload list */
    clean_sections( lv ); 
    lv = load_sections( src );
    if( !lv )
    {
      OUTPROC(output)( txt_srcerror, sizeof( txt_srcerror ) - 1 );
      return;
    }
    
    /* ok */
    ret = _snprintf( buffer, 1024, msk_additemok, link );
    OUTPROC(output)( buffer, ret );
  }
  
  /* display section header */
  ret = _snprintf( buffer, 1024, msk_sectioned,
  lv->sections[ sec ].name, req, sec, adddata, req, sec, adddata);
  OUTPROC(output)( buffer, ret );
  
  /* enum items */
  for( j = 0; j < lv->sections[ sec ].item_no; j++ )
  {
    ret = _snprintf( buffer, 2048, msk_itemsected,
    req, j, sec, adddata,
    req, j, sec, adddata,
    req, j, sec, adddata,
    req, j, sec, adddata,
    lv->sections[ sec ].items[ j ].linkname,
    lv->sections[ sec ].items[ j ].desc );
    OUTPROC(output)( buffer, ret );
     /* req, item_no, sect_no, adddata, sel_left, sel_right
  * req, item_no, sect_no, adddata, sel_left, sel_right
  * req, item_no, sect_no, adddata, sel_left, sel_right
  * itemname, itemdesc
  */
  }
  
  /* section break */
  OUTPROC(output)( txt_sectbreak, sizeof( txt_sectbreak ) - 1 );
  
  /* display form */
  ret = _snprintf( buffer, 1024, msk_additemdata,
  req, sec, adddata );
  OUTPROC(output)( buffer, ret );
  
  /* display "return..." */
  ret = _snprintf( buffer, 1024, msk_addsectret,
                   req, adddata, sel_left, sel_right );
  OUTPROC( output )( buffer, ret );
  
  clean_sections( lv ); 
  
}



/* uploads the plugin command list to GoSu
 */
void init( plugins *plg )
{
  gosuapi_addcmd( plg, "linkvault", "linkvault" );
}

/* info stuff */
void info( char *buffer, int n )
{
  _snprintf( buffer, n, "GoSu Linkvault" );
}



