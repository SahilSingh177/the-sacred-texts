#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "utils.hpp"

int main(int argc, char *argv[]) noexcept
{
    // 1. Ensure the user provided the input file argument
    if(argc != 2){
        char buf[128];
        char* ptr = buf;
        append_(ptr, "Usage: ");
        append_string(ptr, argv[0]);
        append_(ptr, "<filename>\n");
        ::write(2,buf,ptr-buf);
        _exit(1);
    }

    // 2. Open the file in read-only mode
    int fd = ::open(argv[1], O_RDONLY);
    if(fd<0) [[unlikely]]{
        unix_error("Error opening file");
    }

    // 3. Get the size of the file using fstat
    struct stat statbuf;
    if(::fstat(fd, &statbuf) < 0)[[unlikely]]{
        ::close(fd);
        unix_error("Error getting file status");
    }

    size_t file_size = statbuf.st_size;

    // Handle the edge case of an empty file gracefully
    if (file_size == 0)
    {
        ::close(fd);
        return 0;
    }

    // 4. Map the file into the process's address space
    // We map it as read-only (PROT_READ) and private (MAP_PRIVATE)
    // MAP_POPULATE for aggressive kernel read-ahead (faulting the pages early)
    void *src = ::mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    if(src == MAP_FAILED)[[unlikely]]{
        ::close(fd);
        unix_error("Error mapping file");
    }
    // File descriptor can technically be closed immediately after mapping!
    // The mapping keeps a reference to the underlying file open inside the kernel.
    ::close(fd);

    // 5. High-Speed Direct Write
    // Fixed pointer math by casting 'src' to 'const char*'
    const char *file_bytes = static_cast<const char *>(src);
    size_t bytes_written = 0;
    while(bytes_written < file_size){
        ssize_t n = ::write(STDOUT_FILENO, file_bytes + bytes_written, file_size - bytes_written);
        if(n < 0)[[unlikely]]{
            unix_error("Error writing to stdout");
        }
        bytes_written += n;
    }

    // 6. Clean up resources
    if(::munmap(src, file_size) < 0){
        unix_error("Error unmapping memory");
    }

    return 0;
}