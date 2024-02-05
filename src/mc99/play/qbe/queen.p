# 1 "queen.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "queen.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdio.h" 1
extern int printf(const char *format, ...);




extern void * __stdoutp;
extern int fprintf(void *stream, const char *format, ...);
# 2 "queen.c" 2
# 1 "/Users/david/arwen/IR/minc/include/stdlib.h" 1
extern int atoi(const char*);
extern void *calloc(unsigned long nitems, unsigned long size);
extern void *malloc(unsigned long size);
extern int abs(int x);
extern void exit(int status);
# 3 "queen.c" 2

int Q, nSolutions;

void print(int **board) {
 int i, j;
 for (j=0; j<Q; j++) {
  for (i=0; i<Q; i++)
   if (board[i][j])
    printf(" Q");
   else
    printf(" .");
  printf("\n");
 }
 printf("\n");
}

int chk(int i, int j, int **board) {
 int k, r;
 for (r=k=0; k<Q; k++) {
  r = r + board[i][k];
  r = r + board[k][j];
  if (i+k < Q & j+k < Q)
   r = r + board[i+k][j+k];
  if (i+k < Q & j-k >= 0)
   r = r + board[i+k][j-k];
  if (i-k >= 0 & j+k < Q)
   r = r + board[i-k][j+k];
  if (i-k >= 0 & j-k >= 0)
   r = r + board[i-k][j-k];
 }
 return r;
}

void go(int j, int **board) {
 int i;
 if (j == Q) {
  print(board);
  nSolutions++;
  return;
 }
 for (i=0; i<Q; i++)
  if (chk(i, j, board) == 0) {
   board[i][j]++;
   go(j+1, board);
   board[i][j]--;
  }
}

int **newBoard(int N) {
    int **answer, i;
    answer = calloc(N, sizeof(int *));
    for (i=0; i<N; i++)
        answer[i] = calloc(N, sizeof(int));
    return answer;
}

int main(int ac, char *av[]) {
 int **board;
 Q = 8;
 if (ac >= 2)
  Q = atoi(av[1]);
 board = newBoard(Q);
    nSolutions = 0;
 go(0, board);
 printf("found %d solutions\n", nSolutions);
}
