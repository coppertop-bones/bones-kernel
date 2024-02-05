# 1 "knight2.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "knight2.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdlib.h" 1
extern int atoi(const char*);
extern void *calloc(unsigned long nitems, unsigned long size);
extern void *malloc(unsigned long size);
extern int abs(int x);
extern void exit(int status);
# 2 "knight2.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 3 "knight2.c" 2
# 1 "/Users/david/arwen/IR/minc/include/time.h" 1


extern void time(void *t);
extern char *ctime(void *timer);
# 4 "knight2.c" 2
# 1 "/Users/david/arwen/IR/minc/include/jones.h" 1
# 1 "/Users/david/arwen/IR/minc/include/bk/buckets.h" 1
# 38 "/Users/david/arwen/IR/minc/include/bk/buckets.h"
void * initBuckets(void *a, unsigned long chunkSize);
void * allocInBuckets(void *a, unsigned int n, unsigned int align);
void * reallocInBuckets(void *a, void* p, unsigned int n, unsigned int align);
void checkpointBuckets(void *a, void *s);
void resetToCheckpoint(void *a, void *s);
void cleanBuckets(void *first_bucket);
void freeBuckets(void *first_bucket);
unsigned long numBuckets(void *first_bucket);
int inBuckets(void *a, void *p);
int isAlive(void *a, void *p);
int isDead(void *a, void *p);
# 1 "/Users/david/arwen/IR/minc/include/jones.h" 2
# 5 "knight2.c" 2

int N = 0;
int *b;
void *t;

void * gen;

int board() {
 int x, y;
    time(t);
    printf("t: %s\n", ctime(t));
 for (y=0; y<8; y++) {
  for (x=0; x<8; x++)
   printf(" %02d", b[8 * x + y]);
  printf("\n");
 }
 printf("\n");
 return 0;
}

int chk(int x, int y) {
 if (x < 0 || x > 7 || y < 0 || y > 7) return 0;
 return b[8 * x + y] == 0;
}

int go(int k, int x, int y) {
 int i, j;
 b[8 * x + y] = k;
 if (k == 64) {
  if (x!=2 && y!=0 && (abs(x-2) + abs(y) == 3)) {
   board();
   N++;
   if (N==10) exit(0);
  }
 }
    else {
  for (i=-2; i<=2; i++)
   for (j=-2; j<=2; j++) {
    if (abs(i) + abs(j) == 3 && chk(x+i, y+j))
     go(k+1, x+i, y+j);
            }
    }
 b[8 * x + y] = 0;
 return 0;
}

int main() {
 int i;
    gen = malloc(42);
    initBuckets(gen, 4096);
    t = malloc(8);
    time(t);
    printf("t: %s\n", ctime(t));
    b = allocInBuckets(gen, 64 * sizeof (int), 8);
 go(1, 2, 0);

}
