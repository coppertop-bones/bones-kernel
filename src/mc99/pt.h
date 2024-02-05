#ifndef INCLUDE_PT_H
#define INCLUDE_PT_H

#include "bk/tm.h"
#include "tokens.h"


typedef struct _csym {      // 20
    enum {                  // 4
        Con = 1,            // constant - integer or double with type btyp
        Str,                // literal string
        Tmp,                // qbe temporary - hidden from user
        Var,                // local variable, with type btyp - named
        Glo,                // global variable, with type btyp - numbered
        Ext,                // external variable or function, with type btyp, named
        Fn,                 // function, with type btyp - either a forward declare or definition
    } styp;
    btypeid_t btyp;         // 4 (upto ***<type>)
    union {                 // 8
        long n;             // styp defined - integer constant, temp variable ordinal
        char *v;            // string constant, name of fn or variable (and in short term structs, unions, typedefs etc) OPEN: change v to name
        double d;           // double constant
    } u;
    int i;                  // 4 - styp defined - global string number, iEllipsis
} Symb;


// list of name types
typedef struct NameType {
    char *name;             // 8
    struct NameType *next;  // 8
    btypeid_t btyp;         // 4
} NameType;


// see https://peps.python.org/pep-3123/

typedef struct PTNode {       // 48 bytes
    struct PTNode *l, *r;     // 8 + 8
    Symb s;                 // 20
    enum tok cat;           // 4
    enum tok tok;           // 4
    unsigned int lineno;    // 4
} PTNode;

#endif INCLUDE_PT_H
