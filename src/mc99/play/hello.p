# 1 "hello.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "hello.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 2 "hello.c" 2


void * f;
int (*p) (void *, const char *, ...);

char lf() {return 10;}

int main() {
    int i;
    i = 1;
    int (*p2) (void *, const char *, ...);
    p = fprintf;
    f = __stdoutp;
    p2 = p;
    p2(f, "hello %d%c", i, lf());
}
