/* Dynamic Segment Data Bank
 * Ver.   : 0.0.3
 * Author : gynvael.coldwind//vx
 * Date   : 05-01-04
 * Desc.  : see doc/dsdb/index.html for desc
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"dsdb_config.h"
#include"memory.h"

/*
 * #define EXTREAM_DEBUG
 * #define EXTREAMS_DEBUG 
 * #define EXTREAMR_DEBUG
 * #define EXTREAMC_DEBUG
 * #define MEGA_DEBUG
 */


/* enums
 */
enum
{
  DSEG_NORMAL,
  DSEG_PREALLOC,
  DSEG_FASTALLOC
};

enum
{
  DSEG_NONE,
  DSEG_EXIST
};

/* segment def
 */
typedef struct dsdb_segment_def
{
  int type;
  unsigned int size;
  unsigned int free;
  struct dsdb_segment_def *next, *prev;
  char *data;  
} dsdb_segment;

/* offset def
 */
typedef struct dsdb_offset_def
{
  dsdb_segment *seg;
  unsigned int seg_offset;
  unsigned int offset;
} dsdb_offset;

/* dsdb struct def
 */
typedef struct dsdb_def
{
  unsigned int seg_size;
  unsigned int seg_count;
  unsigned int data_size;
  unsigned int mem_usage;
  dsdb_offset start;
  dsdb_offset end;
  dsdb_offset current;
} dsdb;

/* preallocated segments list
 */
typedef struct dsdb_prelst_def
{
  unsigned int count;
  unsigned int maxcount;
  unsigned int memusage;
  dsdb_segment *first;
  dsdb_segment *last;  
} dsdb_prelst;

/* dsdb global 
 */
typedef struct dsdb_global_def
{
  dsdb_prelst pre;
  unsigned int memusage;
} dsdb_global;

/* globals 
 */
dsdb_global dsdb_info;

/* 
 * functions (PRIVATE)
 */
dsdb_segment* 
dsdb_allocate_segment( unsigned int seg_size )
{
  dsdb_segment *seg;
  
  /* can we get segment from pre- list ? 
   */
  if( ( seg_size == DSDB_DEFAULT_SEG_SIZE ) &&
      ( dsdb_info.pre.count > 0 ) )
  {
    /* get the segment from pre- list 
     */
    seg = dsdb_info.pre.first;
    dsdb_info.pre.count--;
    dsdb_info.pre.first = dsdb_info.pre.first->next;
    #ifdef EXTREAM_DEBUG
    printf("->ALLOC SEGMENT (PRE-) prelistcnt: %u max: %u\n",
    dsdb_info.pre.count, dsdb_info.pre.maxcount);
    #endif
  }
  else
  {
    #ifdef EXTREAM_DEBUG
    puts("->ALLOC SEGMENT");
    #endif
    /* allocate new segment
     */
    seg = (dsdb_segment*)malloc( sizeof( dsdb_segment ) + seg_size );
    if( !seg ) return NULL;
    
    memset( seg, 0, sizeof( dsdb_segment ) );
    
    seg->size = seg->free = seg_size;
    seg->data = (char*)( ((char*)seg) + sizeof( dsdb_segment ) );
    
    dsdb_info.memusage += sizeof( dsdb_segment ) + seg_size;
  }
  
  return seg;
}

void
dsdb_free_segment( dsdb_segment *seg )
{
  /* check
   */
  if( !seg ) return;
  
  /* do we move or free the segment ? */
  if( seg->type == DSEG_PREALLOC )
  {
    /* move the segment back to pre- list
     */
    #ifdef EXTRA_SAFE
    if( dsdb_info.pre.maxcount == 0 ) return; /* HUH?! this sould never happen */
    #endif
    if( dsdb_info.pre.count == 0 )
    {
      dsdb_info.pre.first = dsdb_info.pre.last = seg;
    }
    else
    {
      dsdb_info.pre.last->next = seg;
      dsdb_info.pre.last = seg;      
    }
    
    seg->next = NULL;
    dsdb_info.pre.count++;
    #ifdef EXTREAM_DEBUG
    printf("->FREE SEGMENT (PRE-) prelistcnt: %u max: %u\n",
    dsdb_info.pre.count, dsdb_info.pre.maxcount);
    #endif
  }
  else if( seg->type == DSEG_NORMAL )
  {
    /* just free it */
    dsdb_info.memusage -= seg->size + sizeof( dsdb_segment );
    free( seg );
  }
  /* else do nothing, we do not touch the FASTALLOC segments
   */
}

