#include <setjmp.h>
#include <stdio.h>

#define TRY 0
#define FAILED 1
#define RETRY 2
#define FINALLY 3
#define SIGNAL(env, val) _longjmp(env, val)
#define WITH(env) _setjmp(env)
#define Ctx jmp_buf

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/sigsetjmp.3.html

void func(Ctx ctx, int count) {
    printf("Welcome to GeeksforGeeks\n");
    if (count < 5) SIGNAL(ctx, RETRY);
    printf("Finished\n");
}

int main() {
    Ctx ctx; int i=0;

    switch (WITH(ctx)) {
        case TRY:
            printf("Trying...\n");
            func(ctx, ++i);
            printf("Finished 1st time\n");
            break;
        case RETRY:
            printf("RETRY\n");
            func(ctx, ++i);
            SIGNAL(ctx, FINALLY);
        case FAILED:
            printf("FAILED\n");
            SIGNAL(ctx, FINALLY);
        case FINALLY:
            printf("FINALLY\n");
    }
    printf("Tried %i times\n", i);
    return 0;
}

// https://stackoverflow.com/questions/14685406/practical-usage-of-setjmp-and-longjmp-in-c - try catch framework