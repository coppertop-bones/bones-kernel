# COMPOSABLE STRING API

## OVERVIEW

We describe TextPad, a composable string api for C that uses the buckets arena.


## WHY

In bones we want good error messages. To make this possible it must therefore be easy to code them but string handling 
in C is rather pedestrian. We make string manipulation easier by providing a composable API.

However this means we need memory management that works separately to the program flow.

We will provide an arena for the lib to allocate with, copy the results out and reset the arena manually.


```c
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  btypeid_t *tl;
    checkpointBuckets((buckets = self->tm->buckets), &cp);
    TP_init(&tp, 0, buckets);
    PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", tm_pp_typelist(self->tm, &tp, tl).cs);
    resetToCheckpoint(buckets, &cp);
    return 0;
```


## WHY COMPOSABILITY REQUIRES MEMORY MANAGEMENT

Consider the following code fragment that returns an error message to Python

```c
PyObject * PyTM_intersection(struct PyTM *self, btypeid_t *tl) {
    BK_TP tp;
    TP_init(&tp);
    
    char *typelistStr = tm_pp_typelist(self->tm, &tp, tl).cs;
    PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", typelistStr);
    free(typelistStr);
    
    return 0;
}
```

We have to keep track of the pointer typelistStr so it's memory can be freed. This boilerplate our ability to just 
focus on creating good error messages. If we compose the function calls:

```c

    PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", tm_pp_typelist(self->tm, &tp, tl).cs);

```

We have one less thing to deal with whilst writing the error message but we need to add some memory management 
functions:

```c
PyObject * PyTM_intersection(struct PyTM *self, btypeid_t *tl) {
    BK_TP tp;  Buckets *buckets;  BucketsCheckpoint cp;  
    TP_init(&tp, 0, buckets);
    checkpointBuckets((buckets = self->tm->buckets), &cp);
    
    PyErr_Format(PyExc_TypeError, "There are exclusion conflicts within (%s)", tm_pp_typelist(self->tm, &tp, tl).cs);
    
    resetToCheckpoint(buckets, &cp);
    return 0;
}
```

## ADVANTAGES OF COMPOSABILITY 

Instead of two lines per string pointer assignment and freeing the memory, we now have two lines per multiple strings. 
The first checkpointBuckets marks a point where we will roll back to; the second resetToCheckpoint frees the memory. 
The programmer focussing on writing the error message doesn't need to think about how the memory allocated to build the 
error message string needs to be freed and can focus instead of writing clearer and fuller error messages.

Imposing a different memory management style here also opens up the possibility for efficient algorithms and 
optimisations.

## ADVANTAGES OF ARENA MEMORY MANAGEMENT

As well as simplifying semantics of tm_pp_typelist - we no longer need to consider who is responsible for freeing the 
memory - and removing the need to manually track pointer, performance is also improved. We:
1) cen bump allocate each string - which is quicker than malloc, 
2) don't need to coalesce string fragments into a new string earlier than necessary to free memory, 
3) have just the one call to free the memory - which also happens to be faster than free,
4) can reduce vulnerability to buffer overflow,
5) can use hot memory more intentionally by coalescing string fragments and resetting the arena to reallocate into hot memory.


## API

TPN - Text Pad Node
16 bytes
first 8 bytes encodes a type and size
second 8 bytes is a union
    TPN* - a sequence of TPNs
    char* - null terminated so we can use standard string functions
    char* - non-null terminated for slicing
in the C-ABI this small two element struct is passed in registers when possible





NOTES



// COMPOSITIONAL STRING RENDERING
// options:
// 1) imperative - onto stream, can't compose non-stack without heap allocation
// 2) composition with malloc and free - hard work and apis become messy and non-obvious about who is responsible for what, also fragments
// 3) composition with arenas - much easier, cleans apis as caller is responsible for clearing stuff, more string copying than necessary, but could add delayed concatenation
// 4) composition with LXR - needs typesystem, since can free in middle of process potentially can use less temporary memory than simple arena
// 5) composition with tree walking onto shared stream - tree needs creating in arena - this may not be too hard, but basic types need wrapping in node constructor

// requirements:
// easy MM and coding
// compositional style
// minimal mem usage
// performance - allocation of tree/control structure vs allocation of temporaries (strings, streams, etc)
// fair resource usage - e.g. to defend against buffer overruns, OOM, etc, e.g. not reasonable for an error message
//   to use GB but maybe a complex file format, e.g. large db of DNA sequence, might (or what ever the upper bound is)
//   if we can GC effectively and return memory to OS we might be ok with temporarily hogging physical memory

// we want to be in control of the amount of memory used but in a nested print we could generate unlimited
// temporaries, before the parent gets called, for example, tp_printf(tp, "problem in this: %s", getDesc())
// and I don't want to code a lazy framework for this either
//
// serialising a tree is the problem - composition involves creating trees
//
// linear programming (aka imperative) is verbose but very clear at small scale