/*
 * functions (PREALLOC/CLEAN) 
 */
int 
dsdb_pre_alloc( unsigned int seg_cnt )
{
  unsigned int i;
  char *pre;
  dsdb_segment *seg = NULL;
  unsigned unitsize = sizeof( dsdb_segment ) + DSDB_DEFAULT_SEG_SIZE;
  
  /* check
   */
  if( !seg_cnt ) return 0;
  if( dsdb_info.pre.maxcount != 0 ) return -1;
  
  /* allocate
   */
  pre = (char*)malloc( seg_cnt * unitsize );
  if( !pre ) return -1;
  
  /* fill
   */
  for( i = 0; i < seg_cnt; i++ )
  {
    seg = (dsdb_segment*)( pre + ( i * unitsize ) );
    seg->type = DSEG_PREALLOC;
    seg->free = seg->size = DSDB_DEFAULT_SEG_SIZE;
    seg->next = (dsdb_segment*)( ((char*)seg) + unitsize );
    seg->data = ((char*)seg) + sizeof( dsdb_segment );
  }
  
  /* set global info
   */
  dsdb_info.pre.maxcount = dsdb_info.pre.count = seg_cnt;
  dsdb_info.pre.first = (dsdb_segment*)pre;
  dsdb_info.pre.last = seg;
  dsdb_info.pre.last->next = NULL;
  dsdb_info.pre.memusage = seg_cnt * unitsize;
  dsdb_info.memusage += dsdb_info.pre.memusage;
  
  return 0;
}

int 
dsdb_pre_free( void )
{
  #ifdef EXTREAM_DEBUG
    printf("->PREFREE prelistcnt: %u max: %u\n",
    dsdb_info.pre.count, dsdb_info.pre.maxcount);
  #endif
    
  /* check
   */
  if( dsdb_info.pre.maxcount == 0 ) return 0;
  if( dsdb_info.pre.maxcount != dsdb_info.pre.count )
  {
    return dsdb_info.pre.maxcount - dsdb_info.pre.count;
  }
    
  /* free */
  dsdb_info.memusage -= dsdb_info.pre.memusage;
  free( dsdb_info.pre.first );
  dsdb_info.pre.maxcount = 0;
  dsdb_info.pre.count = 0;
  return 0; 
}

/*
 * functions (CREATE/DESTROY) 
 */
dsdb* 
dsdb_createex( unsigned int segsize, unsigned int segcnt )
{
  dsdb *bank;
  dsdb_segment *seg;
  char *space;
  unsigned int unitsize = 0;
  unsigned int memsize = 0;
   
  /* check 
   */
  if( !segsize ) return NULL;
  
  /* calculate mem needed
   */
  if( segcnt )
  {
    unitsize = sizeof( dsdb_segment ) + segsize * segcnt;
  }
  memsize = sizeof( dsdb ) + unitsize;
  
  /* allocate mem
   * NOTE: it's faster to allocate memory for both the segments and dsdb then
   *       using the preallocated segments; the pre- segments are good only
   *       when resizing data bank; the trick also is that insted of allocating
   *       "segcnt" of segment, it just allocates one bigger, since it we be
   *       a constant not movable segment anyway
   */
  space = (char*)malloc( memsize );
  if( !space ) return NULL;
  
  /* fill the info structures
   */
  dsdb_info.memusage += memsize;
  bank = (dsdb*)space;
  memset( bank, 0, sizeof( dsdb ) );
  bank->seg_size = segsize;
  bank->seg_count = !!segcnt;
  bank->mem_usage = memsize;
  
  if( segcnt )
  {
    /* fill the structs so it has all the info about the first seg
     */
    seg = (dsdb_segment*)( space + sizeof( dsdb ) );
    seg->type = DSEG_FASTALLOC;
    seg->free = 
    seg->size = segsize * segcnt;
    seg->data = (char*)( ((char*)seg) + sizeof( dsdb_segment ) );
    bank->start.seg = seg;
    bank->current.seg = seg;
    bank->end.seg = seg;
  }
  
  return bank;
}

