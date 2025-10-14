/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "threads/vaddr.h"
#include "threads/mmu.h"
#include "userprog/process.h"
#include "threads/malloc.h"
#include <string.h>

static bool file_backed_swap_in(struct page *page, void *kva);
static bool file_backed_swap_out(struct page *page);
static void file_backed_destroy(struct page *page);

static const struct page_operations file_ops = {
    .swap_in = file_backed_swap_in,
    .swap_out = file_backed_swap_out,
    .destroy = file_backed_destroy,
    .type = VM_FILE,
};

/*
 * mmap을 위한 lazy loading initializer
 *
 * UNINIT 페이지가 처음 접근될 때 호출됨
 */
static bool
lazy_load_mmap(struct page *page, void *aux)
{
    struct lazy_load_arg *arg = (struct lazy_load_arg *)aux;

    // file_backed_initializer 호출
    file_backed_initializer(page, VM_FILE, NULL);

    // 파일 페이지 정보 설정
    struct file_page *file_page = &page->file;
    file_page->file = arg->file;
    file_page->offset = arg->ofs;
    file_page->read_bytes = arg->read_bytes;
    file_page->zero_bytes = arg->zero_bytes;

    // 프레임이 할당되어 있어야 함
    if (page->frame == NULL)
        return false;

    void *kva = page->frame->kva;

    // 파일에서 읽기
    off_t bytes_read = file_read_at(file_page->file, kva,
                                    file_page->read_bytes,
                                    file_page->offset);

    if (bytes_read != (off_t)file_page->read_bytes)
    {
        return false;
    }

    // 나머지는 0으로 채움
    memset(kva + file_page->read_bytes, 0, file_page->zero_bytes);

    return true;
}

void vm_file_init(void)
{
}

bool file_backed_initializer(struct page *page, enum vm_type type, void *kva)
{
    page->operations = &file_ops;
    struct file_page *file_page = &page->file;

    file_page->file = NULL;
    file_page->offset = 0;
    file_page->read_bytes = 0;
    file_page->zero_bytes = 0;

    return true;
}

static bool
file_backed_swap_in(struct page *page, void *kva)
{
    struct file_page *file_page = &page->file;

    if (file_page->file == NULL)
    {
        memset(kva, 0, PGSIZE);
        return true;
    }

    off_t bytes_read = file_read_at(file_page->file, kva,
                                    file_page->read_bytes,
                                    file_page->offset);

    if (bytes_read != (off_t)file_page->read_bytes)
        return false;

    memset(kva + file_page->read_bytes, 0, file_page->zero_bytes);

    return true;
}

static bool
file_backed_swap_out(struct page *page)
{
    struct file_page *file_page = &page->file;

    if (file_page->file == NULL)
        return false;

    if (page->frame == NULL)
        return true;

    // 핵심: owner의 pml4 사용
    struct thread *owner = page->owner;
    if (owner == NULL || owner->pml4 == NULL)
        return false;

    // dirty bit 확인 후 파일에 쓰기
    if (pml4_is_dirty(owner->pml4, page->va))
    {
        file_write_at(file_page->file, page->frame->kva,
                      file_page->read_bytes, file_page->offset);
        pml4_set_dirty(owner->pml4, page->va, false);
    }

    pml4_clear_page(owner->pml4, page->va);

    // 물리 메모리만 해제
    palloc_free_page(page->frame->kva);

    // 연결만 끊음
    page->frame->page = NULL;
    page->frame = NULL;

    // free(page->frame) 하지 않음!

    return true;
}



static void
file_backed_destroy(struct page *page)
{
    struct file_page *file_page = &page->file;

    if (page->frame != NULL)
    {
        struct thread *owner = page->owner;

        // owner의 pml4 사용
        if (owner != NULL && owner->pml4 != NULL)
        {
            if (pml4_is_dirty(owner->pml4, page->va))
            {
                file_write_at(file_page->file, page->frame->kva,
                              file_page->read_bytes, file_page->offset);
            }
            pml4_clear_page(owner->pml4, page->va);
        }

        palloc_free_page(page->frame->kva);

        // 연결만 끊음
        page->frame->page = NULL;
        page->frame = NULL;
    }
}

