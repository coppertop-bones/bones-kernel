// ---------------------------------------------------------------------------------------------------------------------
// Copyright 2025 David Briant, https://github.com/coppertop-bones. Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------------------------------------------------

// demonstrates closures on the stack in a standards compliant manner (although the same mechanism can have closures
// on the heap)


#include <stdio.h>
#include <stdbool.h>

// https://www.reddit.com/r/C_Programming/comments/s99jej/a_new_design_pattern_for_implementing_capturing/

// advantages of such a design pattern:
// - fully portable, it is fully defined within C89 and newer standards
// - combines a function pointer and void pointer to context into one entity
// - is strongly typed
// - no need for ABI extensions
// - user has full control over what is captured by the closure, an advantage over GCC's nested functions or CLANG's blocks
// - the closure is a simple struct, it can be dynamic, automatic, static, or even from alloca()


// lib types
typedef bool block_ctx_f64_bool(void*, double); // a block function that takes a context and a double and returns a bool
typedef bool block_ctx_i32_bool(void*, int);
typedef block_ctx_f64_bool blockfn_ctx_f64_bool;
typedef block_ctx_i32_bool blockfn_ctx_i32_bool;



// #define BLOCK_T(t) (block_ctx_##t##_bool **)
#define BLOCK_T(T) T **
// #define BLOCK_FN_T(T) T **



// lib fns - C, Python or bones
void select_i32(int n, BLOCK_T(block_ctx_i32_bool) block) {
    // mock by printing the selected numbers - else needs a bucket to be neat
    for (int i = 1; i <= n; ++i)
        if ((*block)(block, i))             // dereference the block function and call it with the block and other args
            printf("%d ", i);
    puts("");
}

void select_f64(int n, BLOCK_T(block_ctx_f64_bool) block) {
    for (int i = 1; i <= n; ++i)
        if ((*block)(block, (double) i))
            printf("%d ", i);
    puts("");
}


// fred local - generated in MIR
typedef struct {
    blockfn_ctx_i32_bool *_;
    int divisor;                    // read-only
    int opCount;                    // rebindable 
} is_divisible_ctx;

typedef struct {
    blockfn_ctx_i32_bool *_;
} is_primeable_ctx;


bool is_divisible(void *ctx, int n) {
    is_divisible_ctx *ctx_ = ctx;
    ctx_->opCount++;
    return (n % ctx_->divisor) == 0;
}

bool is_prime(void *ctx, int n) {
    int d;
    if (n <= 1) return 0;
    for (d = 2; d * d <= n; ++d) {
        if (n % d == 0) return false;
    }
    return true;
}

void fred() {
    is_divisible_ctx ctx1;  block_ctx_i32_bool *ctx2;
    ctx1._ = is_divisible;
    ctx1.divisor = 3;
    ctx1.opCount = 0;

    // with the context
    puts("Divisible by 3");
    select_i32(20, (BLOCK_T(block_ctx_i32_bool)) &ctx1);
    printf("opCount: : %i\n", ctx1.opCount);

    // just the function
    ctx2 = is_prime;
    puts("Primes");
    select_i32(20, (BLOCK_T(block_ctx_i32_bool)) &ctx2);
}


// joe local - ctx has 2 fns with shared name opCount
typedef struct {
    int opCount;
} CommonCtx;

typedef struct {
    blockfn_ctx_i32_bool *_;
} IsPrimeCtx;

typedef struct {
    IsPrimeCtx is_prime_ctx;
    CommonCtx common_ctx;
} IsPrimeAndCommonCtx;

typedef struct {
    blockfn_ctx_f64_bool *_;
    double divisor;
} IsDivCtx;

typedef struct {
    IsDivCtx is_div_ctx;
    IsPrimeCtx _is_prime_ctx;
    CommonCtx common_ctx;
} IsDivAndCommonCtx;

typedef struct {
    IsDivCtx is_div_ctx;
    IsPrimeCtx is_prime_ctx;
    CommonCtx common_ctx;
} MainCtx;


bool is_divisible2(void *ctx_, double n) {
    IsDivAndCommonCtx *ctx = ctx_;
    ctx->common_ctx.opCount++;
    return ((int) n % (int)ctx->is_div_ctx.divisor) == 0;
}

bool is_prime2(void *ctx_, int n) {
    IsPrimeAndCommonCtx *ctx = ctx_;
    if (n <= 1) return 0;
    for (int d = 2; d * d <= n; ++d) {
        ctx->common_ctx.opCount++;
        if (n % d == 0) return false;
    }
    return true;
}

int joe() {
    MainCtx ctx;
    ctx.is_div_ctx._ = is_divisible2;
    ctx.is_div_ctx.divisor = 3;
    ctx.is_prime_ctx._ = is_prime2;
    ctx.common_ctx.opCount = 0;

    puts("Divisible by 3");
    select_f64(20, (BLOCK_T(block_ctx_f64_bool)) &ctx.is_div_ctx);
    printf("opCount: : %i\n", ctx.common_ctx.opCount);

    puts("Primes");
    select_i32(20, (BLOCK_T(block_ctx_i32_bool)) &ctx.is_prime_ctx);
    printf("opCount: : %i\n", ctx.common_ctx.opCount);

    return 0;
}



// removing the void*

// lib types
typedef int b_i32_i32_pvt1(),
        b_i32_i32_pvt2(b_i32_i32_pvt1**, int),
        b_i32_i32_pvt3(b_i32_i32_pvt2**, int),
        b_i32_i32(b_i32_i32_pvt3**, int);

// lib fns
void each_i32_void(int n, b_i32_i32** block) {
    for (int i = 0; i < n; ++i) {
        ((b_i32_i32*) *block)(block, i);
    }
}

// sally locals
typedef struct {
    b_i32_i32 *_;
    int opCount;
} SallyBlock;

// instead of addOne(void *block, int n) we can now have:
int addOne(b_i32_i32** block, int n) {
    SallyBlock *ctx = (SallyBlock *) block;
    ctx->opCount++;
    printf("Result: %d\n", n + 1);
    return n + 1;
}


int main() {
    fred();  puts("");
    joe();

    SallyBlock s;
    s._ = addOne;
    s.opCount = 0;
    puts("");
    each_i32_void(3, (b_i32_i32**) &s);
    return 0;
}
