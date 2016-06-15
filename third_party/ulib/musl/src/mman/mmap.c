#include "libc.h"
#include "syscall.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <magenta/syscalls.h>

static void dummy(void) {}
weak_alias(dummy, __vm_wait);

#define UNIT SYSCALL_MMAP2_UNIT
#define OFF_MASK ((-0x2000ULL << (8 * sizeof(long) - 1)) | (UNIT - 1))

static void* io_mmap(void* start, size_t len, int prot, int flags, int fd, off_t off) {
    return (void*) MAP_FAILED;
}
weak_alias(io_mmap, __libc_io_mmap);

void* __mmap(void* start, size_t len, int prot, int flags, int fd, off_t off) {
    if (off & OFF_MASK) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    if (len >= PTRDIFF_MAX) {
        errno = ENOMEM;
        return MAP_FAILED;
    }
    if (flags & MAP_FIXED) {
        __vm_wait();
    }

    //printf("__mmap start %p, len %zu prot %u flags %u fd %d off %llx\n", start, len, prot, flags, fd, off);

    // look for a specific case that we can handle, from pthread_create
    if ((flags & MAP_PRIVATE) && (flags & MAP_ANON) && (fd < 0)) {
        // round up to page size
        len = (len + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        mx_handle_t vmo = _magenta_vm_object_create(len);
        if (vmo < 0)
            return MAP_FAILED;

        // build magenta flags for this
        uint32_t mx_flags = 0;
        mx_flags |= (prot & PROT_READ) ? MX_VM_FLAG_PERM_READ : 0;
        mx_flags |= (prot & PROT_WRITE) ? MX_VM_FLAG_PERM_WRITE : 0;
        mx_flags |= (prot & PROT_EXEC) ? MX_VM_FLAG_PERM_EXECUTE : 0;
        mx_flags |= (flags & MAP_FIXED) ? MX_VM_FLAG_FIXED : 0;

        mx_handle_t current_proc_handle = 0; /* TODO: get from TLS */

        uintptr_t ptr = (uintptr_t)start;
        mx_status_t status = _magenta_process_vm_map(current_proc_handle, vmo, 0, len, &ptr, mx_flags);
        _magenta_handle_close(vmo);
        if (status < 0) {
            return MAP_FAILED;
        }

        return (void *)ptr;
    } else {
        // let someone else get a shot at this
        return __libc_io_mmap(start, len, prot, flags, fd, off);
    }
}

weak_alias(__mmap, mmap);

LFS64(mmap);