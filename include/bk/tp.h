#ifndef BK_TP_H
#define BK_TP_H "tp.h"

#include "stdio.h"
#include "mm.h"

// the internals of TP can be protected by

struct PTP {
    size sz;            // 8
    char *buf;          // 8
    struct MM *mm;      // 8
    size buf_size;      // 8
};

typedef struct tp_reserved {
    char reserved[sizeof(struct PTP)];
} TP;

union TP_PTP {
    TP *tp;
    struct PTP *ptp;
};

tdd void tp_init(TP *, size initSz, struct MM *);
tdd void tp_free(TP *);
tdd FILE *tp_open(TP *tp, char const *mode);
tdd s8 tp_getS8(TP *tp);


#endif      // BK_TP_H
