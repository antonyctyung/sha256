#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    off_t offset = 0x43C00000;
    size_t len = 11*4;

    // Truncate offset to a multiple of the page size, or mmap will fail.
    size_t pagesize = sysconf(_SC_PAGE_SIZE);
    off_t page_base = (offset / pagesize) * pagesize;
    off_t page_offset = offset - page_base;

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    unsigned char *mem = mmap(NULL, page_offset + len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_base);
    mem += page_offset;
    if (mem == MAP_FAILED) {
        perror("Can't map memory");
        return -1;
    }

    size_t i;
    for (i = 0; i < len; ++i)
        printf("%02x ", (int)mem[i]);

    return 0;
}