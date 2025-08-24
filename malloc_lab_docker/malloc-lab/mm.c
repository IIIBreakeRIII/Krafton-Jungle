/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
  "Team 6",
  "Dev/ Paul",
  "ryu990305@gmail.com",
  "",
  ""
};

// Initial Constant
#define WSIZE 4               // Word Size - bytes
#define DSIZE 8               // Double Word Size - bytes
#define CHUNKSIZE (1 << 12)   // 힙 확장을 위한 기본 크기 - bytes

// Macro Parts
#define MAX(x, y) (x > y ? x : y)
// size(블록 크기), alloc(할당 비트)
#define PACK(size, alloc) (size | alloc)
// 주소 p가 참조하는 워드 - read
#define GET(p) (*(unsigned int *)(p))
// GET 한 주소 p를 val로 반환
#define PUT(p, val) (*(unsigned int *)(p) = (val))
// 주소 p의 크기 - read(조건: 0111와 &연산)
#define GET_SIZE(p) (GET(p) & ~0x7)
// 주소 p의 할당
#define GET_ALLOC(p) (GET(p) & 0x1)
// Header Pointer, “bp가 payload 시작을 가리키니까, header는 그 앞부분이니 WSIZE(=4바이트)만큼 빼준다”
#define HDRP(bp) ((char *)(bp) - WSIZE)
// Footer Pointer "bp + {전체 블록 크기(hdr + pyld + ftr) - (hdr + ftr)}"
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
// 다음 블록의 포인터
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
// 이전 블록의 포인터
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
// 메모리 정렬: 8바이트 단위로 정렬되어야 함
#define ALIGNMENT 8
// 주어진 size를 가장 가까운 8의 배수로 올림(&~0x7 = 하위 3비트를 연산)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
// 정렬 안정을 위한 8바이트 연산
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static void *coalesce(void *bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && !next_alloc) {
    return bp;
  } 
  else if (prev_alloc && !next_alloc) {
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  } 
  else if (!prev_alloc && next_alloc) {
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  } 
  else {
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }
  return bp;
}


/*
 * extend_heap - extends the heap with a new free block
 */

static void *extend_heap(size_t words) {
  char *bp;
  size_t size;

  // Allocate an even number of words to maintain alignment
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  
  if((long)(bp = mem_sbrk(size)) == -1) {
    return NULL;
  }

  // Initialize free block header / footer and the epilogue header
  // Free block header
  PUT(HDRP(bp), PACK(size, 0));
  // Free block footer
  PUT(FTRP(bp), PACK(size, 0));
  // New epilogue header
  PUT(NEXT_BLKP(bp), PACK(0, 1));

  // Coalesce if the previous block was free
  return coalesce(bp);
}

int mm_init(void) {

  // 초기 힙 생성
  char *heap_listp;

  // 비어있는 힙 생성 (4 * 4 = 16bytes): 프롤로그 블록을 구성하기 위해
  if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) - 1) { return -1; }
  // 정렬용 패딩
  PUT(heap_listp, 0);
  // Prologue header
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
  // Prologue footer
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
  // Epilogue header
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));

  // heap_listp += (2 * WSIZE);

  // 힙의 확장을 위해 - 1024워드로
  if (extend_heap(CHUNKSIZE / WSIZE) == NULL) { return -1; }

  return 0;
}

void *mm_malloc(size_t size) {
  // 올림 진행: 사용자 요청 크기(payload) + 최소 8비트(header + sorting)
  int newsize = ALIGN(size + SIZE_T_SIZE);
  // 힙 공간을 newsize만큼 늘림
  void *p = mem_sbrk(newsize);
  // p == (void *) - 1 : 오류 시에 쓰는 특별한 센티넬 값 (전통적인 sbrk()의 관례)
  if (p == (void *) - 1) { return NULL; }
  else {
    // p위치에 요청된 크기 저장
    *(size_t *)p = size;
    // payload의 시작값을 반환하기 위해, 헤더를 포함한 주소 시작 + SIZE_T_SIZE
    return (void *)((char *)p + SIZE_T_SIZE);
  }
}

void mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * malloc의 재할당
 */
void *mm_realloc(void *ptr, size_t size) {
  void *oldptr = ptr;
  void *newptr;
  size_t copySize;

  newptr = mm_malloc(size);
  if (newptr == NULL) {
    return NULL;
  }
  copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
  if (size < copySize) {
    copySize = size;
  }
  memcpy(newptr, oldptr, copySize);
  mm_free(oldptr);
  return newptr;
}
