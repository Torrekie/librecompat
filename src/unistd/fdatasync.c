#include <unistd.h>
#include <fcntl.h>

int fdatasync(int fd) {
    return fcntl(fd, F_FULLFSYNC);
}
