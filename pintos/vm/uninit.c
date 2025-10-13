#include "vm/vm.h"
#include "vm/uninit.h"

static bool uninit_initialize (struct page *page, void *kva);
static void uninit_destroy (struct page *page);

static const struct page_operations uninit_ops = {
	.swap_in = uninit_initialize,
	.swap_out = NULL,
	.destroy = uninit_destroy,
	.type = VM_UNINIT,
};

void
uninit_new (struct page *page, void *va, vm_initializer *init,
		enum vm_type type, void *aux,
		bool (*initializer)(struct page *, enum vm_type, void *)) {
	ASSERT (page != NULL);

	*page = (struct page) {
		.operations = &uninit_ops,
		.va = va,
		.frame = NULL,
		.uninit = (struct uninit_page) {
			.init = init,
			.type = type,
			.aux = aux,
			.page_initializer = initializer,
		}
	};
}

static bool
uninit_initialize (struct page *page, void *kva) {
	struct uninit_page *uninit = &page->uninit;

	vm_initializer *init = uninit->init;
	void *aux = uninit->aux;

	bool success = uninit->page_initializer(page, uninit->type, kva) &&
		(init ? init(page, aux) : true);
	
	// 성공하면 aux를 여기서 해제
	if (success && aux != NULL) {
		struct lazy_load_arg *larg = (struct lazy_load_arg *)aux;
		if (larg->file != NULL) {
			file_close(larg->file);
		}
		free(aux);
	}
	
	return success;
}

static void
uninit_destroy(struct page *page) {
    struct uninit_page *uninit = &page->uninit;
    
    // 로드되지 않은 페이지의 aux 정리
    if (uninit->aux != NULL) {
        struct lazy_load_arg *aux = (struct lazy_load_arg *)uninit->aux;
        if (aux->file != NULL) {
            file_close(aux->file);
        }
        free(aux);
        uninit->aux = NULL;
    }
}