https://cdecl.org/




# specify enum size
https://stackoverflow.com/questions/6159677/gcc-clang-msvc-extension-for-specifying-c-enum-size





# reserved names in C
https://devblogs.microsoft.com/oldnewthing/20230109-00/?p=107685


# standard c lib alternatives
https://ccodearchive.net/
https://github.com/rustyrussell/ccan
https://ccodearchive.net/example-config.h
https://github.com/rustyrussell/ccan/blob/master/ccan/xstring/xstring.h

https://docs.gtk.org/glib/

https://apr.apache.org/

https://libcork.io/0.15.0/


# standard c libs

https://www.gnu.org/software/libc/libc.html

https://musl.libc.org/



# problems with padding
https://research.nccgroup.com/2019/10/30/padding-the-struct-how-a-compiler-optimization-can-disclose-stack-memory/


# structs on the stack - C-ABI
https://news.ycombinator.com/item?id=15520887

https://wolchok.org/posts/parameter-passing/

# Unicode
https://www.haible.de/bruno/packages-libutf8.html
https://www.cprogramming.com/tutorial/unicode.html
https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/
https://icu.unicode.org/
https://home.unicode.org/
https://www.linkedin.com/pulse/mastering-unicode-modern-c-comprehensive-guide-wide-characters-tariq/



# streaming onto memory
https://stackoverflow.com/questions/64328716/c-redirect-fprintf-into-a-buffer-or-char-array
https://man7.org/linux/man-pages/man3/open_memstream.3.html


# subscripts and sizes should be signed
https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf

# floating points
https://possiblywrong.wordpress.com/2015/06/21/floating-point-round-trips/
https://randomascii.wordpress.com/2012/03/08/float-precisionfrom-zero-to-100-digits-2/
https://randomascii.wordpress.com/2023/10/17/localization-failure-temperature-is-hard/#more-4050

# string libs
https://github.com/antirez/sds
https://bstring.sourceforge.net/
https://insights.sei.cmu.edu/library/specifications-for-managed-strings-second-edition/
https://github.com/maxim2266/str


# memory backed FILE* - not available on windows
open_memstream
https://pubs.opengroup.org/onlinepubs/9699919799/functions/open_memstream.html
https://man7.org/linux/man-pages/man3/fmemopen.3.html
https://man7.org/linux/man-pages/man3/open_memstream.3.html
not available in 2023 https://developercommunity.visualstudio.com/t/fmemopenopen_memstream-in-MSVC/10348431?q=open+a+suggestion+ticket
https://github.com/SFTtech/openage/issues/1306
https://stackoverflow.com/questions/12249610/c-create-file-in-memory
https://stackoverflow.com/questions/2864178/equivalent-of-open-memstream-for-msvc - mentions funopen
https://stackoverflow.com/questions/10305095/can-i-replace-a-call-to-open-memstream-with-a-malloc-and-an-implicit-cast
https://stackoverflow.com/questions/46836658/how-to-map-byte-array-as-file-on-windows
https://stackoverflow.com/questions/10305095/can-i-replace-a-call-to-open-memstream-with-a-malloc-and-an-implicit-cast

https://stackoverflow.com/questions/539537/how-to-write-to-a-memory-buffer-with-a-file
https://github.com/Snaipe/fmem/blob/master/src/fmem-funopen.c <<<<<<<<<
https://man7.org/linux/man-pages/man3/fopencookie.3.html <<<<<<<<<

https://man.freebsd.org/cgi/man.cgi?query=funopen&sektion=3&manpath=freebsd-release-ports

https://learn.microsoft.com/en-gb/archive/blogs/larryosterman/its-only-temporary

https://github.com/Arryboom/fmemopen_windows/



