# 1 "collatz.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "collatz.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdlib.h" 1
extern int atoi(const char*);
extern void *calloc(unsigned long nitems, unsigned long size);
extern void *malloc(long size);
extern int abs(int x);
extern void exit(int status);
# 2 "collatz.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 3 "collatz.c" 2

int main() {
 int n;
 int nv;
 int c;
 int cmax;
 int *mem;

 mem = malloc(sizeof(int) * 4000);

 cmax = 0;
 for (nv = 1; nv < 1000; nv++) {
  n = nv;
  c = 0;
  while (n != 1) {
   if (n < nv) {
    c = c + mem[n];
    break;
   }
   if (n & 1)
    n = 3*n + 1;
   else
    n = n / 2;
   c++;
  }
  mem[nv] = c;
  if (c > cmax)
   cmax = c;
 }
 printf("should print 178: %d\n", cmax);
}
