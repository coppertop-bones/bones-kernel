# BONES


## PURPOSE

Bones is a high level, high performance, decision support language intended for non-career programmers aged 8 to 88. 



## SYNTAX

### RESERVED PATTERNS
```
[...]               - blocks and exponential accessing
[[...]...]          - blocks with parameters
{...}               - function (unary) and literal structs
{[...]...}          - function with arguments
{{...}}             - function (binary)
{{[...]...}}        - function (binary) with arguments
(...)               - parenthesis, literal tuples, function application 
<:...>              - type-lang
.                   - tuple / struct accessing (including namespaces), phrase separation, 
                      accessing parent scope (as an implicit parameter)
..                  - accessing module scope
_.                  - accessing contextual scope
_..                 - accessing global scope
:                   - binding, separation of parameter names and types
->                  - return type (of a function or block)
,                   - phrase separation
;                   - phrase separation
\                   - line continuation??
^                   - scope exit
^^                  - scope exit
^^^                 - scope exit
!!                  - signalling
load ...            - load a module
from ... import ... - import names into a module namespace
...                 - elipsis
```


### NAMES
variable names - _, A-Z, a-z and 0-9 - must start with either a letter or _. \
function names - _, A-Z, a-z, 0-9, <, >, !, =, #, @, $, %, ^, &, *, +, /, -, |, ~, ' and ? - may not start with a number 

function names can be interspersed with :, for example ifTrue:ifFalse:, which can be used with a Smalltalk style 
pipeline. \
we will probably allow variable names to have the non-alphanumeric characters but not certain yet.


### PHRASES

A phrase is sequence of literals, names and function calls that returns a value. Given:
```
n0, n1, n2, n2      - four nullary functions with 0, 1, 2 and 3 arguments
u1, u2, u3          - three unary functions
b2, b3              - two binary functions
t3, t4              - two ternary functions
```

fortran style function invocation:
```
n0()
n1(n2(1,2))
```
and similarly for the unary, binary and ternary functions.

partial invocation:
```
u3(,2,)(1,3)        - in total the same as u3(1,2,3)
```

unary pipeline:
```
1 u1                - u1(1)
1 u2(,2)            - u2(1,2)
2 u2(1,)            - u2(1,2)
3 u3(1,2,)          - u3(1,2,3)
1 u1 u2(,2)         - u2(u1(1),2)
```

binary pipeline:
```
1 b2 2              - b2(1,2)
1 b3(,2,) 3         - b3(1,2,3)
2 b3(1,,) 3         - b3(1,2,3)
1 b2 2 b3(,3,4)     - b3(b2(1,2),3,4)
```

ternary pipeline:
```
1 t3 2 3            - t3(1,2,3)
1 t4(,2,,) 3 4      - t4(1,2,3,4)
```

keyword style: \
given a 3 argument function ifTrue:ifFalse:
```
condition ifTrue: [block1] ifFalse: [block2]  - ifTrue:ifFalse:(condition, [block1], [block2])
```

so putting it all together 
```
a + b addOne
```


### BINDING
We say that we bind a name to a value, i.e. we give a value a name. We may rebind the name to a new value.

Bind the name(s) on the left to the thing(s) on the right. There must be no space between the name(s) on the left and 
the :, and there must be a space between the : and the phrase on the right.
```
a: 1                    // binds the literal number 1 to a
(a, b): (1, {{x + y}})  // binds the literal number 1 to a and the binary function to b
atup: (1, {{x + y}})    // binds the tuple (1, {{x + y}}) to atup
(a,b): atup             // unpacks the tuple atup and binds the literal number 1 to a and the binary function to b
```

