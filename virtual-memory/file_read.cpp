#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>    
#include <x86intrin.h>
#include "utils.hpp"

int main(int argc, char *argv[]) noexcept
{
    if (argc != 2)
    {
        char buf[128];
        char *ptr = buf;
        append_(ptr, "Usage: ");
        append_string(ptr, argv[0]);
        append_(ptr, " <filename>\n");
        ::write(2, buf, ptr - buf);
        ::_exit(1);
    }

    int fd = ::open(argv[1], O_RDONLY);
    if (fd < 0) [[unlikely]]
        unix_error("Error opening file");

    struct stat statbuf;
    if (::fstat(fd, &statbuf) < 0) [[unlikely]]
    {
        ::close(fd);
        unix_error("Error getting file status");
    }

    size_t file_size = statbuf.st_size;
    if (file_size == 0)
    {
        ::close(fd);
        return 0;
    }

    unsigned int dummy;
    uint64_t start_cycles = __rdtscp(&dummy); // CPU cycle snapshot before execution

    void *src = ::mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    if (src == MAP_FAILED) [[unlikely]]
    {
        ::close(fd);
        unix_error("Error mapping file");
    }
    ::close(fd);

    const char *file_bytes = static_cast<const char *>(src);
    size_t bytes_written = 0;
    while (bytes_written < file_size)
    {
        ssize_t n = ::write(STDOUT_FILENO, file_bytes + bytes_written, file_size - bytes_written);
        if (n < 0) [[unlikely]]
        {
            if (errno == EINTR)
                continue;
            ::munmap(src, file_size);
            unix_error("Error writing to stdout");
        }
        bytes_written += n;
    }

    if (::munmap(src, file_size) < 0) [[unlikely]]
        unix_error("Error unmapping memory");

    uint64_t end_cycles = __rdtscp(&dummy); // CPU cycle snapshot after execution
    uint64_t total_cycles = end_cycles - start_cycles;

    // Print profiling out to stderr so it doesn't pollute the raw stdout file data
    char perf_buf[128];
    char *perf_ptr = perf_buf;
    append_(perf_ptr, "\n[MMAP PERF] CPU Cycles taken: ");
    append_var(perf_ptr, perf_buf, total_cycles);
    append_char(perf_ptr, '\n');
    ::write(2, perf_buf, perf_ptr - perf_buf);

    return 0;
}