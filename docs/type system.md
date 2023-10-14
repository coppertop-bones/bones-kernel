# Bones type system

In bones, a type is considered a label that is used by the type system to check some aspects of the validity of a 
program, to generate code from a template and to manage memory. This label or type is attached to name, i.e. a 
function or a value, always in the building process, sometimes at runtime to track a contained object in a union, and
to tag objects in kernel managed memory. The tag is needed for a garbage collection style termed conservative on 
stack, precise on heap. Side note: we allow conservative escape hatches on the heap to handle things like C unions, 
and pin both conservatively reference objects and objects shared outside of the kernel.

Every type has a unique identity, used at build time for inference, checking, code generation and compilation, etc,
at runtime to distinguish types in a union which cannot be statically determined, and by the kernel for precise memory
management.


## metatypes

We have the following sorts of types: 
nominal \
intersection, union - set relation types \
tuple, struct - aggregate (aka product) types \
sequence, map, function - arrow (aka exponential) types 

Any type that can recursively reach its own definition is flagged as such.

Types can be grouped into exclusive sets where types of a particular exclusion may only appear once in a definition.
For example, the exclusive set "memory" (aka "concrete type" by Wikipedia) is used to prevent the creation of 
intersections like Int & Double. Other examples include, finance / business types such as Currency (possibly with 
subtypes GBP, USD, etc) and FX (GBPUSD, USDEUR, USDJPY, etc) and physical types such as feet, inches, cm, hour, 
minute, second, weekday, etc.


## The subtype relation

Intuitive subtyping rules are defined in the behaviour of the <: operation - which as well as answering if type A fits 
within type B can also measure distance between types. 

The following uses & for intersection, + for union, (A, B) for tuple, {fred:A, joe:B} for a struct with fields fred and joe, A->B 
for a function from A to B, and {fn1, fn2} as a set of functions, aka overlaods. Memory based functions are so 
common that we use N**A for a sequence of A, i.e. a map from ordinal to A, and A**B for a map from A to B.


### nominal, set and aggregate subtyping
```
         A   <:  A      - true
         A   <:  B      - false

        A&B  <:  A      - true
        A    <:  A&B    - false

        A    <:  A+B    - true
        A+B  <:  A      - false


(A,A&B)      <:  (A+B,B)            all elements must fit within
(A,A)        <:  (A,A) + (B,B)
(A,A)&(B,B)  <:  (A,A)


(A&B) & (C&D) == A&B&C&D
(A&B) + (C&D) == (A&B) + (C&D) 
(A&B) + (A&D) == A & (B + D)
(A+B) & (C+D) = A+C & A+D & B+C & B+D
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

fred: fn(A)    - okay
fred: fn(B)    - not okay
joe: fn(A)     - okay
sally: fn(A)   - not okay
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



