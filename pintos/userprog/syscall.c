
#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"
#include "threads/init.h"

#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_SYSCALL_MASK 0xc0000084

void syscall_entry(void);
void syscall_handler(struct intr_frame *);
static void check_address(void *addr);
static void release_fd(int fd);
static int allocate_fd(struct file *file);
static struct file *get_file(int fd);
static void check_read_buffer(void *buffer, unsigned size);
static void check_writable_buffer(void *buffer, unsigned size);
static struct lock filesys_lock;

void syscall_init(void)
{
    write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48 |
                            ((uint64_t)SEL_KCSEG) << 32);
    write_msr(MSR_LSTAR, (uint64_t)syscall_entry);
    write_msr(MSR_SYSCALL_MASK,
              FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
    lock_init(&filesys_lock);
}

void syscall_handler(struct intr_frame *f)
{
    // 핵심 수정 1: 시스템 콜 진입시 user rsp 저장
    // 커널 모드에서 페이지 폴트 발생시 스택 증가 판단에 사용
    thread_current()->saved_rsp = f->rsp;
    
    uint64_t nr = f->R.rax;

    switch (nr)
    {
    case SYS_HALT:
        power_off();
        break;

    case SYS_EXIT:
    {
        int status = (int)f->R.rdi;
        struct thread *cur = thread_current();
        cur->exit_status = status;
        printf("%s: exit(%d)\n", cur->name, status);
        thread_exit();
    }
    break;

    case SYS_WRITE:
    {
        int fd = (int)f->R.rdi;
        const void *buffer = (const void *)f->R.rsi;
        unsigned size = (unsigned)f->R.rdx;

        // 수정: WRITE는 버퍼를 읽기만 하므로 읽기 체크
        check_read_buffer((void *)buffer, size);

        if (fd == 1)
        {
            putbuf((const char *)buffer, size);
            f->R.rax = size;
        }
        else if (fd == 0)
        {
            f->R.rax = -1;
        }
        else
        {
            struct file *file = get_file(fd);
            if (file == NULL)
            {
                f->R.rax = -1;
                break;
            }

            lock_acquire(&filesys_lock);
            int bytes_written = file_write(file, buffer, size);
            lock_release(&filesys_lock);

            f->R.rax = bytes_written;
        }
        break;
    }

    case SYS_CREATE:
    {
        const char *path = (const char *)f->R.rdi;
        unsigned sz = (unsigned)f->R.rsi;

        check_address((void *)path);

        if (!path || path[0] == '\0')
        {
            f->R.rax = false;
            break;
        }

        lock_acquire(&filesys_lock);
        bool result = filesys_create(path, sz);
        lock_release(&filesys_lock);

        f->R.rax = result;
        break;
    }

    case SYS_OPEN:
    {
        const char *path = (const char *)f->R.rdi;

        check_address((void *)path);

        lock_acquire(&filesys_lock);
        struct file *file = filesys_open(path);
        lock_release(&filesys_lock);

        if (file == NULL)
        {
            f->R.rax = -1;
        }
        else
        {
            int fd = allocate_fd(file);
            if (fd == -1)
            {
                file_close(file);
                f->R.rax = -1;
            }
            else
            {
                f->R.rax = fd;
            }
        }
        break;
    }

    case SYS_CLOSE:
    {
        int fd = (int)f->R.rdi;

        struct file *file = get_file(fd);
        if (file != NULL)
        {
            lock_acquire(&filesys_lock);
            file_close(file);
            lock_release(&filesys_lock);

            release_fd(fd);
        }
        break;
    }

    case SYS_READ:
    {
        int fd = (int)f->R.rdi;
        void *buffer = (void *)f->R.rsi;
        unsigned size = (unsigned)f->R.rdx;

        // 핵심 수정 2: READ는 버퍼에 쓰는 작업이므로 쓰기 가능 체크
        check_writable_buffer(buffer, size);

        if (fd == 0)
        {
            char *buf = (char *)buffer;
            for (unsigned i = 0; i < size; i++)
            {
                buf[i] = input_getc();
            }
            f->R.rax = size;
        }
        else if (fd == 1)
        {
            f->R.rax = -1;
        }
        else
        {
            struct file *file = get_file(fd);
            if (file == NULL)
            {
                f->R.rax = -1;
                break;
            }

            lock_acquire(&filesys_lock);
            int bytes_read = file_read(file, buffer, size);
            lock_release(&filesys_lock);

            f->R.rax = bytes_read;
        }
        break;
    }

    case SYS_FILESIZE:
    {
        int fd = (int)f->R.rdi;

        struct file *file = get_file(fd);
        if (file == NULL)
        {
            f->R.rax = -1;
            break;
        }

        lock_acquire(&filesys_lock);
        off_t size = file_length(file);
        lock_release(&filesys_lock);

        f->R.rax = size;
        break;
    }

    case SYS_FORK:
    {
        const char *thread_name = (const char *)f->R.rdi;
        check_address((void *)thread_name);
        f->R.rax = process_fork(thread_name, f);
        break;
    }

    case SYS_EXEC:
    {
        const char *cmd_line = (const char *)f->R.rdi;
        check_address((void *)cmd_line);

        char *cmd_copy = palloc_get_page(0);
        if (cmd_copy == NULL)
        {
            f->R.rax = -1;
            break;
        }
        strlcpy(cmd_copy, cmd_line, PGSIZE);

        f->R.rax = process_exec(cmd_copy);
        break;
    }

    case SYS_WAIT:
    {
        tid_t pid = (tid_t)f->R.rdi;
        f->R.rax = process_wait(pid);
        break;
    }

    case SYS_SEEK:
    {
        int fd = (int)f->R.rdi;
        unsigned position = (unsigned)f->R.rsi;

        struct file *file = get_file(fd);
        if (file != NULL)
        {
            lock_acquire(&filesys_lock);
            file_seek(file, position);
            lock_release(&filesys_lock);
        }
        break;
    }

    case SYS_TELL:
    {
        int fd = (int)f->R.rdi;
        struct file *file = get_file(fd);
        if (file == NULL)
        {
            f->R.rax = -1;
            break;
        }

        lock_acquire(&filesys_lock);
        f->R.rax = file_tell(file);
        lock_release(&filesys_lock);
        break;
    }

    case SYS_REMOVE:
    {
        const char *file = (const char *)f->R.rdi;
        check_address((void *)file);

        lock_acquire(&filesys_lock);
        f->R.rax = filesys_remove(file);
        lock_release(&filesys_lock);
        break;
    }

    case SYS_MMAP:
    {
        void *addr = (void *)f->R.rdi;
        size_t length = (size_t)f->R.rsi;
        int writable = (int)f->R.rdx;
        int fd = (int)f->R.r10;
        off_t offset = (off_t)f->R.r8;

        if (fd == 0 || fd == 1)
        {
            f->R.rax = NULL;
            break;
        }

        struct file *file = get_file(fd);
        if (file == NULL)
        {
            f->R.rax = NULL;
            break;
        }

        f->R.rax = (uint64_t)do_mmap(addr, length, writable, file, offset);
        break;
    }

    case SYS_MUNMAP:
    {
        void *addr = (void *)f->R.rdi;
        do_munmap(addr);
        break;
    }

    default:
        printf("Unknown system call: %d\n", (int)nr);
        printf("%s: exit(-1)\n", thread_current()->name);
        thread_current()->exit_status = -1;
        thread_exit();
        break;
    }
}

