#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <charconv>
#include <system_error>

// [[noreturn]] tells the compiler this function never returns
// , thus allowing deep optimizations
// [[gnu::cold]] tells the compiler this path is rarely taken.
// So, move them to a distant, "cold" text segment in the final binary.
// noexcept means this function will never throw exception
// this optimizes as hidden stack-unwinding tables aren't needed as they are required by exceptions
[[noreturn]][[gnu::cold]]void unix_error(const char* msg)noexcept{
    // Sends the custom error string directly to stderr (File descriptor 2)
    // strlen is evaluated at compile time for string literals under optimizationflags

    // 0 -> standard input
    // 1 -> standard output
    // 2 -> standard error
    // fprintf or std::cerr allocate memory and buffer the string in user-space before eventually calling write.
    // If the process is crashing due to memory corruption, trying to allocate memory for a buffer inside fprintf might cause a secondary segmentation fault, masking the original error. write needs no heap allocation.
    ::write(2,msg,::strlen(msg));
    ::write(2,": ",2);

    const char* err_str = ::strerror(errno);
    ::write(2,err_str,::strlen(err_str)); // ::strlen calculates length at compile time.
    ::write(2,"\n",1);

    // Exit immediately without running C++ destructor cleanups
    ::_exit(1);
    // In systems managing shared states running static destructors in a 
    // failing child or parent process could accidently close file descriptors
    // or corrupt shared memory that the rest of the application still relies on. 
}

// [[nodiscard]] forces the caller to use the PID, preventing silent failures.
// i.e, the compiler will throw a loud warning (or an error, if you compile with -Werror).
// inline makes it like the function is inside the main function i.e the caller's code
[[nodiscard]]inline pid_t Fork() noexcept {
    // It tells the compiler to look exclusively in the global namespace (where the POSIX C library functions live) for fork.
    // So, even if we have a function named fork it won't call that.
    pid_t pid = ::fork();
    // [[unlikely]] is a C++20 branch predictor hint.
    // It tells the CPU to optimize the assembly for the success path (pid >= 0).
    // Thus, the compiler ensures that the CPU's path of least resistance—its default, continuous assembly line—is completely dedicated to the success path. It guarantees that the CPU won't waste time speculatively executing error-handling code that almost never happens.
    if(pid<0)[[unlikely]]{
        unix_error("Fork error");
    }
    return pid;
}

int main() noexcept {
    char buf[16]; // The maximum number of characters a 32-bit integer can take up is 11 (including a negative sign), plus a few extra bytes for safety and the newline.
    // Thus, the stack is almost always "hot" in the L1 data cache.
    pid_t pid = Fork();
    // std::to_string allocates memory on the heap. sprintf has to parse a format string ("%d") at runtime and is burdened by checking your system's "locale" settings (e.g., whether to use commas or periods for thousands separators).

    // std::to_chars ignores locale entirely. It does zero heap allocation. It uses highly optimized, bare-metal algorithms to convert the integer directly into the bytes of the buf array. It returns a struct containing a pointer (ptr) pointing to the exact memory address right after the last digit written.
    auto [ptr,ec] = std::to_chars(buf,buf+sizeof(buf),static_cast<int>(pid));
    // Instead of throwing a heavy C++ exception if the buffer is too small, std::to_chars returns an error code (ec). Comparing it to std::errc() (which means "no error") is a simple integer comparison. It's incredibly fast.
    if(ec==std::errc()){
        *ptr++ = '\n'; // take the pointer ptr (which is already sitting exactly where the string ended), write a newline character to that exact byte in memory, and then increment the pointer.
        ::write(1,buf,ptr-buf); // pointers subtraction to find the length of word+newline that we added
    }
}