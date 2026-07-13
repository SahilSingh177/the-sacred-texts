#define WSIZE 4 // Word and header/footer size (bytes)
#define DSIZE 8 // Double word size (bytes)
#define CHUNKSIZE (1<<12) // Extend heap by this amount (bytes)

#define MAX(x,y) ((x)>(y)?(x):(y))

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size)|(alloc))

// Read and write a word at address p
#define GET(p) (())