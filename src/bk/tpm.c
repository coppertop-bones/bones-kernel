#ifndef __BK_TP_C
#define __BK_TP_C "bk/tp.c"

#include "../../include/bk/kernel.h"


pub struct TPM * TPM_create(struct MM *mm) {
    struct TPM *tp = (struct TPM *) mm->malloc(sizeof(struct TPM));
    tp->all_strings = mm->malloc(sizeof(Buckets));
    initBuckets(tp->all_strings, 1);
    checkpointBuckets(tp->all_strings, &tp->cp);
    return tp;
}

pub int TPM_trash(struct TPM *tp) {
    freeBuckets(tp->all_strings->first_bucket);
    free(tp);
    return 0;
}

// OPEN replace FILE * with a handle
pub FILE * tpm_start(struct TPM *tp) {
    FILE* f = open_memstream(&tp->buf, (size_t*)&tp->size);
    return f;
}

pub s8 tpm_printf(struct TPM *tp, s8 format, ...) {
    FILE *f = tpm_start(tp);
    va_list args;
    va_start(args, format);
    fprintf(f, format.cs, args);
    va_end(args);
    return tpm_finish(tp, f);
}

pub void tpm_fprintf(struct TPM *tp, FILE *f, s8 format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(f, format.cs, args);
    va_end(args);
}

pub s8 tpm_finish(struct TPM *tp, FILE *f) {
    fclose(f);
    char *txt = allocInBuckets(tp->all_strings, tp->size, 1);
    strcpy(txt, tp->buf);
    s8 answer = (s8){tp->size, txt};
    free(tp->buf);
    tp->buf = 0;
    tp->size = 0;
    return answer;
}

pub void tpm_drop(struct TPM *tp) {
    resetToCheckpoint(tp->all_strings, &tp->cp);
}




#endif // __BK_TP_C
