#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = { "Krafton-Jungle", "Dev/ Paul", "ryu990305@gmail.com", "", "" };

// Define initial constant
#define WSIZE 4
#define DSIZE 8
#define ALIGNMENT 8
#define CHUNKSIZE (1 << 12)

// Define Macro
#define MAX(x, y) (x > y ? x : y)
#define PACK(size, alloc) (size | alloc)
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val)(*(unsigned int *)(p) = (val))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Pointer access helpers for explicit free list links stored in free block payload */
#define PTR_SIZE (sizeof(void *))
#define PREV_FREE_PTR(bp) ((void **)(bp))
#define NEXT_FREE_PTR(bp) ((void **)((char *)(bp) + PTR_SIZE))
#define PREV_FREE(bp) (*PREV_FREE_PTR(bp))
#define NEXT_FREE(bp) (*NEXT_FREE_PTR(bp))

/* Minimum block size: header+footer + prev/next pointers (payload) */
#define MINBLOCK ALIGN((2 * WSIZE) + (2 * PTR_SIZE))

/* Align a requested payload size including header/footer overhead */
#define ALIGNED_ASIZE(req) (ALIGN((req) + (2 * WSIZE)))

/* ===== Segregated free lists settings ===== */
#define LISTS 16               /* number of size classes */
#define USE_BEST_FIT 0         /* 0: first-fit within bin, 1: best-fit within bin */

/* Segregated free list heads */
static void *seg_free_lists[LISTS];

/* helpers */
static int list_index(size_t size);
static void insert_free(void *bp);
static void remove_free(void *bp);

static void *heap_listp; 
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void *place(void *bp, size_t asize);

// mm_init
int mm_init(void) {

  /* initialize segregated list heads */
  for (int i = 0; i < LISTS; i++) seg_free_lists[i] = NULL;

  if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) - 1) { return -1; }

  PUT(heap_listp, 0);
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));  /* prologue header */
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));  /* prologue footer */
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));      /* epilogue header */

  heap_listp += (2 * WSIZE);

  if (extend_heap(CHUNKSIZE / WSIZE) == NULL) { return -1; }

  return 0;
}

// mm_malloc
void *mm_malloc(size_t size) {
  size_t asize;
  size_t extendsize;
  char *bp;

  if (size == 0) { return NULL; }

  asize = ALIGNED_ASIZE(size);
  if (asize < MINBLOCK) asize = MINBLOCK;

  if ((bp = find_fit(asize)) != NULL) {
    return place(bp, asize);
  }

  extendsize = MAX(asize, CHUNKSIZE);

  if ((bp = extend_heap(extendsize/WSIZE)) == NULL) { return NULL; }
  return place(bp, asize);
}

// mm_free
void mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

