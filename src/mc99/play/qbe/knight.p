# 1 "knight.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "knight.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdlib.h" 1
extern int atoi(const char*);
extern void *calloc(unsigned long nitems, unsigned long size);
extern void *malloc(unsigned long size);
extern int abs(int x);
extern void exit(int status);
# 2 "knight.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 3 "knight.c" 2
# 1 "/Users/david/arwen/IR/minc/include/time.h" 1


extern void time(void *t);
extern char *ctime(void *timer);
# 4 "knight.c" 2

int N = 0;
int **b;
void *t;

int board() {
 int x, y;
    time(t);
    printf("t: %s\n", ctime(t));
 for (y=0; y<8; y++) {
  for (x=0; x<8; x++)
   printf(" %02d", b[x][y]);
  printf("\n");
 }
 printf("\n");
 return 0;
}

int chk(int x, int y) {
 if (x < 0 || x > 7 || y < 0 || y > 7) return 0;
 return b[x][y] == 0;
# 33 "knight.c"
}

int go(int k, int x, int y) {
 int i, j, no, x1, y1;
 b[x][y] = k;
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
# 66 "knight.c"
            }
    }
 b[x][y] = 0;
 return 0;
}

int main() {
 int i;
    t = malloc(8);
    time(t);
    printf("t: %s\n", ctime(t));
 b = calloc(8, sizeof (int *));
 for (i=0; i<8; i++)
  b[i] = calloc(8, sizeof (int));
 go(1, 2, 0);

}
