#ifndef SRC_PT_C
#define SRC_PT_C

#include "minc.h"
#include "bk/qbe.h"

Buckets nodes;



// postfix_expression an expression that can take a postfix operation including,



PTNode * ptnode(int cat, int tok, PTNode *l, PTNode *r, int lineno) {
    PTNode *n = allocInBuckets(&nodes, sizeof *n, alignof(n));
    n->cat = cat;
    n->tok = tok;
    n->l = l;
    n->r = r;
    n->lineno = lineno;
    return n;
}

PTNode * leaf(int cat, int tok, int lineno) {
    return ptnode(cat, tok, 0, 0, lineno);
}

// ---------------------------------------------------------------------------------------------------------------------
// PARSE TREE NODE CONSTRUCTION
// ---------------------------------------------------------------------------------------------------------------------

PTNode * node(int tok, PTNode *l, PTNode *r, int lineno) {
    PTNode *n = allocInBuckets(&nodes, sizeof *n, alignof (n));
    n->tok = tok;
    n->l = l;
    n->r = r;
    n->lineno = lineno;
    return n;
}

PTNode * bindl(PTNode *n, PTNode *l, int lineno) {
    if (!n) return l;
    if (n->l != 0) {
        PP(parse, "bindl from @%d", lineno);
        die("node.l already bound");
    }
    n->l = l;
    return n;
}

PTNode * bindr(PTNode *n, PTNode *r, int lineno) {
    if (!n) return r;
    if (n->r != 0) {
        PP(parse, "bindr from @%d", lineno);
        die("2nd arg of fn already bound");
    }
    n->r = r;
    return n;
}

PTNode * bindlr(PTNode *n, PTNode *l, PTNode *r, int lineno) {
    if (n->l != 0) {PP(parse, "bindlr from @%d", lineno); die("n.l already bound");}
    if (n->r != 0) {PP(parse, "bindlr from @%d", lineno); die("n.r already bound");}
    n->l = l;
    n->r = r;
    return n;
}