void dsdb_destroy( dsdb *bank )
{
  unsigned int segcnt, i;
  dsdb_segment *seg, *lastseg;
  
  /* check
   */
  if( !bank ) return;
  
  /* get segcnt
   */
  segcnt = bank->seg_count;
  

  /* clean segments
   */  
  if( segcnt )
  {
    #ifdef EXTREAM_DEBUG
    printf("->DESTROY segcnt: %u\n", segcnt);
    #endif
    /* go to the last seg
     */
     seg = bank->start.seg;
     for( i = 1; i < segcnt; i++ ) seg = seg->next;
  
    /* free the segments
     */
    for( i = 0; i < segcnt; i++ )
    {
      lastseg = seg;
      seg = seg->prev;
      dsdb_free_segment( lastseg );
    }
  }
  
  /* free the bank
   */
  dsdb_info.memusage -= bank->mem_usage;
  free( bank );
}

/*
 * functions (INFO) 
 */
unsigned int 
dsdb_datasize( dsdb *bank )
{
  /* check
   */
  if( !bank ) return ~0;
  
  return bank->data_size;
}

unsigned int 
dsdb_segcount( dsdb *bank )
{
  /* check
   */
  if( !bank ) return ~0;
  
  return bank->seg_count;
}

unsigned int 
dsdb_segsize( dsdb *bank )
{
  /* check
   */
  if( !bank ) return ~0;
  
  return bank->seg_size;
}

unsigned int 
dsdb_memsize( dsdb *bank )
{
  /* check
   */
  if( !bank ) return ~0;
  
  return bank->mem_usage;
}

unsigned int 
dsdb_memusage( void )
{
  return dsdb_info.memusage;
} 

/*
 * functions (INPUT/OUTPUT) 
 */
int dsdb_read( void *data, unsigned int s_size, unsigned int s_cnt, dsdb *bank )
{
  unsigned int datasize = s_size * s_cnt;
  unsigned int avail;
  unsigned int doffset = 0;
  
  /* check
   */
  if( !data || !bank ) return 0;
  
  /* check data size
   */
  if( datasize > bank->data_size - bank->current.offset )
  {
    datasize = bank->data_size - bank->current.offset;
    s_cnt = datasize / s_size;
  }
  
  if( !datasize ) return 0;
  
  /* try to read data
   */
  #ifdef EXTREAMR_DEBUG
  puts("->READ LOOP");
  #endif
  while( datasize )
  {
    avail = bank->current.seg->size - bank->current.seg_offset;
    #ifdef EXTREAMR_DEBUG
    printf("->READ avail: %u left: %u segoff: %4u/%p\n", avail, datasize,
    bank->current.seg_offset, (void*)bank->current.seg );
    #endif
 
    if( !avail )
    {
      /* jump to next segment
       */
      bank->current.seg = bank->current.seg->next;
      bank->current.seg_offset = 0;
    }
    else
    {
      avail = ( datasize > avail ) ? avail : datasize;
      memcpy( ((char*)data) + doffset,
              &bank->current.seg->data[ bank->current.seg_offset ],
              avail );
      bank->current.seg_offset += avail;
      bank->current.offset += avail;
      doffset += avail;
      datasize -= avail;
    }
  }
  
  #ifdef MEGA_DEBUG
  puts("\n--read cut--\n\n");
  fwrite(data, s_size, s_cnt, stdout);
  puts("\n--read cut--\n\n");
  #endif
  
  return s_cnt;
}

