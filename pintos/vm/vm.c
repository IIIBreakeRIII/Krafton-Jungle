/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/mmu.h"
#include "threads/vaddr.h"

static struct list frame_table; // 전역 프레임 리스트
static struct lock frame_table_lock;

void vm_init(void)
{
    vm_anon_init();
    vm_file_init();
    list_init(&frame_table);
    lock_init(&frame_table_lock);
    register_inspect_intr();

#ifdef EFILESYS
    pagecache_init();
#endif
}
enum vm_type
page_get_type(struct page *page)
{
    int ty = VM_TYPE(page->operations->type);
    switch (ty)
    {
    case VM_UNINIT:
        return VM_TYPE(page->uninit.type);
    default:
        return ty;
    }
}

static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
                                    vm_initializer *init, void *aux)
{

    ASSERT(VM_TYPE(type) != VM_UNINIT)

    struct supplemental_page_table *spt = &thread_current()->spt;

    if (spt_find_page(spt, upage) == NULL)
    {
        struct page *page = malloc(sizeof(struct page));
        if (page == NULL)
            goto err;

        bool (*page_initializer)(struct page *, enum vm_type, void *);
        switch (VM_TYPE(type))
        {
        case VM_ANON:
            page_initializer = anon_initializer;
            break;
        case VM_FILE:
            page_initializer = file_backed_initializer;
            break;
        default:
            free(page);
            goto err;
        }

        uninit_new(page, upage, init, type, aux, page_initializer);
        page->writable = writable;

        page->owner = thread_current();

        if (!spt_insert_page(spt, page))
        {
            free(page);
            goto err;
        }

        return true;
    }
err:
    return false;
}

void spt_remove_page(struct supplemental_page_table *spt, struct page *page)
{
    hash_delete(&spt->pages, &page->hash_elem);
    vm_dealloc_page(page);
}


static struct frame *
vm_get_victim(void)
{
    lock_acquire(&frame_table_lock);

    if (list_empty(&frame_table))
    {
        lock_release(&frame_table_lock);
        return NULL;
    }

    struct list_elem *e = list_begin(&frame_table);

    // Clock 알고리즘: 한 바퀴 돌면서 accessed bit 체크
    while (true)
    {
        if (e == list_end(&frame_table))
            e = list_begin(&frame_table);

        struct frame *victim = list_entry(e, struct frame, elem);

        // 페이지가 없는 프레임은 건너뜀
        if (victim->page == NULL)
        {
            e = list_next(e);
            continue;
        }

        struct page *page = victim->page;

        // 핵심: 페이지의 소유자 스레드의 pml4를 사용
        struct thread *owner = page->owner;

        // 소유자가 없거나 pml4가 없으면 즉시 victim으로 선택
        if (owner == NULL || owner->pml4 == NULL)
        {
            lock_release(&frame_table_lock);
            return victim;
        }

        // Accessed bit 확인 (올바른 pml4 사용)
        if (pml4_is_accessed(owner->pml4, page->va))
        {
            // Accessed bit를 클리어하고 다음으로
            pml4_set_accessed(owner->pml4, page->va, false);
            e = list_next(e);
        }
        else
        {
            // Accessed bit가 0이면 이 페이지를 victim으로 선택
            lock_release(&frame_table_lock);
            return victim;
        }
    }
}
static struct frame *
vm_evict_frame(void)
{
    struct frame *victim = vm_get_victim();
    struct page *page = victim->page;

    // 페이지를 스왑아웃
    if (!swap_out(page))
        PANIC("Swap out failed");

    victim->page = NULL;
    return victim;
}


static struct frame *
vm_get_frame(void)
{
    void *kva = palloc_get_page(PAL_USER);

    if (kva == NULL)
    {
        // 메모리 부족 - eviction 필요
        struct frame *victim = vm_evict_frame();

        // victim은 이미 swap out되어서 page가 NULL 상태
        // 물리 메모리를 새로 할당받아서 재사용
        victim->kva = palloc_get_page(PAL_USER);
        if (victim->kva == NULL)
        {
            PANIC("Cannot allocate page even after eviction");
        }

        return victim;
    }

    // 새 프레임 할당
    struct frame *frame = malloc(sizeof(struct frame));
    if (frame == NULL)
    {
        palloc_free_page(kva);
        return NULL;
    }

    frame->kva = kva;
    frame->page = NULL;

    lock_acquire(&frame_table_lock);
    list_push_back(&frame_table, &frame->elem);
    lock_release(&frame_table_lock);

    return frame;
}


