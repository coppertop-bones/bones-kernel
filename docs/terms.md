# TERMS

Every subject has its own language and advanced and fundamental concepts, and here we describe our terms and list 
general references both their "inner qualities" and their "essential relationships"



ADT - algebraic data type

agent - a type, a value in a computer system that has identity, it can be referred to by more than one observer, its
  state may change and all observers will see the change. A simple identity is a memory pointer. Smalltalk objects are 
  agents. The identity property is necessary for an agent to respond to messages - else there can be no guarantee.
  Like in q/kdb variable names in bones are agents but the value they point to is immutable. Similarly, ML has 
  references. The call by value semantics of ML make local variables effectively immutable although they can 
  be locally masked / overridden (aka shaddowing) and as a stack unwinds the old value emerges. Contrast this with the 
  idea of rebinding a name to a value which is the mechanism that make a name in bones an agent. This fine grained 
  definition helps distinguish between functions and messages at the language level. Functions operate purely on value,
  whereas messages operate in the context of agents. Although in reality once values are immutable there is only a 
  subtly difference between them and how the programmer conceptualises a name / variable.

  "In the rare occasions where explicit pointers are needed (the most common case is when translating into OCaml an 
  algorithm described in a classic imperative language), OCaml provides references that are full-fledged pointers"

  Agents (even passive agents without behaviour) are a major cause of complexity in agent based software (e.g. 
  applications) as the program must coordinate their interaction. Multiple threads make coordinate agents even harder
  and GPLs have adapted by providing various langauge features to assist. At the language level bones is single 
  threaded and agent free, i.e. in return for a slightly restricted programming model it manages the agents on behalf 
  of the user-programmer.

Alan Kay - a key influence on bones - "OOP to me means only messaging, local retention and protection and hiding 
  of state-process, and extreme late-binding of all things. It can be done in Smalltalk and in LISP. There are 
  possibly other systems in which this is possible, but I'm not aware of them." - http://userpage.fu-berlin.de/~ram/pub/pub_jf47ht81Ht/doc_kay_oop_en

algebraic-subtyping - "a type system combining subtyping and ML-style parametric polymorphism" first described by
  Stephen Dolan (MLSub) and later expanded by Lionel Parreaux (simple-sub and mlscript). The focus is on 
  algebra first, before semantics and syntax and the result is we can infer good things like principal types etc in the
  presence of subtype relationships (i.e. unions, intersections, and recursions).

APL - A Programming Langauge - see wikipedia - a GPL

application-programming - the style of programming and langauge features that are typically needed when developing 
  applications.

arrow-function

Alexander Stepanov - his ideas on generic programming (https://en.wikipedia.org/wiki/Alexander_Stepanov) have helped 
  form the bones Template style type syntax.

Arthur Whitney - a key influence on bones - https://en.wikipedia.org/wiki/Arthur_Whitney_(computer_scientist)

bag - in bones a nominally indexed product type of known size and element types. We term this a bag to indicate its 
  unordered nature. Some languages term this a record. See also tuple and struct. DEPRECATED.

behaviour

Big-O-notation

binary - in bones, an in-fix function that takes two arguments apart from any provided parentheses, 
  e.g. `a dot(,b,,d,e) c`

boxing - physically storing a value in a box usually together with its type to implement unions and disjoint unions. 
  May have an impact syntactically by requiring boilerplate - which is generally considered undesirable.

branch-mis-predict - can take 10-25 cycles to correct

building - in bones building is the process of parsing and checking source code then integrating the result into a 
  currently running bones-kernel.

business-algorithm-programming

C - a GPL

C++ - a GPL

cache - e.g. memoise an expensive to calculate value

cache-miss - cache line is typically 64 bytes, can take up to 200 cycles to fetch from main memory

cardinal - a number used for counting a quantity. In bones this has the type <:count>.

circular-unification

class - a template of a data-structure that defines allowable memory layout and allowable behaviours

code-generation

collection

combinator - see lambda-calculus in the notations section

composition

context - aka type-table aka environment

contextual-scope - similar to Scala's implicit?

CoW - copy-on-write, an optimisation mechanism that ensures the immutability of values

D - general purpose programming language similar to C / C++.

decidability

destructive-update - altering a value. any destructive updates bones does are done in such a way that they are
  invisible to the program

dictionary-order - "The primary rule in standard dictionary order is that capital letters come before lower case 
  letters. Let's go over some examples to make this clearer. If there are two identical words and one of them
  is capitalized then the capitalized word goes first in the alphabetical order like so: Apple, apple."


discriminated-union-types

DSL - domain-specific-langauge

duck-typing - the use of structural and or behavioural typing in determining the type of something - e.g. "If it 
  walks like a duck and it quacks like a duck, then it must be a duck" - see https://en.wikipedia.org/wiki/Duck_typing

dynamic-dispatch - see also late binding, the dispatch decision (i.e. exactly which function to call) is done at 
  run-time, e.g. by some sort of look-up, e.g. on the instance or it's class in Smalltalk or Python, in a v-table in 
  C++ or Haskell.

dynamic-typing

effect - a permanent change of state of the world. See also side-effect, side-input, side-output, agent (object), value

error-handling - 4 options - 1) ignore it / defer it, 2) return an error code, 3) signal the error, 4) return a 
  non-signalling error I don't know of any others but I think 4) is generally the most productive for dealing with 
  aggregations. 1) looks productive but incurs technical debt. 2) looks rigorous but the price paid is extreme loss 
  of productivity, 3) is the common method in Java, C++ etc. bones allows all four. I don't know how to do 4 without 
  overloads.

