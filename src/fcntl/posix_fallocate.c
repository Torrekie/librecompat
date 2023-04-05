#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int posix_fallocate(int fd, off_t offset, off_t len) {
    if (offset < 0 || len <= 0) {
        return EINVAL;
    }

    fstore_t store = {
        .fst_flags = F_ALLOCATECONTIG | F_ALLOCATEALL,
        .fst_posmode = F_PEOFPOSMODE,
        .fst_offset = offset,
        .fst_length = len,
        .fst_bytesalloc = 0
    };

    int ret = fcntl(fd, F_PREALLOCATE, &store);
    if (ret == -1 && errno == ENOTSUP) {
        store.fst_flags = F_ALLOCATEALL;
        ret = fcntl(fd, F_PREALLOCATE, &store);
    }

    if (ret == -1) {
        return errno;
    }

    if (store.fst_bytesalloc == 0) {
        return 0;
    }

    if (lseek(fd, offset + len - 1, SEEK_SET) == -1) {
        return errno;
    }

    char buf = 0;
    ssize_t n = write(fd, &buf, 1);
    if (n == -1) {
        return errno;
    }

    return 0;
}
