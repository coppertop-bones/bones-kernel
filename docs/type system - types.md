This doc describes the bones types system



# Bones type system

In bones, we can think of a type as just a label that is used by the type system to check program consistency,
generate code from a pro forma (aka a template) and to manage memory. This label or type is attached to a variable name 
in the building process, at runtime to track objects in a union, and in memory management meta data. The tag is 
needed for a garbage collection style termed conservative on stack, precise on heap. Side note: we allow conservative 
escape hatches on the heap to handle things like C unions, and pin both conservatively reference objects and objects 
shared outside of the kernel.

Every type has a unique identity, used at build time for inference, checking, code generation and compilation, etc,
at runtime to distinguish types in a union which cannot be statically determined, and by the kernel for precise memory
management.


## Metatypes

We have the following sorts of types: 
nominal - an atomic (can't be broken down into smaller parts) type that is identified by a name \
intersection, union - subtype relationships \
tuple, struct - aggregate (aka product) types \
sequence, map, function - arrow (aka exponential) types 

Any type that can recursively reach its own definition is flagged as such.

Types can be grouped into exclusive sets where types of a particular exclusion may only appear once in an intersection.
For example, the exclusive set "memory" (aka "concrete type" by Wikipedia) is used to prevent the creation of 
intersections like Int & Double. Other examples include, finance / business types such as Currency (possibly with 
subtypes GBP, USD, etc) and FX (GBPUSD, USDEUR, USDJPY, etc) and physical types such as feet, inches, cm, hour, 
minute, second, weekday, etc.


## The subtype relation

Intuitive subtyping rules are defined in the behaviour of the `<:` operation - which as well as answering if type A fits 
within type B can also measure the distance between types.

The following uses `&` for intersection, `+` for union, `(A, B)` for tuple, `{fred:A, joe:B}` for a struct with fields 
fred and joe. Memory based error types are common so we differentiate them from functions. We use `A->B` for a function 
from A to B, `{fn1, fn2}` as a set of functions, aka an overload. We use `N**A` for a sequence of A, i.e. a map from 
ordinal to A, and `A**B` for a map from A to B. To define a type A we use `A: <type expression>`.


### Nominal, relationship and aggregate subtyping
```
        A: 'A'
        B: 'B'

         A   <:  A      - true
         A   <:  B      - false

        A&B  <:  A      - true
        A    <:  A&B    - false

        A    <:  A+B    - true
        A+B  <:  A      - false


(A,A&B)      <:  (A+B,B)            all elements must fit within
(A,A)        <:  (A,A) + (B,B)
(A,A)&(B,B)  <:  (A,A)


(A+B) + (C+D) == A+B+C+D
(A&B) & (C&D) == A&B&C&D
(A&B) + (C&D) == (A&B) + (C&D) 
(A&B) + (A&D) == A & (B + D)
(A+B) & (C+D) == A+C & A+D & B+C & B+D
```


E.g. to model integers and natural numbers, given
```
int: 'int'
```
we could either use a dummy type ascribing naturalness
```
_nat: 'naturalness'
nat: _nat & int
```
or use a recursive definition
```
nat: nat & int
```


### arrows

In addition to each element of an arrow's keys a subtype relationship can only exist for arrows with the same number 
of types in their keys i.e. (int, int) -> is not a subtype of (int) -> int even though structurally we might consider 
(int, int) a subtype of (int).


#### checking function application

When checking a function call all LHS args must fit within parameters of the function, and the result of the function
must fit within the name being bound to. I.e.

```
fn: <:(A) -> C&D>
fred: <:C+D>
joe: <:D>
sally: <:E>
a: <:A>
b: <:B>

fred: fn(a)    - always okay
fred: fn(b)    - not okay since fn cannot take a B
joe: fn(a)     - sometimes okay, i.e. as long as fn(A) returns a D and not a C
sally: fn(a)   - not okay since sally can't be a C or D
```

Another way of saying this is that the call-site must fit within the function.

```
callsite: A&B -> C+D
fn: A -> C

callsite <: fn

A&B <: A
C <: C+D
```

I.e. the arguments of the call site must fit within the parameters of the function and the return type of the 
function must fit within that of the call site.


### exclusive types

Exclusive types merely help to declutter the type system and introduce no additional semantics.


### implicit types

char const *

char & const & ptr

we want
a: <:char*>
fn: strlen(<:char const *>)
fn(a) to be valid

i.e. the non-const is a sub type of const

what we are really saying is that everything be default is non-const, explicity const, and non-const <: const.

we define non-const as implicit and const and const and define that non-const is implicit

```
const: 'const'
nonconst: nonconst & const && implicit  // non-const is a recursive intersection with the implicit property (const is tagged as being related to an implicit type)

const <: nonconst   false
nonconst <: nonconst   false
char  <: const & char   true  - 
```

Initially we see `char  <: const & char` as false but when we check the RHS residual `const` we see it is in an 
intersection relationship with the implicit type non-const and since `non-const <: const` the overall expression passes.

Implicit types merely help to declutter the type system and introduce no additional semantics.


#### overloads

An overload is a set of functions and its type looks like an intersection when it is passed as an argument, e.g.

```
fn1: (A) -> C 
fn2: (B) -> D
ov: {fn1, fn2}
fn: (ov) -> ...

fn( (A) -> C )   - okay
fn( (B) -> D )   - okay
fn( (A) -> D )   - not okay
fn( (B) -> C )   - not okay
```

But like a union when selecting the function to apply, e.g.

```
callsite: (A&B, C) -> E+F
fn1: (A,C) -> E
fn2: (B,D) -> F
ov: {fn1, fn2}

callsite <: fn1  - true
callsite <: fn2  - false
```

It can be helpful to compute an upper bound on the overload's type. Noting the return type is the union of each 
function's return type and similarly for each function's arguments we have:

```
fn1: (A,C) -> E
fn2: (B,D) -> F
ov: {fn1, fn2}
ov: (A,C) & (B,D) -> E+F
upperbound: (A+B, C+D) -> E+F

ov <: upperbound
```

Knowing some types at the call site, can help tighten the upper-bound further, as possible function selections get
eliminated. Anecdotally, according to the Julia community, this usually leads to a single function in the overload set.


## schema variables

it's not very helpful defining add(GBP, GBP) -> GBP as GBP has no memory representation.

add(GBP&T1, GBP&T1) -> GBP&T1 defines a whole set of functions termed a template, where T1 could be a float, a 
fraction, even a roman numeral.



## Inference

Bones does not intend to eliminate the need for runtime dispatch / determination of type in all unannotated programs 
but to reduce the possible set to the minimum and additionally provide tools to allow a programmer to reduce that set 
to one possible type by using partial annotation. 



NOTES

// is a tuple of 1 element the same type as the element?
// no
// 1) tuple unpacking should be clear e.g. (A): fn() is not the same as A: fn()
// 2) tuples can be indexed e.g. fn()[1]




To do a X <: Y we partition the two types into three sets:

- X intersect Y' - anything here then it's not a fit
- X intersect Y  - common stuff, if we only have common stuff then it's an exact fit
- X' intersect Y - stuff not in X but in Y - we term this the residual

X & Y' = X - X & Y
X & Y
X' & Y = Y - X & Y

| X    <:   Y   | fitsWithin | X - X&Y | X&Y | Y - X&Y |
|---------------|------------|---------|-----|---------|
| A    <:   A   | exact      | {}      | A   | {}      |
| A    <:   B   | false      | A       | {}  | B       |
| A&B  <:   A   | true       | {}      | A&B | A - A&B |
| A    <:   A&B | false      | A - A&B | A&B | {}      |
| A    <:   A+B | true       | {}      | A   | B       | 
| A+B  <:   A   | false      | B       | A   | {}      |

C = A&B
D 
C <: C + D


{} - A = {} we have no negative numbers with sets or rather this implies A = {}


C = A&B
C + D = A
D = A - C
D = A - A&B


A&B + (B - A&B) + (A - A&B)





(A,A&B)      <:  (A+B,B)            all elements must fit within
(A,A)        <:  (A,A) + (B,B)
(A,A)&(B,B)  <:  (A,A)
```




when types are in the residual set we allow them to behave in exlusively one of the following ways:

generic - the default of all types, e.g. matrix (i.e. N**N**num), matrix&left, right, upper, lower, orthogonal,
identity, diagonal, tridiagonal, banddiagonal, positivedefinite, positivesemidefinite etc
all matrix operations are available and some are optimisable. e.g. cov = AT @ A can return identity for the
orthonormal case
i.e. generics do not prevent matching, thus effectively are discarded from the matching decision

implicit - e.g. anon, named, aliased with aliased as the implicit default, defaults do not prevent matching, and
non-defaults can be explicity weakened to the default to provide the right behaviour

familial - e.g. ISIN, CUSIP, inches, cm, all instances in the signature must have the same residual, i.e. like a T1,
thus add(num, num) called with (cm, inches) will not match as cm and inches are both familial, and (cm, num) will
not match as cm is familial to all other types

explicit - e,g, ccy, fx, anything explicit in a residual results in no match

orthogonal - e.g. listOfLists, dtup, ascii, txt (typically classes / structs / values / etc) only one orthogonal type
may exist in the residual and common. Currently classes are the only orthogonal type I can think of - maybe null set,
void, missing, are too. But what about void&num?


generic / vanilla / tags / occasional / unremarkable / exceptional / partial / minor /