// mm_realloc
void *mm_realloc(void *ptr, size_t size) {
  if (ptr == NULL) return mm_malloc(size);
  if (size == 0) { mm_free(ptr); return NULL; }

  size_t new_asize = ALIGNED_ASIZE(size);
  if (new_asize < MINBLOCK) new_asize = MINBLOCK;

  size_t old_size = GET_SIZE(HDRP(ptr));

  /* Shrink in place */
  if (new_asize <= old_size) {
    size_t rem = old_size - new_asize;
    if (rem >= MINBLOCK) {
      PUT(HDRP(ptr), PACK(new_asize, 1));
      PUT(FTRP(ptr), PACK(new_asize, 1));
      void *nbp = NEXT_BLKP(ptr);
      PUT(HDRP(nbp), PACK(rem, 0));
      PUT(FTRP(nbp), PACK(rem, 0));
      coalesce(nbp);
    }
    return ptr;
  }

  /* Try to grow into next free block */
  void *next = NEXT_BLKP(ptr);
  if (!GET_ALLOC(HDRP(next))) {
    size_t next_size = GET_SIZE(HDRP(next));
    if (old_size + next_size >= new_asize) {
      remove_free(next);
      size_t total = old_size + next_size;
      size_t rem = total - new_asize;

      PUT(HDRP(ptr), PACK(new_asize, 1));
      PUT(FTRP(ptr), PACK(new_asize, 1));

      if (rem >= MINBLOCK) {
        void *nbp = NEXT_BLKP(ptr);
        PUT(HDRP(nbp), PACK(rem, 0));
        PUT(FTRP(nbp), PACK(rem, 0));
        coalesce(nbp);
      } else {
        PUT(HDRP(ptr), PACK(total, 1));
        PUT(FTRP(ptr), PACK(total, 1));
      }
      return ptr;
    }
  }

  /* If at wilderness (before epilogue), extend heap and retry grow-in-place */
  if (GET_SIZE(HDRP(next)) == 0) {
    size_t need = new_asize - old_size;
    size_t words = (need + WSIZE - 1) / WSIZE;
    if (extend_heap(words) == NULL) return NULL;

    /* Now next is a free block (possibly coalesced by extend_heap) */
    next = NEXT_BLKP(ptr);
    if (!GET_ALLOC(HDRP(next))) {
      remove_free(next);
      size_t total = old_size + GET_SIZE(HDRP(next));
      size_t rem = total - new_asize;

      PUT(HDRP(ptr), PACK(new_asize, 1));
      PUT(FTRP(ptr), PACK(new_asize, 1));

      if (rem >= MINBLOCK) {
        void *nbp = NEXT_BLKP(ptr);
        PUT(HDRP(nbp), PACK(rem, 0));
        PUT(FTRP(nbp), PACK(rem, 0));
        coalesce(nbp);
      } else {
        PUT(HDRP(ptr), PACK(total, 1));
        PUT(FTRP(ptr), PACK(total, 1));
      }
      return ptr;
    }
  }

  /* Try to merge with previous free and move left (one copy) */
  if (!GET_ALLOC(FTRP(PREV_BLKP(ptr)))) {
    void *prev = PREV_BLKP(ptr);
    size_t prev_size = GET_SIZE(HDRP(prev));
    size_t total = prev_size + old_size;

    /* Also consider next if it is free to reduce copies later */
    void *maybe_next = NEXT_BLKP(ptr);
    if (!GET_ALLOC(HDRP(maybe_next))) {
      total += GET_SIZE(HDRP(maybe_next));
    }

    if (total >= new_asize) {
      remove_free(prev);
      if (!GET_ALLOC(HDRP(maybe_next))) remove_free(maybe_next);

      size_t rem = total - new_asize;

      /* Move payload left exactly once */
      memmove(prev, ptr, old_size - DSIZE);

      PUT(HDRP(prev), PACK(new_asize, 1));
      PUT(FTRP(prev), PACK(new_asize, 1));

      if (rem >= MINBLOCK) {
        void *nbp = NEXT_BLKP(prev);
        PUT(HDRP(nbp), PACK(rem, 0));
        PUT(FTRP(nbp), PACK(rem, 0));
        coalesce(nbp);
      } else {
        PUT(HDRP(prev), PACK(prev_size + old_size + (GET_ALLOC(HDRP(maybe_next)) ? 0 : GET_SIZE(HDRP(maybe_next))), 1));
        PUT(FTRP(prev), PACK(prev_size + old_size + (GET_ALLOC(HDRP(maybe_next)) ? 0 : GET_SIZE(HDRP(maybe_next))), 1));
      }
      return prev;
    }
  }

  /* Fallback: allocate new, copy, free old */
  void *newptr = mm_malloc(size);
  if (newptr == NULL) return NULL;
  size_t copySize = old_size - DSIZE;
  if (size < copySize) copySize = size;
  memcpy(newptr, ptr, copySize);
  mm_free(ptr);
  return newptr;
}

