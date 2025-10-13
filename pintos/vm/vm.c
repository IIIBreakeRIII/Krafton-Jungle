/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/mmu.h"
#include "threads/vaddr.h"

void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS
	pagecache_init ();
#endif
	register_inspect_intr ();
}

enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	if (spt_find_page (spt, upage) == NULL) {
		struct page *page = malloc(sizeof(struct page));
		if (page == NULL)
			goto err;

		bool (*page_initializer)(struct page *, enum vm_type, void *);
		switch (VM_TYPE(type)) {
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

		if (!spt_insert_page(spt, page)) {
			free(page);
			goto err;
		}

		return true;
	}
err:
	return false;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	hash_delete(&spt->pages, &page->hash_elem);
	vm_dealloc_page(page);
}

static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	return victim;
}

static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	return NULL;
}

static struct frame *
vm_get_frame (void) {
	void *kva = palloc_get_page(PAL_USER);
	if (kva == NULL) {
		PANIC("todo");
	}
	
	struct frame *frame = malloc(sizeof(struct frame));
	
	frame->kva = kva;
	frame->page = NULL;
	
	ASSERT(frame != NULL);
	ASSERT(frame->page == NULL);
	return frame;
}

static void
vm_stack_growth (void *addr UNUSED) {
}

static bool
vm_handle_wp (struct page *page UNUSED) {
	return false;
}

bool
vm_try_handle_fault (struct intr_frame *f, void *addr,
                     bool user, bool write, bool not_present) {
    struct supplemental_page_table *spt = &thread_current()->spt;
    struct page *page = NULL;
    
    void *page_addr = pg_round_down(addr);
    
    page = spt_find_page(spt, page_addr);
    
    if (page == NULL) {
        return false;
    }
    
    return vm_do_claim_page(page);
}

void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

bool
vm_claim_page(void *va) {
    struct page *page = spt_find_page(&thread_current()->spt, va);
    if (page == NULL) {
        return false;
    }
    return vm_do_claim_page(page);
}

static bool
vm_do_claim_page(struct page *page) {
    struct frame *frame = vm_get_frame();
    
    frame->page = page;
    page->frame = frame;
    
    if (!pml4_set_page(thread_current()->pml4, page->va, frame->kva, page->writable)) {
        return false;
    }
    
    return swap_in(page, frame->kva);
}

static bool spt_copy_page(struct hash_elem *e, void *aux) {
    struct supplemental_page_table *dst = (struct supplemental_page_table *)aux;
    struct page *src_page = hash_entry(e, struct page, hash_elem);
    
    enum vm_type type = page_get_type(src_page);
    void *upage = src_page->va;
    bool writable = src_page->writable;
    
    if (type == VM_UNINIT) {
        void *aux_copy = NULL;
        if (src_page->uninit.aux != NULL) {
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
                                           writable, src_page->uninit.init, aux_copy)) {
            if (aux_copy != NULL) {
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
bool
supplemental_page_table_copy (struct supplemental_page_table *dst,
		struct supplemental_page_table *src) {
    struct hash_iterator i;
    hash_first(&i, &src->pages);
    
    while (hash_next(&i)) {
        struct page *src_page = hash_entry(hash_cur(&i), struct page, hash_elem);
        
        enum vm_type type = page_get_type(src_page);
        void *upage = src_page->va;
        bool writable = src_page->writable;
        
        /* UNINIT 페이지: aux를 복사해서 등록 */
        if (type == VM_UNINIT) {
            void *aux_copy = NULL;
            if (src_page->uninit.aux != NULL) {
                struct lazy_load_arg *src_aux = (struct lazy_load_arg *)src_page->uninit.aux;
                struct lazy_load_arg *new_aux = malloc(sizeof(struct lazy_load_arg));
                if (new_aux == NULL)
                    return false;
                
                new_aux->file = file_reopen(src_aux->file);
                if (new_aux->file == NULL) {
                    free(new_aux);
                    return false;
                }
                new_aux->ofs = src_aux->ofs;
                new_aux->read_bytes = src_aux->read_bytes;
                new_aux->zero_bytes = src_aux->zero_bytes;
                aux_copy = new_aux;
            }
            
            if (!vm_alloc_page_with_initializer(src_page->uninit.type, upage, 
                                               writable, src_page->uninit.init, aux_copy)) {
                if (aux_copy != NULL) {
                    struct lazy_load_arg *aux_arg = (struct lazy_load_arg *)aux_copy;
                    if (aux_arg->file != NULL)
                        file_close(aux_arg->file);
                    free(aux_copy);
                }
                return false;
            }
            continue;
        }
        
        /* ANON/FILE 페이지이지만 아직 claim 안 된 경우: 등록만 */
        if (src_page->frame == NULL) {
            if (!vm_alloc_page(type, upage, writable))
                return false;
            continue;
        }
        
        /* 이미 claim된 페이지: 등록 + claim + 내용 복사 */
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
static void spt_destroy_page(struct hash_elem *e, void *aux UNUSED) {
    struct page *page = hash_entry(e, struct page, hash_elem);
    destroy(page);
    free(page);
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt) {
    hash_destroy(&spt->pages, spt_destroy_page);
}
static unsigned page_hash(const struct hash_elem *e, void *aux) {
	struct page *p = hash_entry(e, struct page, hash_elem);
	return hash_bytes(&p->va, sizeof p->va);
}

static bool page_less(const struct hash_elem *a, 
                      const struct hash_elem *b, 
                      void *aux) {
	struct page *p1= hash_entry(a, struct page, hash_elem);
	struct page *p2= hash_entry(b, struct page, hash_elem);
	return p1->va < p2->va;
}

void supplemental_page_table_init(struct supplemental_page_table *spt) {
	hash_init(&spt->pages, page_hash, page_less, NULL);
}

struct page *spt_find_page(struct supplemental_page_table *spt, void *va) {
	struct page p;
	p.va = pg_round_down(va);
	
	struct hash_elem *e = hash_find(&spt->pages, &p.hash_elem);
	
	if (e == NULL)
		return NULL;
	
	return hash_entry(e, struct page, hash_elem);
}

bool spt_insert_page(struct supplemental_page_table *spt, struct page *page) {
	struct hash_elem *he = hash_insert(&spt->pages, &page->hash_elem);
	
	if (he == NULL) {
		return true;
	}
	
	return false;
}

