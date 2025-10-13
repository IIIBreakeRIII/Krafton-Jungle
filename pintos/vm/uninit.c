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

// static bool
// uninit_initialize (struct page *page, void *kva) {
// 	struct uninit_page *uninit = &page->uninit;

// 	vm_initializer *init = uninit->init;
// 	void *aux = uninit->aux;

// 	bool success = uninit->page_initializer(page, uninit->type, kva) &&
// 		(init ? init(page, aux) : true);
	
// 	// 성공하면 aux를 여기서 해제
// 	if (success && aux != NULL) {
// 		struct lazy_load_arg *larg = (struct lazy_load_arg *)aux;
// 		if (larg->file != NULL) {
// 			file_close(larg->file);
// 		}
// 		free(aux);
// 	}
	
// 	return success;
// }
static bool
uninit_initialize (struct page *page, void *kva) {
    struct uninit_page *uninit = &page->uninit;

    vm_initializer *init = uninit->init;
    void *aux = uninit->aux;

    // 페이지 초기화
    bool success = uninit->page_initializer(page, uninit->type, kva) &&
        (init ? init(page, aux) : true);
    
    // 여기서 file_close 하지 않음!
    // aux는 destroy에서 정리됨
    
    return success;
}

// static void
// uninit_destroy(struct page *page) {
//     struct uninit_page *uninit = &page->uninit;
    
//     // 로드되지 않은 페이지의 aux 정리
//     if (uninit->aux != NULL) {
//         struct lazy_load_arg *aux = (struct lazy_load_arg *)uninit->aux;
//         if (aux->file != NULL) {
//             file_close(aux->file);
//         }
//         free(aux);
//         uninit->aux = NULL;
//     }
// }

/* vm/uninit.c */
/* vm/uninit.c */
// static void
// uninit_destroy (struct page *page) {
//     struct uninit_page *uninit = &page->uninit;
    
//     // aux 메모리만 해제
//     // file은 여기서 닫지 않음 (process_cleanup이나 do_munmap에서 처리)
//     if (uninit->aux != NULL) {
//         free(uninit->aux);
//         uninit->aux = NULL;
//     }
// }

static void
uninit_destroy (struct page *page) {
    struct uninit_page *uninit = &page->uninit;
    
    // aux 정리
    if (uninit->aux != NULL) {
        struct lazy_load_arg *aux = (struct lazy_load_arg *)uninit->aux;
        
        // 파일 타입에 따라 다르게 처리
        // mmap의 경우 aux에 저장된 파일이 있지만, do_munmap에서 닫음
        // 실행 파일의 경우 aux에 저장된 파일이 있지만, process_exit에서 닫음
        // 따라서 여기서는 파일을 닫지 않고 aux만 해제
        
        free(aux);
        uninit->aux = NULL;
    }
}