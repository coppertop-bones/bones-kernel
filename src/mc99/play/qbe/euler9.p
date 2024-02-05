# 1 "euler9.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "euler9.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 2 "euler9.c" 2

int main() {
 int i, a, b, c, d;

 for (a = 1; a < 1000; a++) {
  for (b = a + 1; b < 1000; b++) {
   d = a*a + b*b;
   for (i = 0; i < 1000; i++) {
    if (i * i == d) {
     c = i;
     if (b < c && a+b+c == 1000) {
      printf("%d\n", a*b*c);
      return 0;
     }
     break;
    }
   }
  }
 }
}
