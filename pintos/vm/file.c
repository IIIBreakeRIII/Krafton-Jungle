/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "threads/vaddr.h"
#include "threads/mmu.h"
#include "userprog/process.h"
#include "threads/malloc.h"
#include <string.h>

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

static const struct page_operations file_ops = {
    .swap_in = file_backed_swap_in,
    .swap_out = file_backed_swap_out,
    .destroy = file_backed_destroy,
    .type = VM_FILE,
};

/* 
 * mmap을 위한 lazy loading initializer
 * 
 * 페이지 fault가 발생했을 때 이 함수가 호출되어
 * 실제로 파일에서 데이터를 읽어옴
 */
static bool
lazy_load_mmap(struct page *page, void *aux) {
    struct lazy_load_arg *arg = (struct lazy_load_arg *)aux;
    
    // 파일 페이지 정보 설정
    struct file_page *file_page = &page->file;
    file_page->file = arg->file;
    file_page->offset = arg->ofs;
    file_page->read_bytes = arg->read_bytes;
    file_page->zero_bytes = arg->zero_bytes;
    
    // 프레임이 이미 할당되어 있다고 가정하고 데이터 로드
    if (page->frame == NULL)
        return false;
    
    void *kva = page->frame->kva;
    
    // 파일에서 읽기
    off_t bytes_read = file_read_at(file_page->file, kva, 
                                     file_page->read_bytes, 
                                     file_page->offset);
    
    if (bytes_read != (off_t)file_page->read_bytes) {
        return false;
    }
    
    // 나머지는 0으로 채움
    memset(kva + file_page->read_bytes, 0, file_page->zero_bytes);
    
    return true;
}

void
vm_file_init (void) {
}

bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
    page->operations = &file_ops;
    struct file_page *file_page = &page->file;
    
    // uninit 상태에서 호출되므로 아직 파일 정보가 없음
    // lazy_load_mmap이 호출될 때 설정됨
    file_page->file = NULL;
    file_page->offset = 0;
    file_page->read_bytes = 0;
    file_page->zero_bytes = 0;
    
    return true;
}

static bool
file_backed_swap_in (struct page *page, void *kva) {
    struct file_page *file_page = &page->file;
    
    if (file_page->file == NULL) {
        memset(kva, 0, PGSIZE);
        return true;
    }
    
    // 파일에서 데이터 읽기
    off_t bytes_read = file_read_at(file_page->file, kva, 
                                     file_page->read_bytes, 
                                     file_page->offset);
    
    if (bytes_read != (off_t)file_page->read_bytes)
        return false;
    
    // 나머지 부분은 0으로 채움
    memset(kva + file_page->read_bytes, 0, file_page->zero_bytes);
    
    return true;
}

static bool
file_backed_swap_out (struct page *page) {
    struct file_page *file_page = &page->file;
    
    if (file_page->file == NULL)
        return false;
    
    // dirty bit 확인 - 수정된 경우에만 파일에 씀
    if (pml4_is_dirty(thread_current()->pml4, page->va)) {
        // 파일에 데이터 다시 쓰기
        file_write_at(file_page->file, page->frame->kva, 
                      file_page->read_bytes, file_page->offset);
        
        // dirty bit 클리어
        pml4_set_dirty(thread_current()->pml4, page->va, false);
    }
    
    // 페이지 테이블에서 제거
    pml4_clear_page(thread_current()->pml4, page->va);
    
    // 프레임 해제
    if (page->frame != NULL) {
        palloc_free_page(page->frame->kva);
        free(page->frame);
        page->frame = NULL;
    }
    
    return true;
}

static void
file_backed_destroy (struct page *page) {
    struct file_page *file_page = &page->file;
    
    // 페이지가 메모리에 있고 dirty하면 파일에 쓰기
    if (page->frame != NULL && 
        pml4_is_dirty(thread_current()->pml4, page->va)) {
        file_write_at(file_page->file, page->frame->kva,
                      file_page->read_bytes, file_page->offset);
    }
    
    // 페이지 테이블에서 제거
    if (thread_current()->pml4 != NULL) {
        pml4_clear_page(thread_current()->pml4, page->va);
    }
    
    // 프레임 해제
    if (page->frame != NULL) {
        palloc_free_page(page->frame->kva);
        free(page->frame);
        page->frame = NULL;
    }
}