evaluate - find a numerical expression or equivalent for (an equation, formula, or function). "substitute numbers in 
  a simple formula and evaluate the answer"
evaluation-strategy - https://en.wikipedia.org/wiki/Evaluation_strategy

exception-handling - we don't use this term as it's usually synonymous with signalling

execute - put (a plan, order, or course of action) into effect

exponential-type - a function including:
  - discrete / finite range sequences (ordinally indexed functions that return a single type - including unions) such as arrays, lists, etc, 
  - discrete / finite range maps (nominally indexed functions that return a single type - including unions) such as dictionarys, hashmaps etc,
  - potentially infinite computation - i.e. what we usually consider a function to be

exponential-time - "are we there yet?"

FFI - foreign-function-interface, e.g. to C, Python, D, Fortran, Rust etc

flow-typing

Fortran - a GPL. I've never read any nor written any but reputedly it's faster than C. It's worth investigating why.

function - including collections and maps

functional-programming - google "what is functional programming" - the take here is that functional programming 
  languages have been formed and informed by the HM inference algorithm. This is at the heart of the FP culture.
  In bones we mean functional in the sense of composing functions together - like Smalltalk and q/kdb we provide
  a pipeline syntax and the ability to pass functions as arguments to other functions, and like Smalltalk and q/kdb
  we believe a mix of imperative and functional style is the most effective for productivity and clarity. Unlike 
  the usual imperative languages, and unlike Smalltalk, and like q/kdb, all our datastructures are values, i.e. 
  immutable. Like kdb and like Smalltalk our names are agents and maybe rebound to different values of the same 
  type. Unlike dynamic languages the type an agent may be bound to is immutable.

fusion - https://wiki.haskell.org/GHC_optimisations#Fusion using the type system to apply optimisations.
  "Crucially, the polymorphic design of Pilatus allows us to use multi-stage programming and rewrite-based 
  optimisation to recover the performance of specialised code, supporting fixed sized matrices, algebraic 
  optimisations, and fusion."

general-type

generics - aka templates

global-type-inference

GPL - general-purpose-langauge

Grady Booch - behaviour, state and identity

Haskell - a GPL - https://qr.ae/pvCmJJ in answer to https://www.quora.com/What-is-Haskell-actually-useful-for. "Wondeful 
  answer! It definitly clarifies the “aura” of mistery and ocultism from other languages. It is a tool for programming 
  language research!". "Haskell came about by committee. A need for a common language between researchers but it’s 
  power made it very useful practically.". Also see [A History of Haskell: Being Lazy With Class](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/07/history.pdf).

happy path - see https://en.wikipedia.org/wiki/Happy_path

HM - Hindley-Milner - a global type inference algorithm (named after J. Roger Hindley and Robbin Milner (of ML fame) 
  not forgetting Luis Damas) and a key influence on bones. Variants, include algorithm W (bottom up - the original) 
  and algorithm M (top down - the folk lore algo). The way values are accessed in bones has been designed to allow 
  global type inference using HM style inference algorithms yielding principal types. HM is possibly a fundamental of 
  functional programming.

