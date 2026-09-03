# Authors:
- Edgar Chaves. 2017239281. Edjchg
- Esteban Ureña. carné. Eur

# Compilation Directives with Make

- `make all`: this directive will compile **lotery_scheduler** binary with main.c as entry point.
- `make clean`: this directive will clean all binary and ASan reports.
- `make test`: this directive will compile and run the executable **lotery_scheduler** with main.c as the entry point.
- `make asan`: this directive will compile **lotery_scheduler** against AdressSanitizer. If the binary has any memory issue, then the reports from ASan will be redirected to file called asan_report.<process id> and will look something like:
```bash
asan_report.1823

=================================================================
==1823==ERROR: LeakSanitizer: detected memory leaks

Indirect leak of 40 byte(s) in 1 object(s) allocated from:
    #0 0x7fad1c24e887 in __interceptor_malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:145
    #1 0x55612f6fa5dd in insert_node src/double_linked_list.c:17
    #2 0x55612f6fa3aa in main src/main.c:27
    #3 0x7fad1bf9ad8f  (/lib/x86_64-linux-gnu/libc.so.6+0x29d8f)

Indirect leak of 40 byte(s) in 1 object(s) allocated from:
    #0 0x7fad1c24e887 in __interceptor_malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:145
    #1 0x55612f6fa5dd in insert_node src/double_linked_list.c:17
    #2 0x55612f6fa390 in main src/main.c:26
    #3 0x7fad1bf9ad8f  (/lib/x86_64-linux-gnu/libc.so.6+0x29d8f)

SUMMARY: AddressSanitizer: 80 byte(s) leaked in 2 allocation(s).
```