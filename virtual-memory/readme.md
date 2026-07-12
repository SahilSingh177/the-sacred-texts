The computer architecture image was generated via the command `lstopo computer_arch.png` in the linux terminal.

To drop cache: `sudo sync && echo 3 | sudo tee /proc/sys/vm/drop_caches`

To test file_read vs mmap, use this command to generate a large txt file: `dd if=/dev/zero bs=1M count=100 | tr '\0' 'a' > test.txt`