int 
dsdb_write( const void *data, unsigned int s_size, unsigned int s_cnt, dsdb *bank )
{
  unsigned int datasize = s_size * s_cnt;
  unsigned int left, wsize;
  unsigned int state = DSEG_NONE;
  unsigned int doffset = 0;
  dsdb_segment *seg;
  
  /* check
   */
  if( !bank ) return 0;
  if( !datasize ) return 0;
  if( !data ) return 0;
  
  /* get state 
   */
  if( bank->end.seg ) state = DSEG_EXIST;
  

  #ifdef MEGA_DEBUG
  puts( "\n--cut--\n\n" );
  fwrite(data, s_size, s_cnt, stdout);
  puts( "\n--cut--\n\n" );
  #endif

  #ifdef EXTREAM_DEBUG  
  puts( "STARTING LOOP" );
  #endif
  
  /* write data
   */
  while( datasize )
  {
    if( state == DSEG_EXIST && 
        bank->current.seg == bank->end.seg &&
        bank->current.seg_offset == bank->current.seg->size )
    {
      #ifdef EXTREAM_DEBUG
      puts(" NONE");
      #endif
      state = DSEG_NONE;
    }
    else if( state == DSEG_EXIST && 
             bank->current.seg_offset == bank->current.seg->size )
    {
      #ifdef EXTREAM_DEBUG
      printf(" %i %i ", 
      bank->current.seg == bank->end.seg,
        bank->end.seg_offset == bank->end.seg->size );
      puts(" NEXT");
      #endif
      bank->current.seg = bank->current.seg->next;      
      bank->current.seg_offset = 0;
    }
  
    if( state == DSEG_EXIST )
    {
      /* calc free space in segment
       */
      left = bank->current.seg->size - ( bank->current.seg_offset  );
      if( left < datasize )
      {
        wsize = left;
      }
      else
      {
        wsize = datasize;
      }
      datasize -= wsize;
      
      #ifdef EXTREAM_DEBUG
      printf( "->EXIST left: %u insert: %4u segleft: %4u segoff: %4u/%p  ", 
              datasize, wsize, left, bank->current.seg_offset, (void*)bank->current.seg );
      #endif
      
      /* write data
       */
      memcpy( &bank->current.seg->data[ bank->current.seg_offset ],
              ((char*)data) + doffset,
              wsize );
      doffset += wsize;              
      #ifdef EXTREAM_DEBUG
      printf(".");
      #endif
      
      /* post-setup
       */
      bank->current.seg_offset += wsize;
      #ifdef EXTREAM_DEBUG
      printf(".");
      #endif
      bank->current.offset += wsize;
      if( bank->current.offset > bank->end.offset )
      {
        bank->end.offset = bank->current.offset;
        bank->end.seg = bank->current.seg;
        bank->end.seg_offset = bank->current.seg_offset;      
        #ifdef EXTREAM_DEBUG
        printf(".");
        #endif
      }
      #ifdef EXTREAM_DEBUG
      printf(".");
      #endif
      bank->data_size += wsize;
      #ifdef EXTREAM_DEBUG
      printf(".OK\n");
      #endif
    }
    
    if( state == DSEG_NONE )
    {
      #ifdef EXTREAM_DEBUG
      printf( "->NONE left: %u off: %u/%p size: %u\n", datasize,
      bank->current.seg_offset, (void*)bank->current.seg, bank->current.seg->size );
      #endif
      /* allocate
       */
      seg = dsdb_allocate_segment( bank->seg_size );
      if( !seg ) return ( s_cnt * s_size - datasize ) / s_size; 
      
      /* setup
       */
      seg->prev = bank->end.seg;
      if( bank->end.seg )
      {
        bank->end.seg->next = seg;
      }
      seg->next = 0;
      bank->end.seg = seg;
      bank->end.seg_offset = 0;
      bank->current.seg->next = seg;
      bank->current.seg = seg;
      bank->current.seg_offset = 0;
      bank->seg_count++;
      if( seg->type == DSEG_NORMAL ) 
      {
        #ifdef EXTREAM_DEBUG
        puts( "->NORMAL" );
        #endif
        bank->mem_usage += seg->size + sizeof( dsdb_segment );
      }

      /* change state
       */      
      state = DSEG_EXIST;
      
    }    
  } 
  
  return s_cnt;
}

int 
dsdb_puts( dsdb *bank, const char *data )
{
  /* check */
  if( !bank || !data ) return 0;
  
  /* write */
  return dsdb_write( (void*)data, 1, strlen( data ), bank );  
}
 
/*
 * functions (MOVEMENT/SIZE) 
 */

