
#include <stdio.h>

#include "../qu/mt.c"


int main() {
    int i;
    unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    qu_mt_init_array(init, 4);
    printf("1000 outputs of qu_mt_u64()\n");
    for (i=0; i<1000; i++) {
        printf("%20llu ", qu_mt_u64());
        if (i%5==4) printf("\n");
    }
    printf("\n1000 outputs of qu_mt_u64()\n");
    for (i=0; i<1000; i++) {
        printf("%10.8f ", qu_mt_f64_2());
        if (i%5==4) printf("\n");
    }
    return 0;
}
