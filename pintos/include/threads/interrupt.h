#ifndef THREADS_INTERRUPT_H
#define THREADS_INTERRUPT_H

#include <stdbool.h>
#include <stdint.h>

/* Interrupts on or off? */
enum intr_level {
	INTR_OFF,             /* Interrupts disabled. */
	INTR_ON               /* Interrupts enabled. */
};

enum intr_level intr_get_level (void);
enum intr_level intr_set_level (enum intr_level);
enum intr_level intr_enable (void);
enum intr_level intr_disable (void);

/* Interrupt stack frame. */
struct gp_registers {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
} __attribute__((packed));

struct intr_frame {
	/* Pushed by intr_entry in intr-stubs.S.
	   These are the interrupted task's saved registers. */
	struct gp_registers R;
	uint16_t es; // 범용 세그먼트 레지스터
	uint16_t __pad1;
	uint32_t __pad2;
	uint16_t ds; // 데이터 세그먼트
	uint16_t __pad3;
	uint32_t __pad4;
	/* Pushed by intrNN_stub in intr-stubs.S. */
	// 인터럽트의 고유 번호입니다. 
	// 운영체제는 이 번호를 보고 어떤 종류의 인터럽트가 발생했는지 (키보드 입력인지, 시스템 콜인지 등) 파악하고 그에 맞는 처리 루틴을 실행합니다. 
	uint64_t vec_no; /* Interrupt vector number. */
/* Sometimes pushed by the CPU,
   otherwise for consistency pushed as 0 by intrNN_stub.
   The CPU puts it just under `eip', but we move it here. */
	//  일부 예외적인 인터럽트(예: 페이지 폴트)가 발생했을 때, CPU가 하드웨어적으로 에러에 대한 추가 정보를 제공합니다. 
	// 그 외의 경우에는 0으로 채워집니다.
	uint64_t error_code;
/* Pushed by the CPU.
   These are the interrupted task's saved registers. */
	uintptr_t rip; // 인터럽트 직후에 실행해야 할 명령어의 주소
	uint16_t cs;	// 현재 실행 중인 코드의 세그먼트 정보
	uint16_t __pad5;  
	uint32_t __pad6;
	uint64_t eflags; // CPU의 각종 상태 플래그. 연산 결과(양수/음수/0 등)나 CPU 제어 상태를 담고 있습니다.
	uintptr_t rsp; // 인터럽트가 발생하기 직전의 스택의 최상단 주소.
	uint16_t ss; // 현재 사용 중인 스택의 세그먼트 정보.
	uint16_t __pad7;
	uint32_t __pad8;
} __attribute__((packed));

typedef void intr_handler_func (struct intr_frame *);

void intr_init (void);
void intr_register_ext (uint8_t vec, intr_handler_func *, const char *name);
void intr_register_int (uint8_t vec, int dpl, enum intr_level,
                        intr_handler_func *, const char *name);
bool intr_context (void);
void intr_yield_on_return (void);

void intr_dump_frame (const struct intr_frame *);
const char *intr_name (uint8_t vec);

#endif /* threads/interrupt.h */
