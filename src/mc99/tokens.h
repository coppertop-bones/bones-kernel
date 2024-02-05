#ifndef INCLUDE_TOKENS_H
#define INCLUDE_TOKENS_H


enum tok {

    MISSING = 0,

    // type tokens
    T_TYPE_NAME,
    T_TYPEDEF,
    T_EXTERN,
    T_STATIC,
    T_AUTO,
    T_REGISTER,
    T_STRUCT,
    T_UNION,

    T_CONST,
    T_RESTRICT,
    T_VOLATILE,
    T_INLINE,

    T_VOID,
    T_CHAR,
    T_SHORT,
    T_INT,
    T_LONG,

    T_FLOAT,
    T_DOUBLE,
    T_SIGNED,
    T_UNSIGNED,
    T_BOOL,
    T_COMPLEX,
    T_IMAGINARY,

    T_PTR,
    T_ELLIPSIS,


    // expressions
    LIT_CHAR,           // e.g. '\n' OPEN: add these to lexer
    LIT_INT,
    LIT_DEC,
    LIT_STR,
    LIT_BOOL,

    OP_CALL,

    OP_IIF,             // ? :
    OP_TF,              // ditto
    OP_AND,
    OP_OR,

    OP_BINV,            // ~ need to use xor
    OP_NOT,             // !
    OP_ATTR,            // e.g. x.name

    IDENT,
    OP_ADDR,            // &
    OP_DEREF,           // *

    OP_INC,
    OP_DEC,
    OP_ASSIGN,
    OP_NEG,             // -

    // base types - w is word, l is long, s is single, d is double
    // extended types - b is byte, h is half word (for aggregate types and data defs)
    // T is wlsd, I is wl, F is sd, m is pointer (on 64-bit architectures it is the same as l)
    // simple binary expressions
    OP_ADD,             // addT
    OP_SUB,             // subT
    OP_MUL,             // mulT
    OP_DIV,             // divT, udivT
    OP_MOD,             // udivI, remI, uremI
    OP_LSHIFT,          // shlI
    OP_RSHIFT,          // sarI, shrI

    OP_EQ,              // ceqT
    OP_NE,              // cneT
    OP_LE,              // csleI, csgeI, culeI, cugeI, cleF, cgeF
    OP_LT,              // csltI, csgtI, cultI, cugtI, cltF, cgtF

    OP_BAND,            // andI
    OP_BOR,             // orI
    OP_BXOR,            // xorI


    // statements
    Label,
    If,
    IfElse,
    Else,
    While,
    // For,               // for is implemented in terms of while
    Select,
    Case,
    Default,
    Goto,
    Continue,
    Break,
    Ret,
    Seq,
    Do,
    DeclVar,
    DeclVars,


    // parse tree construction
    pt_func_def,
    pt_declaration,
    pt_parameter_type_list,
    pt_argument_expression_list,
    pt_declarator,
    pt_abstract_declarator,
    pt_identifier_list,
    pt_init_declarator_list,
    pt_init_declarator,
    pt_parameter_declaration,
    pt_declaration_specifiers,
    pt_type_specifier,
    pt_storage_class_specifier,
    pt_type_qualifier,
    pt_pointer,
    pt_type_qualifier_list,
    pt_type_name,
    pt_array,
    pt_specifier_qualifier_list,
    pt_function_specifier,
    pt_LP_declarator_RP,
    pt_struct_or_union,

    // functions
    FN_ARRAY_AT, FN_ADD, FN_APPLY,
};