identity - the understanding that a value is more than a value, that it is just one instance of many similar values. 
  For example 42 may be just the number 42 or it may be specific copies of 42, e.g. the 42 over here has a distinct
  identity to the 42 over there. Having an identity means it can be aliased and allows mutation into the system. 
  E.g. the thing over here that was previously known as having 42 now has 43. Adding an identity property to a value 
  changes it into an agent. Names are agents in bones.

immutable - the eternal unchangeable essence of a value. In bones values are immutable, i.e. values.

imperative-programming - see also agent. A style of programming where the resulting value of an agent is dependant on 
  the sequence of operations. E.g. integer multiplication could be implemented as doing as addition a certain number
  of times. Sometimes algorthms are easier to understand and implement in an imperative style, however if this is the 
  only style available then things can get messy. See also functional-programming.

intersection-subtyping - e.g. string & propperName is a sub-type of string. bones considers the properties of the 
  types in the intersection as part of its overload resolution so has behavioural ducktyping too.

Java - a GPL

kernel - a bones agent - it can compile and run code and store state. It can be in-process or out-of-process. 
  Communication is either std style - e.g. a REPL with stdin, stdout and stderr - or messaging style (to be 
  implemented) with richer protocols and formats - e.g. peer-to-peer RPC, Jupyter, FFI, etc.

late-binding - delaying the dispatch decision (i.e. exactly which function to call) to the last possible moment. bones 
  does as much of this as statically as possible

lattice

let-polymorphism - 

linear-time - O(n)

map

message

missing-null-na - the typically types that a panel of data needs inorder to usefully model (and manipulate those 
  models of) the realworld. These labels are often used in databases. For example q/kdb builds in the ideal of 
  null values into it's basic datatypes and propagates non-signalling nan automatically and pervasively.

ML-family - Meta Language, OCaml, F#. "Haskell and the ML family are like cars. They all have doors and seats, wheels, 
  an engine, speed and break control and a steering wheel. Aside from the things that make them a car they are much 
  different by model and manufacturer. 

  Haskell and the ML family are not even distant cousins."

ML-gang - Robin Milner

mlscript - a javascript like language that uses algebraic-subtyping for typing

MLSub - a typing methodology "introduced by Dolan and Mycroft \[2017] as an ML-style type system supporting 
  subtyping, polymorphism, and global type inference, while still producing compact principal types."

model - a representation of an observable phenomena used for understanding or prediction of that phenomena

monad

multi-dispatch - the dispatch decision (i.e. exactly which function to call) is made by considering the types of all
  the arguments. In a system that allows int to be passed into a function that takes int + float this can be resolved
  to an implementation that just takes int. However, if an input type is being inferred from something downstream 
  then a combinatorial explosion of possibilities can occur. For example what is the type of the quadratic equation 
  y = ax^2 + bx + c if we allow the variables to be int or float? We can write it as int->int->int->int->int + 
  float->int->int->int->int + ... or we can write it as (int+float)->(int+float)->(int+float)->(int+float)->(int+float)
  The latter feels inefficient as we know that we can use specific machine instructions if we just know the type, but 
  we can intuit that we will run out of memory and time in the inference process if we use the former. Nonetheless
  we can restrict the possible sets statically, optimise after the global-inference step and deal with the remainders 
  either dynamically or by helping the programmer annotate for performance.

  Multi-dispatch gives use two things - 1) extreme late-binding of all things - the word join can be contextual so 
  joining two strings, two panels, two lists, two column vectors can all share the concept join thus reducing 
  cognitive load, and 2) libraries can be made extensible because the addition of new types and new implementations 
  can be handled by the dispatch machinery. The Julia language documentation goes into this at some length. E.g.
  https://docs.julialang.org/en/v1/manual/methods/

  NB doing mulitple receiver-dispatch doesn't scale do to the number of boilerplate methods that need to be created 
  even before the call is done. Additionally fallbacks and metrics are harder (impossible?) to do in multiple 
  receiver-dispatch.

mutation - 

name - in general a label corresponding to an agent, value, function, class, type, etc. In bones names are agents that
  can hold values, types, or functions. Names know the type of the thing they hold. We say a name is bound to a thing
  which commonly might be termed assignment.

name-binding

nominal - a number (or string etc) that is used as a name or an identifier for something. E.g. the number "8" on the 
  back of a player, an american zip code, a telephone area code, a car's model number, e.g. peugeot 106. In bones the 
  usual type for integer nominals is <:nominal>.