// merge - coalesce
static void *coalesce(void * bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  /* coalesce with next */
  if (!next_alloc) {
    void *next = NEXT_BLKP(bp);
    remove_free(next);
    size += GET_SIZE(HDRP(next));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  /* coalesce with prev */
  if (!prev_alloc) {
    void *prev = PREV_BLKP(bp);
    remove_free(prev);
    size += GET_SIZE(HDRP(prev));
    bp = prev;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  insert_free(bp);
  return bp;
}

// extend_heap
static void *extend_heap(size_t words) {
  char *bp;
  size_t size;

  /* Allocate an even number of words to maintain alignment */
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

  if ((long)(bp = mem_sbrk(size)) == -1) { return NULL; }

  /* Initialize free block header/footer and the new epilogue header */
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

  /* Coalesce if the previous block was free and insert into seg list */
  return coalesce(bp);
}

// find_fit
// static void *find_fit(size_t asize) {
//   void *bp = mem_heap_lo() + 2 * WSIZE;
// 
//   for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
//     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) { return bp; }
//   }
// 
//   return NULL;
// }

// find_fit: segregated list first-fit or best-fit within bin
static void *find_fit(size_t asize) {
  int i = list_index(asize);

#if USE_BEST_FIT
  void *best = NULL;
  size_t best_rem = (size_t)-1;
#endif

  for (; i < LISTS; i++) {
    for (void *bp = seg_free_lists[i]; bp != NULL; bp = NEXT_FREE(bp)) {
      size_t csize = GET_SIZE(HDRP(bp));
      if (csize >= asize) {
#if USE_BEST_FIT
        size_t rem = csize - asize;
        if (rem < best_rem) {
          best_rem = rem;
          best = bp;
          if (rem == 0) return bp; /* exact fit */
        }
#else
        return bp; /* first-fit within this bin */
#endif
      }
    }
#if USE_BEST_FIT
    if (best) return best; /* found best within this bin */
#endif
  }
  return NULL;
}

// place
static void *place(void *bp, size_t asize) {
  size_t csize = GET_SIZE(HDRP(bp));

  /* Remove chosen block from its free list */
  remove_free(bp);

  if ((csize - asize) >= MINBLOCK) {
    /* Tail split: leave the front as free, allocate at the tail */
    size_t rem = csize - asize;

    /* front free part */
    PUT(HDRP(bp), PACK(rem, 0));
    PUT(FTRP(bp), PACK(rem, 0));
    insert_free(bp);

    /* allocated tail */
    void *allocp = NEXT_BLKP(bp);
    PUT(HDRP(allocp), PACK(asize, 1));
    PUT(FTRP(allocp), PACK(asize, 1));
    return allocp;
  } else {
    /* allocate the entire block */
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
    return bp;
  }
}

/* Return the segregated list index for a given size (geometric classes) */
static int list_index(size_t size) {
  /* size includes header/footer; pick tighter bins for small sizes */
  if (size <= 32)   return 0;   /* 24–32 */
  if (size <= 48)   return 1;   /* 40–48 */
  if (size <= 64)   return 2;   /* 56–64 */
  if (size <= 80)   return 3;   /* 72–80 */
  if (size <= 96)   return 4;   /* 88–96 */
  if (size <= 112)  return 5;   /* 104–112 */
  if (size <= 128)  return 6;   /* 120–128 */
  if (size <= 192)  return 7;   /* 136–192 */
  if (size <= 256)  return 8;   /* 200–256 */
  if (size <= 384)  return 9;   /* up to 384 (e.g., 320) */
  if (size <= 512)  return 10;  /* 448–512 */
  if (size <= 768)  return 11;
  if (size <= 1024) return 12;
  if (size <= 2048) return 13;
  if (size <= 4096) return 14;
  return 15;                    /* >4096 */
}

/* Insert a free block at the head of its size-class list (LIFO) */
static void insert_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  int i = list_index(size);
  void *head = seg_free_lists[i];

  PREV_FREE(bp) = NULL;
  NEXT_FREE(bp) = head;
  if (head) PREV_FREE(head) = bp;
  seg_free_lists[i] = bp;
}

/* Remove a free block from its size-class list */
static void remove_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  int i = list_index(size);
  void *prev = PREV_FREE(bp);
  void *next = NEXT_FREE(bp);

  if (prev) {
    NEXT_FREE(prev) = next;
  } else {
    seg_free_lists[i] = next;
  }
  if (next) PREV_FREE(next) = prev;

  /* clear links (optional) */
  PREV_FREE(bp) = NULL;
  NEXT_FREE(bp) = NULL;
}
