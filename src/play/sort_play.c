#include "../bk/pp.c"
#include "../lib/radix.c"

pvt void die_(char *preamble, char *msg, va_list args) {
    fprintf(stderr, "%s", preamble);
    vfprintf(stderr, msg, args);
    exit(1);
}

//KSORT_INIT_GENERIC(int)
KRADIX_SORT_INIT(u32, unsigned int, , sizeof(unsigned int))

void counting_sort(int src[], int out[], int N, unsigned int tally[], int tallyN) {
    int i;
    for(i = 1; i <= N; i++) tally[src[i]]++;                        /* tally */
    for(i = 1; i <= tallyN; i++) tally[i] = tally[i-1] + tally[i];  /* cumulate tally */
    for(i = 1; i <= N; i++) {                                       /* stable but not in place */
        out[tally[src[i]]] = src[i];
        tally[src[i]]--;
    }
}


int main() {
    int N, i;
//    unsigned int xs[] = {10,9,8,7,6,5,4,3,2,1};
//    N = 10;
//    ks_combsort(int, N, xs);
//    for (i=0; i < N; i++) PP(info, "%i", xs[i]);
//
//    unsigned int ys[] = {10,9,8,7,6,5,4,3,2,1};
//    N = 10;
//    ks_introsort(int, N, ys);
//    for (i=0; i < N; i++) PP(info, "%i", ys[i]);
//
//    unsigned int zs[] = {10,9,8,7,6,5,4,3,2,1};
//    N = 10;
//    ks_combsort(int, N, zs);
//    for (i=0; i < N; i++) PP(info, "%i", zs[i]);

    unsigned int as[] = {9,9,8,7,6,5,4,3,2,1};
    unsigned int *p = &as[0];
    N = 10;
    ks_radix_sort(u32, p + 1, p[0]);
    for (i=0; i < N; i++) PP(info, "%i", as[i]);

//    // COUNTING SORT
//    int a[] = {0,1,5,0,12,43,22,13,32};
//    N = sizeof(a)/sizeof(int) - 1; // 9
//    int nTally = 44;
//
//    // allocate zeroed tally array
//    unsigned int *tally = (unsigned int*) malloc(sizeof(unsigned int) * nTally);        // calloc
//    for(i = 0; i < nTally; i++) tally[i] = 0;                                           // use memset instead
//
//    int *dest = (int*) malloc(sizeof(int) * N);
//
//    counting_sort(a, dest, N, tally, nTally);
//    for (i=0; i < N; i++) PP(info, "%i", dest[i]);

    // FAST MEDIAN
    // https://attractivechaos.wordpress.com/2008/09/13/calculating-median/
    // http://ndevilla.free.fr/median/median/src/quickselect.c

//    ks_mergesort(int, N, array, 0);
//
//    ks_heapmake(int, N, array);
//    ks_heapsort(int, N, array);

    PP(info, "passed");
    return 0;
}

//
///*
// *  This Quickselect routine is based on the algorithm described in
// *  "Numerical recipes in C", Second Edition,
// *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
// *  This code by Nicolas Devillard - 1998. Public domain.
// */
//
//
//#define ELEM_SWAP(a,b) { register elem_type t=(a);(a)=(b);(b)=t; }
//
//elem_type quick_select(elem_type arr[], int n) {
//    int low, high, median, middle, ll, hh;
//    low = 0 ; high = n-1 ; median = (low + high) / 2;
//    for (;;) {
//        if (high <= low) return arr[median];    /* One element only */
//        if (high == low + 1) {                  /* Two elements only */
//            if (arr[low] > arr[high])
//            ELEM_SWAP(arr[low], arr[high]) ;
//            return arr[median] ;
//        }
//
//        /* Find median of low, middle and high items; swap into position low */
//        middle = (low + high) / 2;
//        if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
//        if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
//        if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;
//
//        /* Swap low item (now in position middle) into position (low+1) */
//        ELEM_SWAP(arr[middle], arr[low+1]) ;
//
//        /* Nibble from each end towards middle, swapping items when stuck */
//        ll = low + 1;
//        hh = high;
//        for (;;) {
//            do ll++; while (arr[low] > arr[ll]) ;
//            do hh--; while (arr[hh]  > arr[low]) ;
//
//            if (hh < ll)
//                break;
//
//            ELEM_SWAP(arr[ll], arr[hh]) ;
//        }
//
//        /* Swap middle item (in position low) back into correct position */
//        ELEM_SWAP(arr[low], arr[hh]) ;
//
//        /* Re-set active partition */
//        if (hh <= median) low = ll;
//        if (hh >= median) high = hh - 1;
//    }
//}
//
//#undef ELEM_SWAP
//
//
//type_t ks_ksmall(size_t n, type_t arr[], size_t kk) {
//    type_t *low, *high, *k, *ll, *hh, *middle;
//    low = arr; high = arr + n - 1; k = arr + kk;
//    for (;;) {
//        if (high <= low) return *k;
//        if (high == low + 1) {
//            if (cmp(*high, *low)) swap(type_t, *low, *high);
//            return *k;
//        }
//        middle = low + (high - low) / 2;
//        if (lt(*high, *middle)) swap(type_t, *middle, *high);
//        if (lt(*high, *low)) swap(type_t, *low, *high);
//        if (lt(*low, *middle)) swap(type_t, *middle, *low);
//        swap(type_t, *middle, *(low+1)) ;
//        ll = low + 1; hh = high;
//        for (;;) {
//            do ++ll; while (lt(*ll, *low));
//            do --hh; while (lt(*low, *hh));
//            if (hh < ll) break;
//            swap(type_t, *ll, *hh);
//        }
//        swap(type_t, *low, *hh);
//        if (hh <= k) low = ll;
//        if (hh >= k) high = hh - 1;
//    }
//}
