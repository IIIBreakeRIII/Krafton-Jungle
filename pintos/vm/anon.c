#include "vm/vm.h"
#include "devices/disk.h"
#include "threads/vaddr.h"

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
vm_anon_init (void) {
	swap_disk = NULL;
}

bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	page->operations = &anon_ops;
	struct anon_page *anon_page = &page->anon;
	return true;
}

static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page = &page->anon;
    memset(kva, 0, PGSIZE);
    return true;
}

static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page = &page->anon;
	return true;
}

static void
anon_destroy(struct page *page) {
    struct anon_page *anon_page = &page->anon;
    
    struct thread *t = thread_current();
    if (t->pml4 != NULL) {
        pml4_clear_page(t->pml4, page->va);
    }
    
    if (page->frame != NULL) {
        palloc_free_page(page->frame->kva);
        free(page->frame);
        page->frame = NULL;
    }
}