void *
do_mmap (void *addr, size_t length, int writable,
        struct file *file, off_t offset) {
    
    // 기본 유효성 검사
    if (addr == NULL || addr == 0 || pg_ofs(addr) != 0)
        return NULL;
    
    if (length == 0)
        return NULL;
    
    if (file == NULL)
        return NULL;
    
    // offset이 페이지 정렬되어 있지 않으면 실패
    if (offset % PGSIZE != 0)
        return NULL;
    
    // 파일의 독립적인 참조 생성
    struct file *reopened_file = file_reopen(file);
    if (reopened_file == NULL)
        return NULL;
    
    size_t file_len = file_length(reopened_file);
    if (file_len == 0) {
        file_close(reopened_file);
        return NULL;
    }
    
    // 실제로 읽을 바이트 수 계산
    size_t read_bytes = length < file_len - offset ? length : file_len - offset;
    if (read_bytes == 0) {
        file_close(reopened_file);
        return NULL;
    }
    
    size_t zero_bytes = pg_round_up(read_bytes) - read_bytes;
    
    void *upage = addr;
    off_t ofs = offset;
    
    // 겹치는 페이지가 있는지 미리 확인
    size_t total_pages = (read_bytes + zero_bytes) / PGSIZE;
    for (size_t i = 0; i < total_pages; i++) {
        if (spt_find_page(&thread_current()->spt, upage + i * PGSIZE) != NULL) {
            file_close(reopened_file);
            return NULL;
        }
    }
    
    // 페이지 단위로 lazy loading 설정
    void *start_addr = addr;
    while (read_bytes > 0 || zero_bytes > 0) {
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;
        
        // lazy_load_arg 생성
        struct lazy_load_arg *aux = malloc(sizeof(struct lazy_load_arg));
        if (aux == NULL) {
            // 실패시 이미 할당된 페이지들 정리
            do_munmap(start_addr);
            file_close(reopened_file);
            return NULL;
        }
        
        aux->file = reopened_file;
        aux->ofs = ofs;
        aux->read_bytes = page_read_bytes;
        aux->zero_bytes = page_zero_bytes;
        
        // UNINIT 페이지로 등록 (lazy_load_mmap이 초기화 함수)
        if (!vm_alloc_page_with_initializer(VM_FILE, upage, writable,
                                            lazy_load_mmap, aux)) {
            free(aux);
            do_munmap(start_addr);
            file_close(reopened_file);
            return NULL;
        }
        
        // 다음 페이지로
        read_bytes -= page_read_bytes;
        zero_bytes -= page_zero_bytes;
        upage += PGSIZE;
        ofs += page_read_bytes;
    }
    
    return start_addr;
}

void
do_munmap (void *addr) {
    if (addr == NULL)
        return;
    
    struct supplemental_page_table *spt = &thread_current()->spt;
    void *current_addr = addr;
    struct file *file_to_close = NULL;
    
    // 연속된 파일 매핑 페이지들을 모두 찾아서 제거
    while (true) {
        struct page *page = spt_find_page(spt, current_addr);
        
        if (page == NULL)
            break;
        
        // FILE 타입인지 확인
        enum vm_type type = page_get_type(page);
        if (type != VM_FILE)
            break;
        
        // 첫 페이지에서 파일 포인터 저장 (나중에 닫기 위해)
        if (file_to_close == NULL && page->file.file != NULL) {
            file_to_close = page->file.file;
        }
        
        // 같은 파일이 아니면 중단
        if (page->file.file != file_to_close)
            break;
        
        // 페이지 제거 (destroy 호출됨)
        spt_remove_page(spt, page);
        
        current_addr += PGSIZE;
    }
    
    // 파일 닫기
    if (file_to_close != NULL)
        file_close(file_to_close);
}