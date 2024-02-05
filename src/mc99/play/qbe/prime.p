# 1 "prime.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "prime.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 2 "prime.c" 2

int main() {
 int n, t, c, p;
 c = 0;
 n = 2;
 while (n < 5000) {
  t = 2;
  p = 1;
  while (t*t <= n) {
   if (n % t == 0)
    p = 0;
   t++;
  }
  if (p) {
   if (c && c % 10 == 0)
    printf("\n");
   printf("%4d ", n);
   c++;
  }
  n++;
 }
 printf("\n");
}