PTNode * nodepp(int tok, PTNode *l, PTNode *r, int lineno, int level, char *msg, ...) {
    if (level & g_logging_level) {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
    return node(tok, l, r, lineno);
}

enum rCat {
    fn_apply,
};

typedef struct rnode {
    int typ;                // 4
    int nargs;              // 4
    int lineno;
    struct rnode * args[];  // nargs * 8
} RNode;

RNode * rnode(enum rCat typ, int nargs, RNode *l, RNode *r, int lineno) {
    RNode *n = allocInBuckets(&nodes, sizeof(RNode) + nargs * sizeof(RNode*), alignof(RNode));
    n->typ = typ;
    n->nargs = nargs;
    n->lineno = lineno;
    n->args[0] = l;
    n->args[1] = r;
    return n;
}

PTNode * call0(PTNode *fn, int lineno) {
    return node(OP_CALL, fn, 0, lineno);
}

PTNode * callN(PTNode *fn, PTNode *args, int lineno) {
    return node(OP_CALL, fn, args, lineno);
}
PTNode * apply2(enum tok fn, PTNode *arg1, PTNode *arg2, int lineno) {
    return ptnode(FN_APPLY, fn, arg1, arg2, lineno);
}


RNode * fn_arrayAt(RNode *ptr, RNode *index, int lineno) {
    // postfix_expression '[' expression ']'
    PTNode *n = node(FN_ADD, ptr, index, lineno);
    n = node(OP_DEREF, n, 0, lineno);
    return n;
}

PTNode * mkifelse(void *c, PTNode *t, PTNode *f, int lineno) {
    return node(IfElse, c, node(Else, t, f, lineno), lineno);
}

PTNode * mkfor(PTNode *ini, PTNode *tst, PTNode *inc, PTNode *s, int lineno) {
    PTNode *s1, *s2;
    if (ini)
        s1 = ini;
    else
        s1 = 0;
    if (inc) {
        s2 = inc;
        s2 = node(Seq, s, s2, lineno);
    } else
        s2 = s;
    if (!tst) {
        tst = node(LIT_INT, 0, 0, lineno);
        tst->s.u.n = 1;
    }
    s2 = node(While, tst, s2, lineno);
    if (s1)
        return node(Seq, s1, s2, lineno);
    else
        return s2;
}

PTNode * mkopassign(PTNode *op, PTNode *l, PTNode *r, int lineno) {
    if (op) {
        if (op->l != 0) die("n->l != 0 @ %d", __LINE__);
        if (op->r != 0) die("n->r != 0 @ %d", __LINE__);
        op->l = l;
        op->r = r;
        r = op;
    }
    return node(OP_ASSIGN, l, r, lineno);
}

PTNode * mktype(int tok, btypeid_t t, int lineno) {
    PTNode * n = node(tok, 0, 0, lineno);
    n->s.btyp = t;
    return n;
}

PTNode * appendR(PTNode * start, PTNode * next) {
    if (start) {
        PTNode * end = start;
        while (end->r) end = end->r;
        end->r = next;
        return start;
    } else
        return next;
}

PTNode * mkidentifierlist(PTNode * start, char * identifier, int lineno) {
    PTNode * next = node(pt_identifier_list, 0, 0, lineno);
    next->s.u.v = identifier;
    return appendR(start, next);
}

PTNode * mkinitdeclaratorlist(PTNode * start, PTNode * initdeclarator, int lineno) {
    PTNode * next = node(pt_init_declarator_list, initdeclarator, 0, lineno);
    return appendR(start, next);
}

PTNode * mkparametertypelist(PTNode * start, PTNode * parameterdeclarationOrELLIPSIS, int lineno) {
    PTNode * next = node(pt_parameter_type_list, parameterdeclarationOrELLIPSIS, 0, lineno);
    return appendR(start, next);
}

PTNode * mktypequalifierlist(PTNode * start, PTNode * typequalifier, int lineno) {
    PTNode * next = node(pt_type_qualifier_list, typequalifier, 0, lineno);
    return appendR(start, next);
}

PTNode * mkspecifierqualifierlist(PTNode * start, PTNode * specifierqualifier, int lineno) {
    PTNode * next = node(pt_specifier_qualifier_list, specifierqualifier, 0, lineno);
    return appendR(start, next);
}

PTNode * mkargumentexpressionlist(PTNode * start, PTNode * expr, int lineno) {
    PTNode * next = node(pt_argument_expression_list, expr, 0, lineno);
    return appendR(start, next);
}

PTNode * mkinitdeclarator(PTNode *declarator, PTNode *initializer, int lineno) {
    return node(pt_init_declarator, declarator, initializer, lineno);
}



// ---------------------------------------------------------------------------------------------------------------------
// PARSE TREE HELPERS
// ---------------------------------------------------------------------------------------------------------------------

int ptdeclarationspecifiersForExtern(PTNode *ds) {
    PTNode *n;
    n = ds->l;
    if (n && (n->tok == pt_storage_class_specifier) && (n->s.btyp == T_EXTERN)) return 1;
    return 0;
}

btypeid_t ptdeclarationspecifiersToBTypeId(PTNode *ds) {
    // OPEN: convert tokens to the correct hardcoded btyp enum
    enum tok op;  PTNode *n;  btypeid_t baseType = 0;  int hasSigned = 0, hasUnsigned = 0, hasConst = 0;
    n = ds->l;
    while (n) {
        op = (enum tok) n->s.btyp;
        switch (n->tok) {

            case pt_storage_class_specifier:
                switch (n->s.btyp) {
                    case T_TYPEDEF:
                        die("pt_storage_class_specifier, T_TYPEDEF should not be possible");
                    case T_EXTERN:
                        // ds( scs(), ds(ts() )
                        n = ds->r->l; // extern handled else where
                        break;
                    case T_STATIC:
                        nyi("pt_storage_class_specifier, T_STATIC");
                    case T_AUTO:
                        nyi("pt_storage_class_specifier, T_AUTO");
                    case T_REGISTER:
                        nyi("pt_storage_class_specifier, T_REGISTER");
                }
                break;

            case pt_type_specifier:
                switch (op) {
                    default:
                        nyi("op == %s @ %d", toktopp[op], __LINE__);
                        // OPEN handle long int and long long, and signed and unsigned
                    case T_VOID:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_VOID]);
                        baseType = B_VOID;
                        break;
                    case T_CHAR:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_CHAR]);
                        baseType = B_CHAR;
                        break;
                    case T_SHORT:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_I16]);
                        baseType = B_I16;
                        break;
                    case T_INT:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_I32]);
                        baseType = B_I32;
                        break;
                    case T_LONG:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_I64]);
                        baseType = B_I64;
                        break;
                    case T_FLOAT:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_F32]);
                        baseType = B_F32;
                        break;
                    case T_DOUBLE:
                        if (baseType != 0) die("2 base types encountered %s and then %s", btyptopp[baseType], btyptopp[B_F64]);
                        baseType = B_F64;
                        break;
                    case T_UNSIGNED:
                        if (hasUnsigned) die("unsigned already encountered before unsigned");
                        if (hasSigned) die("signed already encountered before unsigned");
                        hasUnsigned = 1;
                        break;
                    case T_SIGNED:
                        if (hasUnsigned) die("unsigned already encountered before signed");
                        if (hasSigned) die("signed already encountered before signed");
                        hasSigned = 1;
                        break;
                }
                n = n->r;
                break;

            case pt_type_qualifier:
                switch (op) {
                    default:  // T_RESTRICT, T_VOLATILE
                        nyi("op == %s @ %d", toktopp[op], __LINE__);
                    case T_CONST:
                        if (hasConst) die("const already encountered");
                        hasConst = 1;
                        n = n->r;
                        break;
                }
                break;

            case pt_function_specifier:
                if (op != T_INLINE) die("op != T_INLINE");
                nyi("pt_function_specifier");
                break;

            default:
                die("here");

        }
    }
    if (hasSigned) {
        switch (baseType) {
            case B_CHAR:
                return baseType = B_I8;
                break;
            case B_I16:
                baseType = B_I16;
                break;
            case 0:
            case B_I32:
                baseType = B_I32;
                break;
            case B_I64:
                baseType = B_I64;
                break;
            default:
                die("illegal type - signed %s @ &d", toktopp[op], __LINE__);
        }
    }
    if (hasUnsigned) {
        switch (baseType) {
            case B_CHAR:
                return baseType = B_U8;
                break;
            case B_I16:
                baseType = B_U16;
                break;
            case 0:
            case B_U32:
                baseType = B_U32;
                break;
            case B_U64:
                baseType = B_U64;
                break;
            default:
                die("illegal type - signed %s @ &d", toktopp[op], __LINE__);
        }
    }
    if (hasConst) nyi("const");
    return baseType;
}