// here's the basic pattern for two arena contexts, outerCtx and innerCtx:
// cp = getCP(innerCtx);
// x = doStuff();
// y = allocate(outerCtx);
// copy(x, y);
// rollbackTo(innerCtx, cp);
// return y

// here's the basic pattern for a single region contexts:
// cp = getCP(ctx);
// x = doStuff();
// ctx.keep(x);  // optional - if scanning we'll see this on the stack, but explicit api may make resource usage better
// rollbackTo(ctx, cp);
// return x

// 4) needs a precise type system
// 5) could be probably cobbled together in C but may impact easy of use


// if we have contiguous address space we could compact a context easily for a single string buffer
// if we have two arenas and a precise type system then we can deep copy a dag (or even a closed network) from one
// arena into the other
// LXR etc aim to compact to free areas

// we can help by moving the last object, whose size we know to the cp, however we can't move objects with refs without
// a walker - can double copy - i.e. copy components into a temp arena, and copy back after rolling back.

// print to locally malloc'd buf is problematic
// ideally we want print to shared buffer which can be early terminated without error if we run out of space

// create a composed tree from the code, walk the tree doing the work in the desired sequence

// here's the problem, in C the arguments get computed before the function gets called to you don't have control of the
// sequence

// if we create the tree we need to store the va_args but... then we need polymorphism to evaluate a deferred object
// consider tp_print(tp, "answer: %i > %s", ++x, tp_print(tp, "%i > %s", ++x, tp_tprint(tp, "%i", x)))

// we express the sequence of results in a composible tree but in this case we want to render to a single buffer AND
// that implies we render "answer: " before calling the second tp_print
// it wouldn't be too hard to make tp_print a functor but then non-tp_prints need wrapping (as we don't have types to
// switch on).

// the range system would work - but...

// returning objects - malloc and free a hassle - arena makes it easier and faster - but multiple copying
// single render requires a change of flow

// without too much effort we could get to
//S8 fred() {
//    return tp_print(tp,
//        ds8("answer: %i > %s"),
//        di32(++x),
//        tp_print(tp,
//            ds8("%i > %s"),
//            di32(++x),
//            tp_tprint(tp,
//                ds8("%i"),
//                di32(x)
//            )
//        )
//    ).cs(mm, buckets)
//}
//
//[tp_print(tp, "answer: %i > %s", x, tp_print(tp, "%i > %s", x+1, tp_tprint(tp, "%i", x+3)))] >> renderTo(_, f)
//
//render(
//    "answer: ")
//tp_print(tp, "answer: ", tpprint(tp, "%i", x++), " > %s", x++, tp_print(tp, "%i > %s", x++, tp_tprint(tp, "%i", x)))
//
//
//function syntax is inner to outer ordered - and for efficiency we want lhs tree evaluation order with shared mach_port_context_t
//the idea is bones [] are a tree not byte codes?
//
//
//fprintf[[_.f], "answer: %i > %s", (x, fprintf[[_.f], "%i > %s", (fprintf[[_.f], "%i", x+3 asArgs])])]
//fprintf[[_.f], "answer: %i > %s", (
//    x,
//    fprintf[[_.f], "%i > %s", (
//        x + 2,
//        fprintf[[_.f], "%i", (
//            x+3 asArgs
//        )]
//    )]
//)]



// is it the DSL?
// printf("answer: ", "%i", x+1, tp_print())
// nope here it's the composition
//
// printf("answer: ")
// printf("%i", x+1)
// r = tp_print()
// printf("answer: ")
// printf("answer: ")
//
// rather the meeting of the tree struct of the types and the compositional form of printf creates results before they can be used


// a tree needs to go on the heap and we have a tree generation pass then a rendering pass
// a tree make takes more effort to malloc and free than the strings we are trying to optimise
// so then we arena alloc the tree - who knows what would be faster

//buffer is a struct with three pointers start, cursor and end or 2 pointers and a size start, cursor, sz




// CONCLUSION
// compositional style with normal, inner to outer, execution creates garbage but is more flexible at the client
// most of the garbage can be handled by a simple arena, with a bit of copying to extract the results before wiping
//
// an alternative execution style involves building the tree then walking it (the tree could be supported by the compiler?)
// the tree can be simple arena allocated and once run wiped without thought - the output process can be very efficient
// but the tree building process may be no faster than inner to outer and adds boilerplate to the client code as nodes
// are needed to wrap everything. tree interpretation may be slow and involved lots of fn calls.
//
// region based arena, e.g. LXR, may reduce temporary memory requirements, without copying and the algo can choose when
// to move - this requires precise typing etc