nominal-typing - see also intersection subtyping. The process of reasoning about types by using names and explicitly
  expressed relationships between them, e.g. the usual OO class hierarchies use nominal-typing. see https://en.wikipedia.org/wiki/Nominal_type_system
  bones has nominal types but does not support nominal subtyping. Instead new relations are created e.g. type C is the 
  intersection of type A and type B. Type D is the union of type A and type B. Thus C is a subtype of A, a subtype of B, 
  and a subtype of D.

non-signalling - an error that may affect program behaviour but that doesn't signal the interruption or termination
  of program execution.

nullary - in bones a function style that consumes zero arguments from the pipe and can only be invoked with 
  parenthese, i.e. verb(...)

object-oriented-programming - OOP

object - a piece of memory that is not machine code. A unit of memory allocation. We are using this in the C sense of 
  the word rather than the OOP sense which includes behaviour and usually class hierarchy.

Ocaml - a GPL

occurance-typing

ordinal - a description that indicates ordering, e.g. letters of the alphabet when thought of in alphabetical order, 
  e.g. dictionary ordering where "A" comes before "a", e.g. roman numerals, e.g. a calendar system (e.g. the 
  Gregorian calendar, the Julian calendar), e.g. the words first, second, third, ..., e.g. the symbols 1st, 2nd, 3rd, 
  ..., e.g. the numerical index that intuitively is used for page numbers, i.e. starting at 1, then 2, ..., e.g. and 
  finally the ordinal beloved by computer nerds the offset which starts at 0, then 1, etc. In bones this last one is 
  considered just as equal as the others and has the type <:dijkstra> - see https://www.cs.utexas.edu/users/EWD/transcriptions/EWD08xx/EWD831.html
  However, page-numbering-style is chosen as the default integer value for ordinals and given the type <:index>. See 
  also cardinal.

overload

partial-application

parametric-polymorphism

persistent-data-structures

pipeline-style

polyvariance (the flow-based analog of parametric polymorphism in type inference)

principal-type

product-type - including structs and tuples, a collection of distinct values

programming

puzzle-langauges

Python - a GPL

q/kdb - functional in style and feel though dynamically typed - 'type anyone?

rau - right-associative-unary - in bones a function style that consumes one argument from the pipe which is to
  the immediate right (not including parentheses) of the verb, e.g. verb arg and verb(,...) arg

Rebecca Wirfs-Brock - a key influence on bones - see https://en.wikipedia.org/wiki/Rebecca_Wirfs-Brock and her books
  on responsibility-driven-design https://www.goodreads.com/en/book/show/1887814.Designing_Object_Oriented_Software and 
  https://www.goodreads.com/book/show/179204.Object_Design

rebinding - causing a name to have a different value of a compatible type

receiver-dispatch - (aka single-dispatch) where the dispatch decision (i.e. exactly which function to call) 
  is resolved based on the first argument - the receiver - and where its class is located with regard to a 
  graph (almost always a tree or dag). The graph is usual also known as the inheritance hierarchy. see also 
  multi-dispatch.

recursive-type

reference
  in ML:
    - An ML reference is a value which is the address of a location in the memory. 
    - The value in the location is the content. 
    - The content can be read through the dereferenceing operation and changed through an assignment operation.
  bones doesn't have references, memory location cannot be aliased (except when being passed via an FFI), but
  names are agents that may be rebound.

Scala

semanticist - looking from the view point of a language's semantics - "For a semanticist, ML might stand for a 
  programming language featuring first-class functions, data structures built out of products and sums, mutable 
  memory cells called references, exception handling, automatic memory management, and a call-by-value semantics. 
  This view encompasses the Standard ML"

Scheme - a GPL, lisp based

scope

script

semantic-subtyping - sub-typing based on a set theory - see https://wiki.c2.com/?SemanticSubtyping, 
  http://www.cduce.org/papers.html, https://www.irif.fr/~gc/papers/icalp-ppdp05.pdf and
  https://blog.sigplan.org/2019/10/17/what-type-soundness-theorem-do-you-really-want-to-prove/

sentence - "1. A grammatical unit that is syntactically independent and has a subject that is expressed or, as 
  in imperative sentences, understood and a predicate that contains at least one finite verb." - AHD

set-theoretic

signalling - aka interrupting program execution when an exception / error is encountered

simpleSub

slack-variables

Slang - the Goldman Sach's language - a GPL

Smalltalk - a GPL

soft-typing

state

static-dispatch

static-typing - advantages correctness, performance, comprehension. The type system is applied to the code. Compare this
  to dynamic typing where the type system is applied to run-time in memory objects.