unsigned int pointerise(btypeid_t btyp, PTNode *ptr, int isarray) {
    // OPEN check for const, volatile, restrict
    while (ptr) {
        assertTok(ptr, "ptr", pt_pointer, __LINE__);
        if (ptr->l->tok == T_PTR) {
            btyp <<= 8;
            btyp |= B_P;
        }
        ptr = ptr->r;
    }
    if (isarray) {
        btyp <<= 8;
        btyp |= B_P;
    }
    return btyp;
}


NameType * ptparametertypelistToParameters(PTNode * ptl) {
    NameType *start=0, *next, *prior=0;  PTNode *pd, *ds, *d, *id;  int is_array = 0;  btypeid_t t;
    if (!ptl) return NULL;
    for (; ptl; next->btyp=t, ptl=ptl->r, prior=next) {
        next = allocInBuckets(&nodes, sizeof (NameType), alignof (NameType));
        if (!start) start = next;
        if (prior) prior->next = next;
        assertTok(ptl, "ptl", pt_parameter_type_list, __LINE__);
        assertExists((pd=ptl->l), "ptl->l", __LINE__);
        switch (pd->tok) {
            case pt_parameter_declaration:
                assertExists((d=pd->r), "pd->r", __LINE__);
                assertTok(d, "d", pt_declarator, __LINE__);
                assertExists((id=d->r), "d->r", __LINE__);
                switch (id->tok) {
                    case IDENT:
                        break;
                    case pt_array:
                        is_array = 1;
                        id = id->l;
                        break;
                    default:
                        nyi("@ %d", __LINE__);
                }
                assertTok(id, "id", IDENT, __LINE__);
                next->name = id->s.u.v;
                assertExists((ds=pd->l), "pd->l", __LINE__);
                assertTok(ds, "ds", pt_declaration_specifiers, __LINE__);
                t = ptdeclarationspecifiersToBTypeId(ds);
                if (ptdeclarationspecifiersForExtern(ds)) die("illegal extern in parameter list");
                t = pointerise(t, d->l, is_array);
                break;
            case T_ELLIPSIS:
                nyi("ELLIPSIS @ %d", __LINE__);
                break;
            default:
                bug("ptparametertypelistToParameters @ %d", __LINE__);
        }
        assertTok(pd, "pd", pt_parameter_declaration, __LINE__);

    }
    return start;
}