Bind the name(s) on the right to the thing(s) on the left. There must be a space between the phrase on the left and 
the :, and there must be no space between the : and the name(s) on the right.
```
"hello", `there :(a,b)  // binds the text "hello" to a and the symbol `there to b
```


### SCOPES
In bones we have global scope (which could be backed by disk similarly to q/kdb), module scope, contextual scope and
local scope. Functions create their own local scope. Blocks share the scope of their parents and can rebind parents'
variable names, but new variable names are only visible in the block itself and not to parents.

Children may see but not rebind a parents function names though can extend them by overloading. Children may not see
parents' variable names though it is easy to implicitly add a direct parent's variable name as a parameter.

This scoping style means we don't have closures and don't seem to need monads, thus simplifying the language for the
intended target audience.

Contextual scope is like a contextually defined / changable global scope. To be discussed later.


### FUNCTIONS
A function defines some code that we may run explicitly at a later point. It can have inputs and outputs. A function 
creates a new scope that cannot see variable names in parent scopes, but can see function names.

Examples of binary functions to add to two 64-bit floating point numbers:
```
1.0 {[x:f64, y:f64] -> f64. ^x + y} <:binary> 2.0
1.0 {[x:f64, y:f64] ^ x + y} <:binary> 2.0          // rely on type inference
1.0 {[x:f64, y:f64] x + y} <:binary> 2.0            // last phrase is return value
1.0 {[x, y] x + y} <:binary> 2.0                    // reply even more on inference
1.0 {{[x, y] x + y}} 2.0                            // use {{...}} form for binaries
1.0 {{x + y}} 2.0                                   // use implicit arguments
```


### BLOCKS
Similarly, a block also defines some code that we may run explicitly at a later point. They create a new scope, but 
since the can see the parents' scopes it only needs to contain locally defined variables that do not appear in the 
parents' scopes.

Examples of blocks to add to two 64-bit floating point numbers:
```
[[x:f64, y:f64] -> f64. x + y](1.0, 2.0)    // blocks can only be called fortran style and ^ exits the direct parent
[[x:f64, y:f64] x + y](1.0, 2.0)            // rely on type inference
[[x, y] x + y](1.0, 2.0)                    // reply even more on inference
[[x] x + y](1.0)                            // refer to y defined in the parent scope
```


### TYPE-LANG
TBC


### COMMENTS
```
// rest of line is a comment
/- block comment -/
/! conditional comment, e.g. for optional inclusion of code !/ 
'{[...] ...}' - breakouts for embedding other languages
```


### LITERALS - strings, symbols, numbers, datetimes, tuples, structs, tables
"fred"                  - string
`fred and `fred`joe     - symbol and symbol list
([a:u32, b:txt] a: (1,2), b: ("one","two"))



## SEMANTICS

### INDEXS AND OFFSETS
Indexes, such as page numbers, final postions on the starting grid, etc, start at one. Offsets start at zero.


### IMMUTABLE VALUES
All objects are values in bones and as such may not be changed. Conceptually a change to an object is the creation of 
new object incorporating the desired changes. Thus it is impossible to make a change to a structure via one name that 
can be seen in another name. This allows us to provide nice syntax for deep change, for example:

```
a: {tup: (1,2,3)}
b: a
b.tup.1: 3.  b.tup.3: 1
a == {tup: (1,2,3)}
b == {tup: (3,2,1)}
```

Thus it is not possible in the bones language to create recursive structures.


### AUTOMATIC MEMORY MANAGEMENT
Memory management is handled on behalf of the user using a combination of stack, arena and region style - conservative
on stack and precise on heap. By default objects may be moved in memory but can be pinned where necessary, for 
example when needing to interface with external code.


### SIGNALS
The kernal may be signalled, and will search up the call stack until a signal handler is found which is then invoked to
determine what next to do - close resources, resume, panic, ask the user (e.g. "this is taking a long time, abort?"), 
and so on. Under the hood we will likely use the itanium exception mechanism.



## METATYPES

TBC


## OTHER FEATURES

The bones language does not define any control structures other than return and signal. So the usual assortment of 
conditionals, loops, etc, is left to libraries (including the default library) to provide. 

The accessing functions, '.' for product types, and '[...]' for non-function exponential types may be overloaded.

Bones can easily be called from and call to Python.

It can be interpreted or compiled by an optimising backend compiler such as LLVM, MIR, QBE, etc.

Functions use the platform's C ABI and the itanium exception ABI.

Structs are laid out C style.

Values are strongly typed.