int 
dsdb_seek( dsdb *bank, int offset, int type )
{
  unsigned int wanted = 0, fst = 0;
  /* check
   */
  if( !bank ) return ~0;
  
  /* switch type 
   */
  switch( type )
  {
    /* calc new position
     */
    case DSEEK_SET:
    wanted = (unsigned int)offset;
    break;
    
    case DSEEK_CUR:
    wanted = bank->current.offset + offset;
    break;
    
    /* place pointer after the data
     */
    case DSEEK_END:
    memcpy( &bank->current, &bank->end, sizeof( dsdb_offset ) );
    return 0;
  }
  
  #ifdef EXTREAMS_DEBUG
  printf("->SEEK wanted: %u current: %u\n", wanted, bank->current.offset);
  #endif
  
  if( wanted >= bank->data_size ) return 0;
  
  /* go to the wanted position
   */
  if( wanted == bank->current.offset ) return 0;
  if( wanted < bank->current.offset )
  {
    /* <-- that way */
    fst = bank->current.offset - bank->current.seg_offset;
    if( wanted < fst )
    {
      /* wanted is in some previous segment
       */
      bank->current.offset = fst;
      bank->current.seg_offset = 0;
      #ifdef EXTREAMS_DEBUG
      printf("->SEEK- wanted: %u current: %u\n", wanted, bank->current.offset);
      #endif
      while( wanted < fst )
      {        
        bank->current.seg = bank->current.seg->prev;
        fst = bank->current.offset - bank->current.seg->size;
        bank->current.offset = fst;
        #ifdef EXTREAMS_DEBUG
        printf("->SEEK- wanted: %u current: %u\n", wanted, bank->current.offset);
        #endif
      }
    }
    bank->current.offset = wanted;
    bank->current.seg_offset = wanted - fst;
    #ifdef EXTREAMS_DEBUG
    printf("->SEEK- wanted: %u current: %u\n", wanted, bank->current.offset);
    #endif    
  }
  else
  {
    /* that way --> */
    fst = bank->current.offset - bank->current.seg_offset;
    if( wanted >= fst + bank->current.seg->size )
    {
      /* wanted is in some next segment
       */
      fst = bank->current.offset = fst + bank->current.seg->size;
      bank->current.seg_offset = 0;
      bank->current.seg = bank->current.seg->next;
      #ifdef EXTREAMS_DEBUG
      printf("->SEEK+ wanted: %u current: %u\n", wanted, bank->current.offset);
      #endif
      while( wanted >= fst + bank->current.seg->size )
      { 
        fst = bank->current.offset + bank->current.seg->size;       
        bank->current.seg = bank->current.seg->next;
        bank->current.offset = fst;
        #ifdef EXTREAMS_DEBUG
        printf("->SEEK+ wanted: %u current: %u\n", wanted, bank->current.offset);
        #endif
      }
    }
    bank->current.offset = wanted;
    bank->current.seg_offset = wanted - fst;
    #ifdef EXTREAMS_DEBUG
    printf("->SEEK+ wanted: %u current: %u\n", wanted, bank->current.offset);
    #endif    
  }
  
  return ~0;
}

unsigned int 
dsdb_tell( dsdb *bank )
{
  /* check
   */
  if( !bank ) return ~0;
  
  return bank->current.offset;
}

void 
dsdb_cut( dsdb *bank )
{
  dsdb_segment *seg, *last;
  
  /* check
   */
  if( !bank ) return;
  
  /* cut
   */
  memcpy( &bank->end, &bank->current, sizeof( dsdb_offset ) );
  bank->data_size = bank->current.offset;
  seg = bank->current.seg->next;
  
  while( seg )
  {
    last = seg;
    if( last->type == DSEG_NORMAL )
    {
      #ifdef EXTREAMC_DEBUG
      puts("->CUT NORMAL");
      #endif
      bank->mem_usage -= last->size + sizeof( dsdb_segment );
      dsdb_info.memusage -= last->size + sizeof( dsdb_segment );
    }
    else
    {
      #ifdef EXTREAMC_DEBUG
      puts("->CUT UNORMAL");
      #endif
    }
    seg = last->next;
    dsdb_free_segment( last );
    bank->seg_count--;
  }
  
  bank->end.seg->next = NULL;
} 
 