int argNumOfEllipsis(PTNode *fd) {
    PTNode *ptl, *pd;  int i = 1;
    globals[next_oglo].i = 0;
    for (ptl = fd->r; ptl; ptl = ptl->r, i++) {
        assertTok(ptl, "fd->r", pt_parameter_type_list, __LINE__);
        assertExists(pd = ptl->l, "ptl->l", __LINE__);
        if (pd->tok == T_ELLIPSIS) {
            globals[next_oglo].i = i;
            break;
        }
        assertTok(pd, "pd", pt_parameter_declaration, __LINE__);
    }
    return i;
}

void process_function_definition(PTNode *ds, PTNode *d, PTNode* cs) {
    NameType *params = 0, *p;  btypeid_t tRet;  char *fnname;  int oglo;
    PP(emit, "process_function_definition");
    assertTok(ds, "ds", pt_declaration_specifiers, __LINE__);
    assertTok(d, "d", pt_declarator, __LINE__);
    tRet = ptdeclarationspecifiersToBTypeId(ds);
    if (ptdeclarationspecifiersForExtern(ds)) die("illegal extern in function definition");
    tRet = pointerise(tRet, d->l, 0);
    assertTok(d->r, "d->r", pt_func_def, __LINE__);
    assertExists(d->r->l, "d->r->l", __LINE__);
    assertTok(d->r->l, "d->r->l", IDENT, __LINE__);
    if (d->r->r) {
        assertTok(d->r->r, "d->r->r", pt_parameter_type_list, __LINE__);
        params = ptparametertypelistToParameters(d->r->r);
    }
    fnname = d->r->l->s.u.v;

    oglo = reserve_oglo();
    globals[oglo].styp = Fn;
    globals[oglo].i = argNumOfEllipsis(d->r);;
    globals[oglo].btyp = FUNC(tRet);
    globals[oglo].u.v = fnname;
    symadd(fnname, oglo, FUNC(tRet));
    for (p=params; p; p = p->next)
        symadd(p->name, 0, p->btyp);

    emitfunc(tRet, fnname, params, cs);
    symclr();
    tmp_seed = TMP_START;
}