static void
vm_stack_growth(void *addr) {
    void *stack_page = pg_round_down(addr);
    
    // 스택 크기 제한 체크 (1MB)
    if ((USER_STACK - (uint64_t)stack_page) > (1 << 20))
        return;
    
    // 추가 체크: 너무 낮은 주소는 거부
    // 스택은 USER_STACK에서 아래로 자라지만, 
    // 너무 낮은 주소(예: 힙 영역)까지는 가면 안됨
    if (stack_page < (void *)(USER_STACK - (1 << 20)))
        return;
    
    // 새 페이지 할당하고 claim
    if (vm_alloc_page(VM_ANON, stack_page, true)) {
        vm_claim_page(stack_page);
    }
}

static bool
vm_handle_wp(struct page *page UNUSED)
{
    return false;
}

// vm.c
// vm.c
bool vm_try_handle_fault(struct intr_frame *f, void *addr,
                         bool user, bool write, bool not_present)
{
    struct supplemental_page_table *spt = &thread_current()->spt;
    struct page *page = NULL;

    void *page_addr = pg_round_down(addr);

    page = spt_find_page(spt, page_addr);

    if (page == NULL)
    {
        // 스택 증가 체크
        void *rsp = user ? f->rsp : thread_current()->saved_rsp;

        if (addr >= rsp - 8 && addr < USER_STACK)
        {
            vm_stack_growth(addr);
            return true;
        }

        return false;
    }

    // 쓰기 시도인데 페이지가 읽기 전용이면 실패
    if (write && !page->writable)
    {
        return false;
    }

    return vm_do_claim_page(page);
}
void vm_dealloc_page(struct page *page)
{
    destroy(page);
    free(page);
}

bool vm_claim_page(void *va)
{
    struct page *page = spt_find_page(&thread_current()->spt, va);
    if (page == NULL)
    {
        return false;
    }
    return vm_do_claim_page(page);
}

static bool
vm_do_claim_page(struct page *page)
{
    struct frame *frame = vm_get_frame();

    frame->page = page;
    page->frame = frame;

    if (!pml4_set_page(thread_current()->pml4, page->va, frame->kva, page->writable))
    {
        return false;
    }

    return swap_in(page, frame->kva);
}

static bool spt_copy_page(struct hash_elem *e, void *aux)
{
    struct supplemental_page_table *dst = (struct supplemental_page_table *)aux;
    struct page *src_page = hash_entry(e, struct page, hash_elem);

    enum vm_type type = page_get_type(src_page);
    void *upage = src_page->va;
    bool writable = src_page->writable;

    if (type == VM_UNINIT)
    {
        void *aux_copy = NULL;
        if (src_page->uninit.aux != NULL)
        {
            struct lazy_load_arg *src_aux = (struct lazy_load_arg *)src_page->uninit.aux;
            struct lazy_load_arg *new_aux = malloc(sizeof(struct lazy_load_arg));
            if (new_aux == NULL)
                return false;

            new_aux->file = file_reopen(src_aux->file);
            new_aux->ofs = src_aux->ofs;
            new_aux->read_bytes = src_aux->read_bytes;
            new_aux->zero_bytes = src_aux->zero_bytes;
            aux_copy = new_aux;
        }

        if (!vm_alloc_page_with_initializer(src_page->uninit.type, upage,
                                            writable, src_page->uninit.init, aux_copy))
        {
            if (aux_copy != NULL)
            {
                struct lazy_load_arg *aux_arg = (struct lazy_load_arg *)aux_copy;
                if (aux_arg->file != NULL)
                    file_close(aux_arg->file);
                free(aux_copy);
            }
            return false;
        }
        return true;
    }

    if (!vm_alloc_page(type, upage, writable))
        return false;

    if (!vm_claim_page(upage))
        return false;

    struct page *dst_page = spt_find_page(dst, upage);
    memcpy(dst_page->frame->kva, src_page->frame->kva, PGSIZE);

    return true;
}

bool supplemental_page_table_copy(struct supplemental_page_table *dst,
                                  struct supplemental_page_table *src)
{
    struct hash_iterator i;
    hash_first(&i, &src->pages);

