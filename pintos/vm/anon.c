#include "vm/vm.h"
#include "devices/disk.h"
#include "threads/vaddr.h"
#include "lib/kernel/bitmap.h"

#define SECTORS_PER_PAGE (PGSIZE / DISK_SECTOR_SIZE)  // 8개 섹터
static struct bitmap *swap_table;  // 스왑 슬롯 사용 여부 추적

static struct disk *swap_disk;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

static const struct page_operations anon_ops = {
	.swap_in = anon_swap_in,
	.swap_out = anon_swap_out,
	.destroy = anon_destroy,
	.type = VM_ANON,
};

void
vm_anon_init(void) {
    swap_disk = disk_get(1, 1);  // 스왑 디스크 가져오기
    if (swap_disk == NULL)
        return;
    
    size_t swap_size = disk_size(swap_disk) / SECTORS_PER_PAGE;
    swap_table = bitmap_create(swap_size);
}
bool
anon_initializer(struct page *page, enum vm_type type, void *kva) {
    page->operations = &anon_ops;
    struct anon_page *anon_page = &page->anon;
    anon_page->swap_slot = -1;  // 초기에는 스왑되지 않음
    return true;
}

static bool
anon_swap_in(struct page *page, void *kva) {
    struct anon_page *anon_page = &page->anon;
    
    // 스왑 인덱스가 유효하지 않으면 그냥 0으로 채움 (새 페이지)
    if (anon_page->swap_slot == -1) {
        memset(kva, 0, PGSIZE);
        return true;
    }
    
    // 디스크에서 읽어오기
    for (int i = 0; i < SECTORS_PER_PAGE; i++) {
        disk_read(swap_disk,
                  anon_page->swap_slot * SECTORS_PER_PAGE + i,
                  kva + i * DISK_SECTOR_SIZE);
    }
    
    // 스왑 슬롯 해제
    bitmap_set(swap_table, anon_page->swap_slot, false);
    anon_page->swap_slot = -1;
    
    return true;
}
// static bool
// anon_swap_out(struct page *page) {
//     struct anon_page *anon_page = &page->anon;
    
//     // 빈 스왑 슬롯 찾기
//     size_t slot = bitmap_scan_and_flip(swap_table, 0, 1, false);
//     if (slot == BITMAP_ERROR)
//         return false;
    
//     // 페이지를 디스크에 쓰기 (8개 섹터)
//     for (int i = 0; i < SECTORS_PER_PAGE; i++) {
//         disk_write(swap_disk, 
//                    slot * SECTORS_PER_PAGE + i,
//                    page->frame->kva + i * DISK_SECTOR_SIZE);
//     }
    
//     anon_page->swap_slot = slot;
    
//     // 페이지 테이블에서 제거
//     pml4_clear_page(thread_current()->pml4, page->va);
    
//     // 프레임 해제
//     palloc_free_page(page->frame->kva);
//     page->frame = NULL;
    
//     return true;
// }

static bool
anon_swap_out(struct page *page) {
    struct anon_page *anon_page = &page->anon;
    
    // 프레임이 없으면 이미 swap out된 상태
    if (page->frame == NULL)
        return true;
    
    // 빈 스왑 슬롯 찾기
    size_t slot = bitmap_scan_and_flip(swap_table, 0, 1, false);
    if (slot == BITMAP_ERROR)
        return false;
    
    // 페이지를 디스크에 쓰기
    for (int i = 0; i < SECTORS_PER_PAGE; i++) {
        disk_write(swap_disk, 
                   slot * SECTORS_PER_PAGE + i,
                   page->frame->kva + i * DISK_SECTOR_SIZE);
    }
    
    anon_page->swap_slot = slot;
    
    // 페이지 테이블에서 제거
    pml4_clear_page(thread_current()->pml4, page->va);
    
    // 프레임 완전히 해제 (물리 메모리 + 구조체)
    palloc_free_page(page->frame->kva);
    free(page->frame);  // 이게 빠져있었음!
    page->frame = NULL;
    
    return true;
}

// static void
// anon_destroy(struct page *page) {
//     struct anon_page *anon_page = &page->anon;
    
//     struct thread *t = thread_current();
//     if (t->pml4 != NULL) {
//         pml4_clear_page(t->pml4, page->va);
//     }
    
//     if (page->frame != NULL) {
//         palloc_free_page(page->frame->kva);
//         free(page->frame);
//         page->frame = NULL;
//     }
// }
static void
anon_destroy(struct page *page) {
    struct anon_page *anon_page = &page->anon;
    
    // 페이지 테이블에서 제거
    if (thread_current()->pml4 != NULL) {
        pml4_clear_page(thread_current()->pml4, page->va);
    }
    
    // 프레임이 있을 때만 해제
    if (page->frame != NULL) {
        palloc_free_page(page->frame->kva);
        free(page->frame);
        page->frame = NULL;
    }
    
    // 스왑 슬롯이 있으면 해제
    if (anon_page->swap_slot != -1) {
        bitmap_set(swap_table, anon_page->swap_slot, false);
        anon_page->swap_slot = -1;
    }
}