

# block without parameter

# block with parameter

# function

# get value, overloads, family

# set value, function

# call
Python
C
etc

# early return - ^

# signal - ^^

# lit

# seq

# type-lang

# coerce

# check type

# load

# import



a.b: ()
a.c[2]: `fred      <<< struct and seq access
a[`c].2: `joe      <<< map and tuple access

so namespaces effectively look like structs

(c, d): a.c


local, ctx, module, global are parser / compiler

loops
goto
gosub

fn calling - python, c

ref - answers a slot
cowref - replaces the slot in the container with a new copy if necessary before answering the slot
get - answers a value out of a slot
put - replaces a value in a slot

object navigation, cow copying 


x1 = ref(scope, 'a')
x2 = ref(x1, 'b')
put(x2, emptylist)
x3 = ref(x1, 'c')
x4 = ref(x3, 2)
put(x4, `fred)

lit
seq

struct.ref(struct, name)
e.g. a.b  =>  struct.ref(a, `b)

values can be stored in registers or bytes

bones doesn't allow access to pointers

c does
ref 

how do we copy protect the number 3 in   (1,2,3)[3]
in when doing cow any address of a  you handout must be const

struct { a   b   c}
         1   2   3
a.b: x will change the value at slot 2 / b



c has blocks but no subroutines
functions - blocks governed by an abi

block and block-closure
function and function-closure

stack memory management - fastest allocation, results must be evacuated, good locality, per thread
region - fast allocation, results do not need to be evacuated, needs type info, good locality
pools - poor locality
regions can be compacted / evacuated

stack vs heap is not a great dictotomy
stack vs non-stack is better as there are many subtypes of non-stack

closures could be on stack as long as they cannot be used after their defining scope is exited

bk makes use of global append only


bones reduces the number of decisions exposed to the programmer - overloads

bones / c variables are translated into QBE (MIR, LLVM) vars in the generator
so the RST must have it's own model of variables

C only needs to have global variables and local variables as it has not closures
bones is similar but also may have module and context variables

for loops
if else
switch / case i.e. pattern match


in bones blocks you can pass arguments in
in c blocks you can't


rst_for
rst_for_i
rst_for_each
rst_for_ieach
rst_for_k
rst_for_v
rst_for_kv
rst_if
rst_ifelse
rst_match(expr, n, patterns, blocks, default)


terms:
    block - https://en.wikipedia.org/wiki/Control-flow_graph
    cmodule - a file (post pre-processer) / the resulting lib
    

for cmodule

variables
external variables
public global variables
private global variables
block variables (function args and locals are just block variables) {} define a block

private functions
public functions

to QBE gen a function a symbol table is passed on the stack - new symbols may be pushed onto the symtab

the bones symbol table
- has 4 slots - global, module, context and local (args and local vars)

RST - 5 name type slots - bglobal, bmodule, bctx, blocals, cstyle

so no support for closures

an RST variable is {type, name}


- can be defined globally - location known statically 
block vars - override vars in parent block
function vars - defined on stack

bones
global key value store
no static global variables



bool ifTrue: []
bool ifTrue: [] ifFalse: []
for
```
[init] value
[cond] whileTrueDo: [] followedBy: [postDo]
```
value switch: ()

langlib
    for
    deref




in c a file compiles to a lib (a.o, etc)
static int fred; means fred is only available inside it's file
int joe however is visible outside



FE => RST chunks + module => 

RST is bones typed


symbol table must be passed on stack


composition - needs side band memory management
overloads reduce effort to compose
type inference further reduces effort
exceptions as well as unions - can be more efficient

can we infer exceptions and unions?

bindto
deepbindto
getmutables


in qbe a deep lval cannot be a local but can be a temp
but a shallow lval can be

same for rval



getfn(name)
getoverload
getfamily

apply(fn, args)




    n2 = mklitint(&a, 1);
    n3 = mkbindto(&a, n2, locals, "a");

    n4 = mkget(&a, locals, "a");
    n5 = mklitint(&a, 1);
    n6 = mkgetoverload(&a, fns, "add", 2);
    n7 = mkapply(&a, n6, 2, n4, n5);
    n8 = mkbindto(&a, n7, locals, "a");

    n9 = mkget(&a, locals, "a");
    n10 = mkret(&a, n9, locals);

    n11 = mkseq(&a, 4, n1, n3, n8, n10);