void *
do_mmap(void *addr, size_t length, int writable,
        struct file *file, off_t offset)
{

    if (addr == NULL || addr == 0 || pg_ofs(addr) != 0)
        return NULL;

    if (length == 0)
        return NULL;

    if (file == NULL)
        return NULL;

    if (offset % PGSIZE != 0)
        return NULL;

    if (is_kernel_vaddr(addr))
        return NULL;

    // 매핑 범위가 커널 영역과 겹치는지 체크
    // 중요: length를 더한 후 오버플로우 체크도 필요
    if ((uint64_t)addr + length < (uint64_t)addr) // 오버플로우
        return NULL;

    if (is_kernel_vaddr((void *)((uint64_t)addr + length - 1)))
        return NULL;

    // 파일의 독립적인 참조 생성 (중요!)
    struct file *reopened_file = file_reopen(file);
    if (reopened_file == NULL)
        return NULL;

    size_t file_len = file_length(reopened_file);
    if (file_len == 0)
    {
        file_close(reopened_file);
        return NULL;
    }

    size_t read_bytes = length < file_len - offset ? length : file_len - offset;
    if (read_bytes == 0)
    {
        file_close(reopened_file);
        return NULL;
    }

    size_t zero_bytes = pg_round_up(read_bytes) - read_bytes;

    void *upage = addr;
    off_t ofs = offset;

    // 겹치는 페이지 확인
    size_t total_pages = (read_bytes + zero_bytes) / PGSIZE;
    for (size_t i = 0; i < total_pages; i++)
    {
        if (spt_find_page(&thread_current()->spt, upage + i * PGSIZE) != NULL)
        {
            file_close(reopened_file);
            return NULL;
        }
    }

    // 페이지 단위로 lazy loading 설정
    void *start_addr = addr;
    while (read_bytes > 0 || zero_bytes > 0)
    {
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;

        struct lazy_load_arg *aux = malloc(sizeof(struct lazy_load_arg));
        if (aux == NULL)
        {
            do_munmap(start_addr);
            file_close(reopened_file);
            return NULL;
        }

        // 모든 페이지가 같은 파일 포인터를 공유
        aux->file = reopened_file;
        aux->ofs = ofs;
        aux->read_bytes = page_read_bytes;
        aux->zero_bytes = page_zero_bytes;

        if (!vm_alloc_page_with_initializer(VM_FILE, upage, writable,
                                            lazy_load_mmap, aux))
        {
            free(aux);
            do_munmap(start_addr);
            file_close(reopened_file);
            return NULL;
        }

        read_bytes -= page_read_bytes;
        zero_bytes -= page_zero_bytes;
        upage += PGSIZE;
        ofs += page_read_bytes;
    }

    return start_addr;
}

void do_munmap(void *addr)
{
    if (addr == NULL)
        return;

    struct supplemental_page_table *spt = &thread_current()->spt;
    void *current_addr = addr;
    struct file *file_to_close = NULL;

    // 연속된 파일 매핑 페이지들을 제거
    while (true)
    {
        struct page *page = spt_find_page(spt, current_addr);

        if (page == NULL)
            break;

        enum vm_type type = page_get_type(page);

        // FILE 타입이 아니면 중단
        if (type != VM_FILE)
            break;

        // 파일 포인터 찾기
        if (file_to_close == NULL)
        {
            if (type == VM_FILE && page->file.file != NULL)
            {
                file_to_close = page->file.file;
            }
        }

        // 다른 파일이면 중단
        if (type == VM_FILE && page->file.file != NULL &&
            page->file.file != file_to_close)
        {
            break;
        }

        // 페이지 제거
        spt_remove_page(spt, page);

        current_addr += PGSIZE;
    }

    // 파일을 한번만 닫음
    if (file_to_close != NULL)
        file_close(file_to_close);
}