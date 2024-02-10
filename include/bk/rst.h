// ---------------------------------------------------------------------------------------------------------------------
// RST - REDUCED SYNTAX TREE
// ---------------------------------------------------------------------------------------------------------------------

#ifndef INC_BK_RST_H
#define INC_BK_RST_H "bk/rst.h"

#include "bk.h"
#include "hi.h"
#include "mm.h"


typedef u16 tokid_t;

enum scope {
    local = 1,
    global,
    module,
    contexual,
    consts,
    fn,         // have to let inputs be rebound else we force copy on write so args are not a separate scope
    cstyle,
};


typedef enum {
    RST_ADD = 1,
    RST_SUB,
    RST_MUL,
} rst_op;


typedef struct {
    int i;
} RstSymTab;


typedef struct {
    enum scope scope;
    symid_t symid;
} rstname_t;


typedef struct {
    symid_t symid;      // 4
    tokid_t tokid;      // 2 might be even possible to be a u8
    u16 srcfileid;      // 2
    u16 s1;             // 2 slice into src
    u16 s2;             // 2
    u16 l1;             // 2 line number in src
    u16 l2;             // 2
} RstToken;


void rst_symget(BK_SymTab st, rstname_t);



#endif // INC_BK_RST_H