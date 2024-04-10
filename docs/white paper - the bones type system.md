

The bones type system is algebraic in nature - designed for using with plain old data and multidispatch, and, as it 
turns out, is very ameanable to type inference using the algorithms found in Dolan and Parreaux's algebraic subtyping 
material.


## why?
we want ideas that allow us to think about the program and the problem domain
- performance
- correctness
- productivity
- solve bigger problems

a rigourous type system helps meet these goals

soundness - can trust the analysis

the bones type system gives us 
- faster code
- removes certain classes of bugs and identifies mistakes earlier
- clearer code
- less testing code needed (don't need tests for things the type system handles)
- easier to reason about larger code bases
- less type based decision-making code


## preface

describe important points that are required for understanding the rest of the document here. aka terms / axioms.

the type system is a facade on top of a silicon architecture - in reality we have memory and registers with caches and 
pipelines playing an important yet implicit role in execution.

this type system is not functional per se

it works at the semantic analysis stage of the parsing / compiling process

### weaknesses

this has not been tested with agents only values

## main body

### what is a type?

The best definition I've come across is it is a label used in a type system.


### key features

Separation of business types (which depend on whatever particular business you are in) and physical types.\
Polymorphism & isomorphism.\
Multidispatch.\
Subtyping - via intersections and unions.\
Can be used in a REPL.\
Can be used to describe behaviours (no throw, static, const, pure, etc) and structure


### nomenclature
object - a set of bytes in memory, i.e. in the C style meaning rather than the OO style meaning \
value - a thing that cannot be mutated



### the types

Types maybe described or named. A description maybe named. They cannot be defined. A description is in relation to 
other

#### nominal
Our first type is a nominal. It has no inherent meaning and when push comes to shove is ultimated enforced by 
convention. These are our axioms. They may be logical or physical.

#### intersection

#### union
a or b

#### <:
<: is our first algebraic operation

A <: B means A can be used whenever a B is expected, e.g. A&B <: B, A <: A+B

#### orthogonality
union subtyping (by definition / construction) is always true (at least I can't think of an example where it's not)
but certain intersections are empty. For example:
- a 1 byte object is not a 2 byte object. you can't take a pointer to a 1 byte object and use it where ever you  
  need a 2 byte object. Not vice versa.
- an fx rate is not a currency

We make certain intersections illegal by saying a type is othogonal to a group. E.g. m8, m16 form an orthogonal group called memory.
OPEN: work on the terminology here
GBP, USD form an othogonal group ccy
if we make a type explicit then it can't be dispatched implicitly, e.g. fx and ccy are explicit and can't be serviced by
add(f64, f64) but only by add(f64&ccy, f64&ccy) or add(f64&fx, f64&fx)

#### indirection
in C a pointer. in a relationship database it is a foreign key.


#### note on sum types (aka tagged unions)
these fit within this scheme but note we don't need a tag, e.g. we can get the type of the object somewhere else

#### product
an aggregate type of known size where the type of each element is (statically) known

tuple, struct(ordered and named), record (unordered and named)


#### exponential

an aggregate type where the type of every element is the same and the number of element is not known upfront

##### sequence - ordinal -> object

##### map - label -> object

##### function - tuple -> object

##### mixing products and exponentials
e.g. matrix is 2d so part product and part exponential - this is a bigger reflection

##### converting between products and exponentials
product to exponential - we use the union of all elements \
exponential to product - we check / assert the type dynamically

#### recursive

#### schema variable
for templates and generics

T1, T2 etc are locally consistent within a single type lang phrase, TFxDom, TFxFor are globally consistent across 
several phrases

#### no top or bottom
I've not needed them yet and I don't have enough expertise in type theory to know if they are need theoretically or not.

#### no nominal inheritance
It seems to introduce problems so disallowed.

#### examples
aos / soa - along the lines of JAI \
seq & linkedlist, seq & carray \
a person with indirection

#### untested

not tried this with agents

### checking

using Parreaux's algorthim for the simple essence of algebraic subtyping check the program for all A <: B

### simple inference

## example - the bones langauge

value semantics (which affects memory management possibilities) \
weakenings from source code / literal types to argument types \
non-blocking - a program must have a possible happy path and ideally the runtime machinery can gracefully handle exceptions \
multi-dispatch - i.e. overloads\
dynamic-checks / casts - e.g. an exponential that is read from a csv file can be checked / cast to an aos of known type

for type inference with multi-dispatch we restrict functions to have a known number of arguments (i.e. no optional 
args) at the call site and define overloads and families.

for simplicity, we don't have named arguments

### overloads e.g. add_2 is the intersection of all add functions that take 2 arguments

### families
set of overloads

### multidispatch
this is our second algebraic operation

multidispatch means we have to consider unions of return values

distinguish unions and intersections constructed for the program from those created for the data model

### inference with multi-dispatch


# appendix

## dynamic vs static

I think categorising languages as either dynamic or static is less helpful than understanding what can be known upfront 
and what can't and designing in a way that increases the amount known upfront.

You can think of a dynamic language such as Python as a language where all objects are typed as tagged unions and all 
aggregations are exponentials.