    while (hash_next(&i))
    {
        struct page *src_page = hash_entry(hash_cur(&i), struct page, hash_elem);

        enum vm_type type = page_get_type(src_page);
        void *upage = src_page->va;
        bool writable = src_page->writable;

        // FILE 타입 페이지는 복사하지 않음 (mmap은 상속 안됨)
        if (type == VM_FILE)
            continue;

        /* UNINIT 페이지 처리 */
        if (type == VM_UNINIT)
        {
            // FILE 타입으로 초기화될 UNINIT 페이지도 복사 안함
            if (src_page->uninit.type == VM_FILE)
                continue;

            void *aux_copy = NULL;
            // if (src_page->uninit.aux != NULL) {
            //     struct lazy_load_arg *src_aux = (struct lazy_load_arg *)src_page->uninit.aux;
            //     struct lazy_load_arg *new_aux = malloc(sizeof(struct lazy_load_arg));
            //     if (new_aux == NULL)
            //         return false;

            //     // 실행 파일의 경우: 같은 파일 포인터 공유 (reopen 안함!)
            //     // 자식도 부모와 같은 실행 파일을 사용
            //     new_aux->file = src_aux->file;
            //     new_aux->ofs = src_aux->ofs;
            //     new_aux->read_bytes = src_aux->read_bytes;
            //     new_aux->zero_bytes = src_aux->zero_bytes;
            //     aux_copy = new_aux;
            // }
            // UNINIT 페이지 복사시
            if (src_page->uninit.aux != NULL)
            {
                struct lazy_load_arg *src_aux = (struct lazy_load_arg *)src_page->uninit.aux;
                struct lazy_load_arg *new_aux = malloc(sizeof(struct lazy_load_arg));
                if (new_aux == NULL)
                    return false;

                // 핵심: file은 NULL로 설정
                // lazy_load_segment에서 thread_current()->runn_file을 사용하게 됨
                new_aux->file = NULL; // 이렇게 수정!
                new_aux->ofs = src_aux->ofs;
                new_aux->read_bytes = src_aux->read_bytes;
                new_aux->zero_bytes = src_aux->zero_bytes;
                aux_copy = new_aux;
            }

            if (!vm_alloc_page_with_initializer(src_page->uninit.type, upage,
                                                writable, src_page->uninit.init, aux_copy))
            {
                if (aux_copy != NULL)
                    free(aux_copy);
                return false;
            }
            continue;
        }

        /* ANON 페이지 처리 */
        if (src_page->frame == NULL)
        {
            if (!vm_alloc_page(type, upage, writable))
                return false;
            continue;
        }

        /* claim된 ANON 페이지 복사 */
        if (!vm_alloc_page(type, upage, writable))
            return false;

        if (!vm_claim_page(upage))
            return false;

        struct page *dst_page = spt_find_page(dst, upage);
        if (dst_page == NULL || dst_page->frame == NULL)
            return false;

        memcpy(dst_page->frame->kva, src_page->frame->kva, PGSIZE);
    }

    return true;
}

/* 각 페이지를 destroy하는 액션 함수 */
static void spt_destroy_page(struct hash_elem *e, void *aux UNUSED)
{
    struct page *page = hash_entry(e, struct page, hash_elem);
    destroy(page);
    free(page);
}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt)
{
    hash_destroy(&spt->pages, spt_destroy_page);
}
static unsigned page_hash(const struct hash_elem *e, void *aux)
{
    struct page *p = hash_entry(e, struct page, hash_elem);
    return hash_bytes(&p->va, sizeof p->va);
}

static bool page_less(const struct hash_elem *a,
                      const struct hash_elem *b,
                      void *aux)
{
    struct page *p1 = hash_entry(a, struct page, hash_elem);
    struct page *p2 = hash_entry(b, struct page, hash_elem);
    return p1->va < p2->va;
}

void supplemental_page_table_init(struct supplemental_page_table *spt)
{
    hash_init(&spt->pages, page_hash, page_less, NULL);
}

struct page *spt_find_page(struct supplemental_page_table *spt, void *va)
{
    struct page p;
    p.va = pg_round_down(va);

    struct hash_elem *e = hash_find(&spt->pages, &p.hash_elem);

    if (e == NULL)
        return NULL;

    return hash_entry(e, struct page, hash_elem);
}

bool spt_insert_page(struct supplemental_page_table *spt, struct page *page)
{
    struct hash_elem *he = hash_insert(&spt->pages, &page->hash_elem);

    if (he == NULL)
    {
        return true;
    }

    return false;
}
