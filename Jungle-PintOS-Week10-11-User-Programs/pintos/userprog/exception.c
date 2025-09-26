#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/mmu.h"
#include "intrinsic.h"

/*
 사용자 프로세스가 특권적이거나 금지된 연산을 수행하면, 예외(exception) 또는 폴트(fault)로 커널에 트랩(trap)됩니다.
 이 파일들은 예외를 처리합니다. 현재 모든 예외는 단순히 메시지를 출력하고 프로세스를 종료시킵니다. 
 프로젝트 2의 일부 솔루션에서는 이 파일의 page_fault()를 수정해야 할 수 있습니다.
*/


/* Number of page faults processed. */
static long long page_fault_cnt;

static void kill (struct intr_frame *);
static void page_fault (struct intr_frame *);

/* 사용자 프로그램에 의해 발생할 수 있는 인터럽트의 핸들러를 등록한다.

   실제 Unix 계열 OS에서는 이러한 인터럽트 대부분을 사용자 프로세스에
   시그널 형태로 전달한다. ([SV-386] 3-24, 3-25 참조)
   그러나 우리는 시그널을 구현하지 않는다. 대신 단순히 해당 사용자
   프로세스를 종료(kill)시킨다.

   페이지 폴트(page fault)는 예외이다. 여기서는 다른 예외와 동일하게
   처리하지만, 가상 메모리를 구현하려면 이 부분은 변경되어야 한다.

   각 예외에 대한 설명은 [IA32-v3a] 문서의 5.15절
   "Exception and Interrupt Reference"를 참조하라.
*/
void
exception_init (void) {
	/* 이러한 예외들은 사용자 프로그램에 의해 명시적으로 발생할 수 있다.
	예를 들어 INT, INT3, INTO, BOUND 명령어를 통해 발생시킬 수 있다.
	따라서 우리는 DPL==3으로 설정하는데, 이는 사용자 프로그램이
	이러한 명령어를 통해 해당 예외들을 호출할 수 있음을 의미한다. */
	intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
	intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
	intr_register_int (5, 3, INTR_ON, kill,
			"#BR BOUND Range Exceeded Exception");

	/* 이러한 예외들은 DPL==0으로 설정되어 있어,
	사용자 프로세스가 INT 명령어를 통해 직접 호출하는 것은 불가능하다.
	하지만 여전히 간접적으로 발생할 수는 있는데,
	예를 들어 #DE(0으로 나누기 예외)는 0으로 나누기를 수행하면 발생한다. */
	intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
	intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
	intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
	intr_register_int (7, 0, INTR_ON, kill,
			"#NM Device Not Available Exception");
	intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
	intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
	intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
	intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
	intr_register_int (19, 0, INTR_ON, kill,
			"#XF SIMD Floating-Point Exception");

	/* 대부분의 예외는 인터럽트를 활성화한 상태에서도 처리할 수 있다.
	하지만 페이지 폴트(page fault)의 경우에는 인터럽트를 비활성화해야 한다.
	그 이유는 폴트가 발생한 주소가 CR2 레지스터에 저장되며,
	이 값이 보존되어야 하기 때문이다. */
	intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

/* Prints exception statistics. */
void
exception_print_stats (void) {
	printf ("Exception: %lld page faults\n", page_fault_cnt);
}

/* Handler for an exception (probably) caused by a user process. */
static void
kill (struct intr_frame *f) {
	/* 이 인터럽트는 (아마도) 사용자 프로세스에 의해 발생한 것이다.
	예를 들어, 프로세스가 매핑되지 않은 가상 메모리에 접근을 시도했을 때
	(페이지 폴트) 발생할 수 있다.
	현재는 단순히 해당 사용자 프로세스를 종료(kill)시킨다.
	나중에는 커널에서 페이지 폴트를 처리하도록 바꿔야 한다.
	실제 Unix 계열 운영체제에서는 대부분의 예외를 시그널을 통해
	프로세스에 다시 전달하지만, 우리는 시그널을 구현하지 않는다. */

	/* 인터럽트 프레임의 코드 세그먼트 값(code segment value)을 통해
	예외가 어디에서 기원했는지(발생한 위치)를 알 수 있다. */
	switch (f->cs) {
		case SEL_UCSEG:
			/* 사용자 코드 세그먼트이므로, 예상대로 사용자 예외이다.
			해당 사용자 프로세스를 종료(kill)한다. */
			printf ("%s: dying due to interrupt %#04llx (%s).\n",
					thread_name (), f->vec_no, intr_name (f->vec_no));
			// intr_dump_frame (f);
			// thread_exit ();
			exit (-1);

		case SEL_KCSEG:
			/* 커널의 코드 세그먼트. 이는 커널 버그를 나타낸다.
			커널 코드에서는 예외가 발생해서는 안 된다.
			(페이지 폴트는 커널 예외를 유발할 수 있지만,
			여기까지 도달해서는 안 된다.)
			이 점을 강조하기 위해 커널을 패닉시킨다. */
			intr_dump_frame (f);
			PANIC ("Kernel bug - unexpected interrupt in kernel");

		case SEL_NULL:
			intr_dump_frame(f);
			exit (-1);

		default:
			/* Some other code segment?  Shouldn't happen.  Panic the
			   kernel. */
			printf ("Interrupt %#04llx (%s) in unknown segment %04x\n",
					f->vec_no, intr_name (f->vec_no), f->cs);
			thread_exit ();
	}
}

/* 페이지 폴트 핸들러(Page fault handler).
   이는 가상 메모리를 구현하기 위해 반드시 채워 넣어야 할 골격(skeleton) 코드이다.
   프로젝트 2의 일부 해법에서도 이 코드를 수정해야 할 수도 있다.

   진입 시점에서 fault가 발생한 주소는 CR2(Control Register 2)에 들어 있으며,
   fault에 대한 정보는 exception.h에 정의된 PF_* 매크로 형식으로
   F의 error_code 멤버에 저장되어 있다.
   아래의 예제 코드는 이 정보를 어떻게 해석하는지를 보여준다.
   CR2와 error_code에 대한 더 자세한 설명은
   [IA32-v3a] 문서 5.15절 "Exception and Interrupt Reference"의
   "Interrupt 14--Page Fault Exception (#PF)" 항목에서 확인할 수 있다. */
static void
page_fault (struct intr_frame *f) {
	bool not_present;  /* True: not-present page, false: writing r/o page. */
	bool write;        /* True: access was write, false: access was read. */
	bool user;         /* True: access by user, false: access by kernel. */
	void *fault_addr;  /* Fault address. */

	/* fault가 발생한 주소, 즉 fault를 일으킨 접근 대상 가상 주소를 얻는다.
	이 주소는 코드나 데이터 영역을 가리킬 수 있다.
	반드시 fault를 일으킨 명령어의 주소는 아니며,
	명령어의 주소는 f->rip에 들어 있다. */
	fault_addr = (void *) rcr2();

	/* 인터럽트를 다시 활성화한다.
	(앞에서는 CR2 값이 변경되기 전에 안전하게 읽을 수 있도록
	잠시 인터럽트를 비활성화했었다.) */
	intr_enable ();


	/* Determine cause. */
	not_present = (f->error_code & PF_P) == 0;
	write = (f->error_code & PF_W) != 0;
	user = (f->error_code & PF_U) != 0;

#ifdef VM
	/* For project 3 and later. */
	if (vm_try_handle_fault (f, fault_addr, user, write, not_present))
		return;
#endif

	/* Count page faults. */
	page_fault_cnt++;

	/* If the fault is true fault, show info and exit. */
	printf ("Page fault at %p: %s error %s page in %s context.\n",
			fault_addr,
			not_present ? "not present" : "rights violation",
			write ? "writing" : "reading",
			user ? "user" : "kernel");
	kill (f);
}