// PP nodes
static char *toktopp[] = {
        [LIT_CHAR] = "LIT_CHAR",        [LIT_INT] = "LIT_INT",          [LIT_DEC] = "LIT_DEC",          [LIT_STR] = "LIT_STR",
        [LIT_BOOL] = "LIT_BOOL",
        [OP_CALL] = "OP_CALL",          [OP_ADD] = "OP_ADD",            [OP_SUB] = "OP_SUB",            [OP_MUL] = "OP_MUL",
        [OP_DIV] = "OP_DIV",            [OP_MOD] = "OP_MOD",            [OP_LSHIFT] = "OP_LSHIFT",      [OP_RSHIFT] = "OP_RSHIFT",
        [OP_EQ] = "OP_EQ",              [OP_NE] = "OP_NE",              [OP_LE] = "OP_LE",              [OP_LT] = "OP_LT",
        [OP_AND] = "OP_AND",            [OP_OR] = "OP_OR",              [OP_NOT] = "OP_NOT",            [OP_BAND] = "OP_BAND",
        [OP_BOR] = "OP_BOR",            [OP_BINV] = "OP_BINV",          [OP_BXOR] = "OP_BXOR",          [OP_IIF] = "OP_IIF",
        [OP_TF] = "OP_TF",              [IDENT] = "IDENT",              [OP_ATTR] = "OP_ATTR",
        [OP_ADDR] = "OP_ADDR",          [OP_DEREF] = "OP_DEREF",        [OP_INC] = "OP_INC",            [OP_DEC] = "OP_DEC",
        [OP_ASSIGN] = "OP_ASSIGN",      [OP_NEG] = "OP_NEG",
        [If] = "If",                    [IfElse] = "IfElse",            [Else] = "Else",                [While] = "While",
        [Select] = "Select",            [Case] = "Case",                [Default] = "Default",          [Goto] = "Goto",
        [Continue] = "Continue",        [Break] = "Break",              [Ret] = "Ret",                  [Seq] = "Seq",
        [Do] = "Do",                    [DeclVar] = "DeclVar",          [DeclVars] = "DeclVars",
        [T_TYPE_NAME] = "T_TYPE_NAME",  [T_TYPEDEF] = "T_TYPEDEF",      [T_EXTERN] = "T_EXTERN",
        [T_STATIC] = "T_STATIC",        [T_AUTO] = "T_AUTO",            [T_REGISTER] = "T_REGISTER",    [T_STRUCT] = "T_STRUCT",
        [T_UNION] = "T_UNION",          [T_CONST] = "T_CONST",          [T_RESTRICT] = "T_RESTRICT",    [T_VOLATILE] = "T_VOLATILE",
        [T_INLINE] = "T_INLINE",        [T_VOID] = "T_VOID",            [T_CHAR] = "T_CHAR",            [T_SHORT] = "T_SHORT",
        [T_INT] = "T_INT",              [T_LONG] = "T_LONG",            [T_FLOAT] = "T_FLOAT",          [T_DOUBLE] = "T_DOUBLE",
        [T_SIGNED] = "T_SIGNED",        [T_UNSIGNED] = "T_UNSIGNED",    [T_BOOL] = "T_BOOL",            [T_COMPLEX] = "T_COMPLEX",
        [T_IMAGINARY] = "T_IMAGINARY",  [T_PTR] = "T_PTR",              [T_ELLIPSIS] = "T_ELLIPSIS",
        [pt_func_def] = "pt_func_def",                                  [pt_declaration] = "pt_declaration",
        [pt_parameter_type_list] = "pt_parameter_type_list",            [pt_argument_expression_list] = "pt_argument_expression_list",
        [pt_declarator] = "pt_declarator",                              [pt_abstract_declarator] = "pt_abstract_declarator",
        [pt_identifier_list] = "pt_identifier_list",                    [pt_init_declarator_list] = "pt_init_declarator_list",
        [pt_init_declarator] = "pt_init_declarator",                    [pt_parameter_declaration] = "pt_parameter_declaration",
        [pt_declaration_specifiers] = "pt_declaration_specifiers",      [pt_type_specifier] = "pt_type_specifier",
        [pt_storage_class_specifier] = "pt_storage_class_specifier",    [pt_type_qualifier] = "pt_type_qualifier",
        [pt_pointer] = "pt_pointer",                                    [pt_type_qualifier_list] = "pt_type_qualifier_list",
        [pt_type_name] = "pt_type_name",                                [pt_specifier_qualifier_list] = "pt_specifier_qualifier_list",
        [pt_function_specifier] = "pt_function_specifier",              [pt_LP_declarator_RP] = "pt_LP_declarator_RP",
        [pt_struct_or_union] = "pt_struct_or_union",
        [FN_ARRAY_AT] = "FN_ARRAY_AT",  [FN_ADD] = "FN_ADD",
};



#endif  // INCLUDE_TOKENS_H