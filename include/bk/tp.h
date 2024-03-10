// ---------------------------------------------------------------------------------------------------------------------
// TP - TEXT PAD
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BK_TP_H
#define BK_TP_H "tp.h"
#define BK_TP_H "tp.h"

#include "stdio.h"
#include "mm.h"


typedef struct {
    Buckets *buckets;   // 8
    char *buf;          // 8
    size buf_sz;        // 8
    size32 start;       // 4
    size32 end;         // 4
} BK_TP;


pub void TP_init(BK_TP *, size, Buckets *);
pub void TP_free(BK_TP *);
//tdd FILE *tp_open(BK_TP *, char const *mode);        // macos and linux only I think - not obvious how to catch the flush on windows even it maps to a mm file
//pub S8 tp_pp(BK_TP *, char const *format, ...);
pub TPN tp_printf(BK_TP *, char const *format, ...);
pub void tp_buf_printf(BK_TP *, char const *format, ...);
pub TPN tp_buf_flush(BK_TP *);
pub S8 tp_s8(BK_TP *, TPN);
pub int tp_sizeof(TPN);
pub int tp_render(TPN, S8);


#endif      // BK_TP_H