static void check_address(void *addr)
{
    if (addr == NULL || !is_user_vaddr(addr))
    {
        printf("%s: exit(-1)\n", thread_current()->name);
        thread_current()->exit_status = -1;
        thread_exit();
    }
}

static int allocate_fd(struct file *file)
{
    struct thread *cur = thread_current();

    for (int fd = 3; fd < cur->fd_idx && fd < FDCOUNT_LIMIT; fd++)
    {
        if (cur->fdt[fd] == NULL)
        {
            cur->fdt[fd] = file;
            if (fd >= cur->fd_idx)
                cur->fd_idx = fd + 1;
            return fd;
        }
    }

    if (cur->fd_idx < FDCOUNT_LIMIT)
    {
        cur->fdt[cur->fd_idx] = file;
        return cur->fd_idx++;
    }

    return -1;
}

static struct file *get_file(int fd)
{
    struct thread *cur = thread_current();

    if (fd < 0 || fd >= FDCOUNT_LIMIT || fd >= cur->fd_idx)
    {
        return NULL;
    }

    return cur->fdt[fd];
}

static void release_fd(int fd)
{
    struct thread *cur = thread_current();

    if (fd >= 3 && fd < FDCOUNT_LIMIT && fd < cur->fd_idx)
    {
        cur->fdt[fd] = NULL;
    }
}

/* 읽기용 버퍼 체크 - 주소 유효성만 확인 */
static void check_read_buffer(void *buffer, unsigned size)
{
    if (buffer == NULL || !is_user_vaddr(buffer))
    {
        printf("%s: exit(-1)\n", thread_current()->name);
        thread_current()->exit_status = -1;
        thread_exit();
    }

    char *end = (char *)buffer + size - 1;
    if (!is_user_vaddr(end))
    {
        printf("%s: exit(-1)\n", thread_current()->name);
        thread_current()->exit_status = -1;
        thread_exit();
    }
}

/* 쓰기용 버퍼 체크 - 주소 유효성과 쓰기 권한 확인 */
static void check_writable_buffer(void *buffer, unsigned size)
{
    if (buffer == NULL || !is_user_vaddr(buffer))
    {
        printf("%s: exit(-1)\n", thread_current()->name);
        thread_current()->exit_status = -1;
        thread_exit();
    }

    char *end = (char *)buffer + size - 1;
    if (!is_user_vaddr(end))
    {
        printf("%s: exit(-1)\n", thread_current()->name);
        thread_current()->exit_status = -1;
        thread_exit();
    }
    
    // 핵심: 버퍼의 모든 페이지가 쓰기 가능한지 확인
    struct supplemental_page_table *spt = &thread_current()->spt;
    void *check_addr = pg_round_down(buffer);
    void *end_addr = pg_round_down(end);
    
    while (check_addr <= end_addr) {
        struct page *page = spt_find_page(spt, check_addr);
        
        // 페이지가 있고 쓰기 불가능하면 실패
        if (page != NULL && !page->writable) {
            printf("%s: exit(-1)\n", thread_current()->name);
            thread_current()->exit_status = -1;
            thread_exit();
        }
        
        check_addr += PGSIZE;
    }
}
// /* 쓰기용 버퍼 체크 - 주소 유효성만 확인, 페이지는 page fault로 처리 */
// static void check_writable_buffer(void *buffer, unsigned size)
// {
//     if (buffer == NULL || !is_user_vaddr(buffer))
//     {
//         printf("%s: exit(-1)\n", thread_current()->name);
//         thread_current()->exit_status = -1;
//         thread_exit();
//     }

//     char *end = (char *)buffer + size - 1;
//     if (!is_user_vaddr(end))
//     {
//         printf("%s: exit(-1)\n", thread_current()->name);
//         thread_current()->exit_status = -1;
//         thread_exit();
//     }
    
//     // 주소 범위만 체크하고 페이지 존재 여부는 체크하지 않음
//     // lazy allocation을 위해 page fault가 발생하도록 둠
// }