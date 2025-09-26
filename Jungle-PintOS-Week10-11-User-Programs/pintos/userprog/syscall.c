#include "userprog/syscall.h"
#include <stdio.h>
#include "include/lib/kernel/stdio.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "userprog/process.h"
#include "intrinsic.h"
#include "threads/palloc.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "lib/string.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

void check_addr(const void *addr);
int add_file_to_fdt (struct file *file);
void *find_file_by_fd (int fd);

void halt(void);
void exit(int status);
tid_t fork(const char *thread_name, struct intr_frame *f);
int exec (const char *cmd_line);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);


/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

// 파일 시스템 접근용 락
struct lock file_lock;

void
syscall_init (void) {
	lock_init (&file_lock);

	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f) {
	// TODO: Your implementation goes here.
	switch (f->R.rax) {
		case SYS_HALT:                   /* Halt the operating system. */
			halt ();
			break;
		
		case SYS_EXIT:
			exit((int)f->R.rdi);
			break;
		
		case SYS_FORK:                   /* Clone current process. */
			f->R.rax = fork (f->R.rdi, f);
			break;
		
		case SYS_EXEC:                   /* Switch current process. */
			if (exec (f->R.rdi) == -1)
				exit (-1);
			break;
		
		case SYS_WAIT:                   /* Wait for a child process to die. */
			f->R.rax = process_wait ((tid_t)f->R.rdi);
			break;
		
		case SYS_CREATE:                 /* Create a file. */
			f->R.rax = create (f->R.rdi, f->R.rsi);
			break;
		
		case SYS_REMOVE:                 /* Delete a file. */
			f->R.rax = remove (f->R.rdi);
			break;
		
		case SYS_OPEN:                   /* Open a file. */
			f->R.rax = open (f->R.rdi);
			break;
		
		case SYS_FILESIZE:               /* Obtain a file's size. */
			f->R.rax = filesize (f->R.rdi);
			break;

		case SYS_READ:                   /* Read from a file. */
			f->R.rax = read (f->R.rdi, f->R.rsi, f->R.rdx);
			break;
		
		case SYS_WRITE:                  /* Write to a file. */
			f->R.rax = write((int)f->R.rdi, (const void *)f->R.rsi, (unsigned)f->R.rdx);
			break;
		
		case SYS_SEEK:                   /* Change position in a file. */
			seek (f->R.rdi, f->R.rsi);
			break;
		
		case SYS_TELL:                   /* Report current position in a file. */
			f->R.rax = tell (f->R.rdi);
			break;
		
		case SYS_CLOSE:                  /* Close a file. */
			close (f->R.rdi);
			break;
		
		default:
			exit(-1);
			break;
	}
}

void 
halt (void) {
	power_off ();
}

void
exit(int status) {
	struct thread *curr = thread_current ();
	curr->exit_status = status;
	
	/* Terminate this process. */
	printf("%s: exit(%d)\n", curr->name, status);

	thread_exit();
}

tid_t 
fork(const char *thread_name, struct intr_frame *f) {
	return process_fork(thread_name, f);
}

int 
exec (const char *cmd_line)
{
	check_addr (cmd_line);

	char *cmd_copy = palloc_get_page (PAL_ZERO);
	if (cmd_copy == NULL)
		exit (-1);
	strlcpy (cmd_copy, cmd_line, strlen (cmd_line) + 1);

	if (process_exec (cmd_copy) == -1)
		return -1;
}

bool 
create (const char *file, unsigned initial_size)
{
	check_addr (file);
	return filesys_create (file, initial_size);
}

bool 
remove (const char *file)
{
	check_addr (file);
	return filesys_remove (file);
}

int
open (const char *file)
{
	check_addr (file);

	struct file *open_file = filesys_open(file);

	if (open_file == NULL) return -1;
	
	int fd = add_file_to_fdt (open_file);

	if (fd == -1)
		file_close(open_file);

	return fd;
}

int add_file_to_fdt (struct file *file)
{
	struct thread *curr = thread_current ();
	struct file **fdt = curr->fd_table;
	
	while (curr->fd_idx < FDCOUNT_LIMIT && fdt[curr->fd_idx])
	{
		curr->fd_idx++;
	}

	if (curr->fd_idx >= FDCOUNT_LIMIT)
		return -1;

	fdt[curr->fd_idx] = file;
	return curr->fd_idx;
}

int
filesize (int fd)
{
	struct file *file = find_file_by_fd (fd);
	if (file == NULL)
		return -1;
	return file_length (file);	// file.c 참고, Returns the size of FILE in bytes
}

int
read (int fd, void *buffer, unsigned size)
{
	check_addr (buffer);
	uint8_t *read_buffer = buffer;
	int read_bytes = 0;

	if (fd == 0) {
		char key;
		for (read_bytes = 0; read_bytes < size; read_bytes++)
		{
			key = input_getc ();	// input.c 참고
			*read_buffer = key;
			if (key == '\0') 
				break;
			read_buffer++;
		}
	} else if (fd == 1) {	// stdout
		return -1;
	} else {
		struct file *file = find_file_by_fd (fd);
		if (file == NULL) {
			return -1;
		}
		lock_acquire (&file_lock);
		read_bytes = file_read (file, buffer, size);
		lock_release (&file_lock);
	}
	return read_bytes;
}

int
write (int fd, const void *buffer, unsigned size) {
	check_addr (buffer);
	
	int write_bytes = 0;
	lock_acquire (&file_lock);
	if (fd == 0) {
		lock_release (&file_lock);
		return -1;
	} else if (fd == 1) {
		putbuf (buffer, size);	// console.c 참고
		write_bytes = size;
	} else {
		struct file *file = find_file_by_fd (fd);
		if (file == NULL) {
			lock_release (&file_lock);
			return -1;
		}
		write_bytes = file_write (file, buffer, size);
	}
	lock_release (&file_lock);
	return write_bytes;
}

void seek (int fd, unsigned position)
{
	struct file *file = find_file_by_fd (fd);
	if (fd < 2 || file == NULL)
		return ;
	file_seek (file, position);
}

unsigned tell (int fd)
{
	struct file *file = find_file_by_fd (fd);
	if (fd < 2 || file == NULL)
		exit (-1);
	return file_tell (file);
}

void 
close (int fd)
{
	struct thread *curr = thread_current ();
	struct file *file = find_file_by_fd(fd);

	if (file == NULL)
		return ;

	curr->fd_table[fd] = NULL;
}

void
*find_file_by_fd (int fd)
{
	struct thread *curr = thread_current ();
	
	if (fd < 0 || fd > FDCOUNT_LIMIT)
		return NULL;

	return curr->fd_table[fd];
}

// -------------------------------------------------------------------

// 유저 프로세스가 커널 주소에 접근할 경우 즉시 종료
void check_addr(const void *addr)
{
	struct thread *curr = thread_current ();

	// 잘못된 주소 접근
	if (addr == NULL || !is_user_vaddr(addr))
		exit(-1);

	// 물리 페이지에 매핑되지 않은 가상 주소
	if (pml4_get_page(curr->pml4, addr) == NULL)
		exit(-1);
}