strong-type-system

struct - in bones an ordinally and nominally indexed product type of known size and element types. In some 
  languages (e.g. mlscript) the ordinal property is dropped. In bones we term that a bag (to indicate its 
  unordered nature). In other languages the term record may be used for nominally index products with or without 
  the ordinal property.

structural-subtyping - where the type system can follow the guts of structures - e.g. {height:num, age:int} is a 
  sub-type of {height:num}, i.e. the width rule is satisfied. See https://en.wikipedia.org/wiki/Structural_type_system

subsumption - https://en.wikipedia.org/wiki/Subtyping#Subsumption

sub-type

subtyping-lattice

sum-type

syntactic-subtyping - see Dolan or Parreaux, see also algebraic-subtyping

systems-programming

ternary - in bones a function style that consumes three arguments from the pipe, i.e. arg1 verb arg2 arg3

traditional-unification

tuple - ordinally indexed product type of known size and element types

type - a label for the category of things that a value can be considered compatible with. For example 1, 2, 3, ... 
  can be considered as being compatible with the type "natural number" (aka here "nat"), and 0, 1, 2, 3... can 
  be considered as being compatible with the type "whole number" (aka "whole). And so. The may be relationships 
  between types, two important ones being union and intersection. We can see that the type "nat" is a sub-type 
  of "whole". In some situations when we use intersections we focus on the set of possibilities (in a semantic 
  subtyping style) and in others we may be more abstract, for example we may define the type age as the 
  intersection of nat & _age where _age is a convention / label that indicates that a value is agey. More
  concretely consider the idea of currency which disallows addition of different currencies with involving an
  exchange rate. Nowadays we might use a IEEE double intersected with EUR, CHF, JPY, GBP, USD etc. But what about
  when GBP was record in pounds, shillings and pence? So we want the non-addition rules to be reusable over double 
  and a bespoke pounds-shillings-pence data structure. bones types are designed to provide that sort of modelling.
  Finally, for some fun, consider nummus aureus, silver denarii and bronze asses - I did my research on the internet
  so it must be true.

type-annotation - easier type inference, tighter dispatch, improved readability, improved correctness. 

type-checking

type-class

type-inference

type-rank

type-scheme - see https://smallcultfollowing.com/babysteps/blog/2012/04/23/on-types-and-type-schemes/#:~:text=A%20type%20scheme%20is%20basically,be%20bound%20in%20this%20scheme.
  and https://en.wikipedia.org/wiki/Hindley%E2%80%93Milner_type_system. In bones the types T, T1, T2 etc are 
  the equivalent of bound type-scheme variables

type-system - "type systems are designed to prevent the improper use of program operations. They can be classified as 
  either static or dynamic depending on when the detect type errors. Static type systems detect potential type errors 
  at compile-time and prevent program execution. Dynamic type systems detect type errors at run-time and abort program 
  execution"

typing

typing-rules

user-programmer - a person who is simultaneously a user and a programmer, e.g. this often happens in Excel, counter 
  examples are a person just entering data and into an XL spreadsheet given to them by a colleague - they are a user in 
  this instance - and in similar situation where a desk-developer create an XL spreadsheet for a trader to use - they 
  are a programmer. bones is targets the community of people (who are not programmers or whose programming is not 
  relevant to the problem - e.g. a C++ programmer who's been tasked with writing a risk report) who find themselves 
  wearing the hat of user-programmer. bones doesn't target programmer-users who probably don't need help as their
  tooling is working.

unary - in bones a function style that consumes one argument from the pipe, i.e. arg1 verb

union-type

value - a type, that in bones is an immutable binary encoding of information. Typewise values cannot form a cycle 
  as that would require identity, i.e. agency. Semantically it is impossible to alias a value in the sense of two 
  names sharing a value that can change such that one name can see a change because the other name has mutated.

vectorise

visitation

vtable

weakening

weak-type-system



<br>



### RELATIONS


#### HM <=> languages based on lambda calculus (ML & Haskell?)

When the simplesub algorithm encounters a new name, e.g. a let or a parameter, it pushes a fresh variable 
onto the ctx and pops it after figuring the type of the body. The paper mentions that variables are mutated. 
Child nodes in the AST can see all the variables above them - can parent variables get mutated? This style of 
scoping probably isn't essential to HM though seems to be common - e.g. the idea of variable shadowing comes as a 
consequence. Seeing all parents implicitly (especially module level variables), i.e. as free-variables, facilitates, 
and possibly causes, input spaghetti. We prefer combinators, i.e. no free variables.



#### Deep copy, NEDT, closures, value-orientation, alias errors, identity, Alan Kay and a performance lesson from Fortran

I don't know of any good value oriented languages. Agent bugs are a whole class of errors I'd like the language to 
handle for me just like many languages handle memory management for me. When I'm coding at scale I don't have the 
brain capacity to think about agents. Please, just values only. As it happens almost every data wrangling and algo 
problem I can think of (with the possible exception of producing risk in a curve library) that I could possibly want 
to tackle just doesn't need agents just values. Classes are not the thing that makes OO OO (as Alan Kay points out) 
but identity. Bones isn't OO in that sense, but then neither was Smalltalk in many usage styles. Thank you Arthur 
Whitney.



#### Overloading and options in annotation

"Naming is hard". Apparently. It explains why natural langauge is overloaded and occasionally why misunderstandings 
occur. We want our programs to be correct so we have to deal more precisely in a program than in the real world.

There are several mechanism available to distinguish a function set. Namespace, e.g. num.mul and matrix.mul. 
Function name, numMul:{...} and matrixMul: {...}. Literal function {a * b}. Number of parameters, e.g. 
fred: {[a]...}, fred: {[a,b]...}. Argument annotation, e.g. thing1 <:num> mul (thing2 <:num>). And within a set of 
similar looking functions we can also distinguish using parameter annotation, e.g. mul: {[a:num, b:num]...}, 
mul: {[a:matrix, b:matrix]...}.

All these are used by the inference process to distinguish which specific function is meant and if the program is 
valid or not. The inference process cannot easily distinguish between function sets and if there is ambiguity then
a dynamic error may occur. Statically we know where these errors can occur.


<br>



### NOTATIONS

#### Lambda calculus

See http://www.cs.cornell.edu/courses/cs3110/2008fa/recitations/rec26.html
See https://plato.stanford.edu/entries/lambda-calculus/
Possibly https://personal.utdallas.edu/~gupta/courses/apl/lambda.pdf

A λ-calculus term is:
- a variable x∈Var, where Var is a countably infinite set of variables
- an application, a function f applied to an argument x, usually written f x or f(x)
- a lambda abstraction, an expression λx.b representing a function with input parameter x and body b. Where a 
  mathematician would write x ↦ x*x, or an SML programmer would write fn x => x*x, in the λ-calculus we write λx.x*x.

λfg means a function with input parameters f and g

COMPOSE = λfg. λx.f (g x)
TWICE = λf. COMPOSE f f
= λf. (λfg. λx.f (g x)) f f
= λf. λx.f (f x)      (by β reduction inside the lambda abstraction)

In the lambda term λx. x * y, y is a _free variable_ and x is a _bound variable_. If a lamba term has no free variables
it is called a combinator.



#### Logic / Type System Notation

https://en.wikipedia.org/wiki/Type_rule

https://en.wikipedia.org/wiki/List_of_logic_symbols

https://en.wikipedia.org/wiki/Glossary_of_mathematical_symbols




 expression {\displaystyle e}e of type {\displaystyle \tau }\tau  is written as {\displaystyle e\!:\!\tau }e\!:\!\tau. The typing environment is written as {\displaystyle \Gamma }\Gamma 

Γ - capital Gamma, a typing context (aka typing environment)

for a function domain -> range

e : τ - a typing judement, an assertion that when expression e is evaluated result will be a τ



T1 <: T2   T1 is a sub-type of T2

S ⊆ T - S is a superset of or equivalent to T

S ⊃ T - S is a subset of T

A ∧ B - A and B, A intersection B - a subtype is likely to be "bigger/wider/deeper" (its values hold more information) 
    and "fewer/smaller" (the set of values is smaller) than one of its supertypes (which has more restricted 
    information, and whose set of values are a superset of those of the subtype).

A ∨ B - A or B, A union B

→ implies

⇒ implies

⊃ sometimes means implies

≡ material equivalence

¬ negation
! negation
~ negation

\mathbb {D} - domain of discourse - double D

∧ and
. and
& and

∨ or
+ or
| or

exclusive disjunction - ⊕, ⊻, ≢, ↮

top, tautology, true - ⊤, ■, capital T, 1

bottom, contradiction, false - ⊥, □, capital F, 0

