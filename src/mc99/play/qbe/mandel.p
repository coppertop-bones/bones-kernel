# 1 "mandel.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 414 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "mandel.c" 2





# 1 "/Users/david/arwen/IR/minc/include/stdlib.h" 1
extern int atoi(const char*);
extern void *calloc(unsigned long nitems, unsigned long size);
extern void *malloc(unsigned long size);
extern int abs(int x);
extern void exit(int status);
# 7 "mandel.c" 2
# 1 "/Users/david/arwen/IR/minc/include/SDL2/SDL.h" 1
# 12 "/Users/david/arwen/IR/minc/include/SDL2/SDL.h"
extern void * SDL_CreateWindow(const char *title,
                                                      int x, int y, int w,
                                                      int h, unsigned int flags);

extern void * SDL_CreateRenderer(void * window,
                                                          int index, unsigned int flags);

extern int SDL_SetRenderDrawColor(void * renderer,
                                                   unsigned char r, unsigned char g, unsigned char b,
                                                   unsigned char a);

extern int SDL_RenderDrawPoint(void * renderer,
                                                int x, int y);

extern int SDL_RenderClear(void * renderer);

extern void SDL_RenderPresent(void * renderer);

extern int SDL_PollEvent(void * event);

extern void SDL_DestroyTexture(void * texture);

extern void SDL_DestroyRenderer(void * renderer);

extern void SDL_DestroyWindow(void * window);

extern void SDL_Quit(void);

extern int SDL_Init(unsigned int flags);
# 8 "mandel.c" 2

void *rnd;
int W;
int H;
int *col;

void plot(int x, int y) {
 int n, fx, fy, zx, zy, nx, ny;

 fx = (x - W/2)*4000 / W;
 fy = (y - H/2)*4000 / H;
 zx = fx;
 zy = fy;

 for (n=0; n<200; n++) {
  if (zx*zx + zy*zy > 4000000)
   break;
  nx = (zx*zx)/1000 - (zy*zy)/1000 + fx;
  ny = zx*zy/500 + fy;
  zx = nx;
  zy = ny;
 }
 n = col[n];
 SDL_SetRenderDrawColor(rnd, 100, n, n, 255);
 SDL_RenderDrawPoint(rnd, x, y);
 return;
}

int main() {
 int c, n, x, y, *ie;
 void *e, *win;

 W = 800;
 H = 800;
 SDL_Init(32);
 win = SDL_CreateWindow("Mandelbrot", 0, 0, W, H, 0);
 rnd = SDL_CreateRenderer(win, -1, 0);
 e = malloc(56);
 ie = e;
 col = malloc(201 * sizeof (int));
 c = 20;
 for (n=0; n<200; n++) {
  col[n] = c;
  c = c + (255-c)/8;
 }
 col[n] = 30;

 SDL_RenderClear(rnd);
 for (x=0; x<W; x++)
  for (y=0; y<H; y++)
   plot(x, y);
 SDL_RenderPresent(rnd);

    while (1) {

  if (SDL_PollEvent(e)) {
   if (ie[0] == 769)
    break;
  }
 }

 SDL_DestroyRenderer(rnd);
 SDL_DestroyWindow(win);
 SDL_Quit();
}
