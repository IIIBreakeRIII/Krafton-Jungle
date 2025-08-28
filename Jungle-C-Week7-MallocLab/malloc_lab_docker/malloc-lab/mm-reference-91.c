#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

team_t team = { "Krafton-Jungle", "Dev/Paul", "ryu990305@gmail.com", "", "" };

#define CHUNKSIZE (1<<12)
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))
#define PUT(p,val) (*(unsigned int *)(p) = (val))
#define GET(p)  (*(unsigned int *) (p))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDPR(bp) ((char *)(bp) - WSIZE)
#define FTPR(bp) ((char *)(bp) + GET_SIZE(HDPR(bp)) - DSIZE)
#define SUCCPR(bp) (*(void **)(bp))
#define PREDPR(bp) (*(void **)((bp) + WSIZE))
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)  - GET_SIZE((char *)(bp) - DSIZE))
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE 4
#define DSIZE 8

static char *heap_listp = NULL;
static char *last_ptr= NULL;
static char *free_listp = NULL;
static void *find_fit(size_t size);
static void *place(void *ptr, size_t size);
static void *extend_heap(size_t words);
static void *coalesce(void *ptr);

int mm_init(void) {
  if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) - 1) return -1;
  
  PUT(heap_listp, 0);
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
  PUT(heap_listp + (3 * WSIZE), PACK(0,1));
  
  heap_listp += (2 * WSIZE);
  
  return 0;
}
static void *extend_heap(size_t words) {
  char *ptr;
  size_t size;

  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  
  if((long)(ptr = mem_sbrk(size)) == -1) { return NULL; }
  
  PUT(HDPR(ptr), PACK(size,0));
  PUT(FTPR(ptr), PACK(size,0));
  PUT(HDPR(NEXT_BLKP(ptr)), PACK(0, 1));
  
  return coalesce(ptr);
}

void *mm_malloc(size_t size) {
  size_t asize;
  size_t extendsize;
  char *ptr;

  if (size == 0) { return NULL; }
  
  if (size <= DSIZE) { asize = 2 * DSIZE; }
  else { asize = DSIZE * ((size + (DSIZE) + (DSIZE -1)) / DSIZE); }
  
  if ((ptr = find_fit(asize)) != NULL) {
    place(ptr,asize);
    return ptr;
  }
  
  extendsize = MAX(asize, CHUNKSIZE);
  
  if ((ptr = extend_heap(extendsize/ WSIZE)) == NULL) { return NULL; }
  
  place(ptr,asize);
  
  return ptr;
}

void mm_free(void *ptr) {
  size_t size = GET_SIZE(HDPR(ptr));

  PUT(HDPR(ptr), PACK(size,0));
  PUT(FTPR(ptr), PACK(size, 0));
  coalesce(ptr);
}

static void *coalesce(void *ptr) {
  size_t prev_alloc = GET_ALLOC(FTPR(PREV_BLKP(ptr)));
  size_t next_alloc = GET_ALLOC(HDPR(NEXT_BLKP(ptr)));
  size_t size = GET_SIZE(HDPR(ptr));
  
  last_ptr = heap_listp;
  
  if (prev_alloc && next_alloc) {
    return ptr;
  }
  else if (!prev_alloc && next_alloc) {
    size += GET_SIZE(FTPR(PREV_BLKP(ptr)));
    
    PUT(FTPR(ptr), PACK(size, 0));
    PUT(HDPR(PREV_BLKP(ptr)), PACK(size, 0));
    
    ptr = PREV_BLKP(ptr);
  }
  else if (prev_alloc && !next_alloc) {
    size += GET_SIZE(HDPR(NEXT_BLKP(ptr)));
    
    PUT(HDPR(ptr), PACK(size, 0));
    PUT(FTPR(ptr), PACK(size, 0));
  }
  else {
    size += GET_SIZE(FTPR(PREV_BLKP(ptr)));
    size += GET_SIZE(HDPR(NEXT_BLKP(ptr)));
    
    PUT(HDPR(PREV_BLKP(ptr)), PACK(size, 0));
    PUT(FTPR(NEXT_BLKP(ptr)), PACK(size, 0));
    
    ptr = PREV_BLKP(ptr);
  }
  
  return ptr;
}

static void *find_fit(size_t asize) {
  char *ptr;
  if (last_ptr == NULL) {
    last_ptr = heap_listp;
  }

  ptr = last_ptr;
  
  while (GET_SIZE(HDPR(ptr)) > 0) {
    
    if (GET_SIZE(HDPR(ptr)) >= asize && !GET_ALLOC(HDPR(ptr))) {
      last_ptr = ptr;
      return ptr;
    }
    
    ptr = NEXT_BLKP(ptr);
  }

  ptr = heap_listp;
  
  while(ptr != last_ptr) {
    if(GET_SIZE(HDPR(ptr)) >= asize && !GET_ALLOC(HDPR(ptr))) {
      last_ptr = ptr;
      return ptr;
    }

    ptr = NEXT_BLKP(ptr);
  }

  return NULL;
}

static void *place(void *ptr, size_t asize) {
  size_t csize = GET_SIZE(HDPR(ptr));
  size_t diff = csize - asize;
  
  if (diff < 2*DSIZE) {
    PUT(HDPR(ptr), PACK(csize, 1));
    PUT(FTPR(ptr), PACK(csize, 1));
  }
  else {
    PUT(HDPR(ptr), PACK(asize, 1));
    PUT(FTPR(ptr), PACK(asize, 1));
    PUT(HDPR(NEXT_BLKP(ptr)), PACK(diff,0));
    PUT(FTPR(NEXT_BLKP(ptr)), PACK(diff,0));
  }

  return ptr;
}

void *mm_realloc(void *ptr, size_t size) {
  void *oldptr = ptr;
  
  if (ptr == NULL) {
    return mm_malloc(size);
  }

  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }

  if (!GET_ALLOC(HDPR(NEXT_BLKP(ptr)))) {
    
    size_t asize;
    size_t total = GET_SIZE(HDPR(NEXT_BLKP(ptr))) + GET_SIZE(HDPR(ptr));
    
    if(size <= DSIZE) { asize = 2 * DSIZE; }
    else asize = DSIZE * ((size + (DSIZE) + (DSIZE -1)) / DSIZE);
    
    if (total >= asize) {
      size_t diff = total - asize;

      // PUT(HDPR(ptr), PACK(total, 0));
      // PUT(FTPR(ptr), PACK(total, 0));

      // place(ptr, asize);
    
      if (diff < 2 * DSIZE) {
        PUT(HDPR(ptr), PACK(total, 1));
        PUT(FTPR(ptr), PACK(total, 1));
      }
      else {
        PUT(HDPR(ptr), PACK(asize, 1));
        PUT(FTPR(ptr), PACK(asize, 1));
        PUT(HDPR(NEXT_BLKP(ptr)), PACK(diff,0));
        PUT(FTPR(NEXT_BLKP(ptr)), PACK(diff,0));
      }
      
      return ptr;
    }
  }

  void *newptr = mm_malloc(size);
  
  if (!newptr) { return NULL; }
  
  size_t old_size = GET_SIZE(HDPR(ptr)) -DSIZE;
  
  if (old_size > size) { old_size = size; }

  memcpy(newptr, oldptr, old_size);
  mm_free(oldptr);
  
  return newptr;
}