PTNode * parseInitDeclPt(PTNode *id, btypeid_t baseType, int isExtern) {
    PTNode *d, *decl, *fd, *d2;  char *name;  btypeid_t btyp, btypFn;  int isPtrToFn=0;
    assertTok(id, "id", pt_init_declarator, __LINE__);
    assertExists(d = id->l, "d", __LINE__);
    assertTok(d, "d", pt_declarator, __LINE__);
    assertExists(d->r, "d->r", __LINE__);
    switch (d->r->tok) {

        case IDENT:
            // variable declaration - local or global - optionally with init, not added to symbol table
            // OPEN: handle brackets e.g. int (*p);
            name = d->r->s.u.v;
            btyp = pointerise(baseType, d->l, 0);      // OPEN: handle array
            if (btyp == B_VOID) die("invalid void declaration for %s", name);
            decl = node(DeclVar, d->r, id->r, __LINE__);
            if (isExtern) {
                btyp = tm_interv(_bk->tm, 2, btyp, B_EXTERN, 0);
                decl->s.styp = Ext;     // will be added to globals later
            }
            else
                decl->s.styp = Var;     // changed later to Glo if it turns out that declaration is global
            decl->s.btyp = btyp;
            decl->s.u.v = name;
            symadd(name, 0, btyp);
            return decl;

        case pt_func_def:
            // function declaration - can only be global so is added to symbol table
            fd = d->r;
            if (fd->l->tok == pt_LP_declarator_RP) {
                // OPEN: for mo just catch pointer to function, parse properly later
                assertMissing(fd->l->r, "fd->l->r", __LINE__);
                assertExists(fd->l->l, "fd->l->l", __LINE__);
                assertTok(d2=fd->l->l, "fd->l->l", pt_declarator, __LINE__);
                assertExists(d2->l, "d2->l", __LINE__);
                assertExists(d2->r, "d2->r", __LINE__);
                assertTok(d2->l, "d2->l", pt_pointer, __LINE__);
                assertTok(d2->r, "d2->r", IDENT, __LINE__);
                name = d2->r->s.u.v;
                isPtrToFn = 1;
            }
            else {
                name = fd->l->s.u.v;
            }
            btyp = pointerise(baseType, d->l, 0);      // OPEN: handle array, e.g. char *[] fred();
            btypFn = FUNC(btyp);
            if (isExtern) {
                btypFn = tm_interv(_bk->tm, 2, btypFn, B_EXTERN, 0);
                globals[next_oglo].styp = Ext;
            }
            else if (isPtrToFn) {
                btypFn = tm_interv(_bk->tm, btypFn, B_P, 0);
                globals[next_oglo].styp = Glo;
            }
            else
                globals[next_oglo].styp = Fn;
            globals[next_oglo].btyp = btypFn;
            globals[next_oglo].u.v = name;
            globals[next_oglo].i = argNumOfEllipsis(fd);
            symadd(name, reserve_oglo(), btypFn);
            return 0;

        default:
            bug(MINC_MINC99_Y ">>parseInitDeclPt @ %d", __LINE__);
            return 0;
    }
}


void process_external_declaration(PTNode *decls) {
    PP(emit, "process_external_declaration");
    PTNode * decl;
    if (!decls) return;
    assertTok(decls, "decls", DeclVars, __LINE__);
    for (; decls; decls=decls->r) {
        decl = decls->l;
        globals[next_oglo].styp = (decl->s.styp == Var) ? Glo : decl->s.styp;
        globals[next_oglo].btyp = decl->s.btyp;
        globals[next_oglo].u.v = decl->s.u.v;
        symset(decl->s.u.v, reserve_oglo(), decl->s.btyp);
    }
}


PTNode * mkDeclarations(PTNode *ds, PTNode *idl, int lineno) {
    btypeid_t baseType;  PTNode *start=0, *next, *current, *id, *decl;  int isExtern=0;
    assertTok(ds, "ds", pt_declaration_specifiers, __LINE__);
    baseType = ptdeclarationspecifiersToBTypeId(ds);
    isExtern = ptdeclarationspecifiersForExtern(ds);
    assertTok(idl, "idl", pt_init_declarator_list, __LINE__);
    for (; idl; idl=idl->r) {
        assertTok(idl, "idl", pt_init_declarator_list, __LINE__);
        assertExists(id=idl->l, "id", __LINE__);
        if (decl=parseInitDeclPt(id, baseType, isExtern)) {
            next = node(DeclVars, decl, 0, lineno);
            if (!start) current = start = next;
            else {
                current->r = next;
                current = next;
            }
        }
    }
    return start;
}


#endif  // SRC_PT_C