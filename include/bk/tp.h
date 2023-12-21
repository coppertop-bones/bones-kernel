// ---------------------------------------------------------------------------------------------------------------------
// TP - TEXT PAD
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BK_TP_H
#define BK_TP_H "tp.h"
#define BK_TP_H "tp.h"

#include "stdio.h"
#include "mm.h"




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


typedef struct TP_pub BK_TP;       // protect internals of BK_TP

typedef struct {
    size opaque;        // 8
    void *p;            // 8
} TPN;



pub void TP_init(BK_TP *, size, Buckets *);
pub void TP_free(BK_TP *);
tdd FILE *tp_open(BK_TP *, char const *mode);        // macos and linux only I think - not obvious how to catch the flush on windows even it maps to a mm file
pub S8 tp_pp(BK_TP *, char const *format, ...);
pub TPN tp_printftp(BK_TP *, char const *format, ...);
pub void tp_printfb(BK_TP *, char const *format, ...);
pub TPN tp_flush(BK_TP *);
pub S8 tp_render(BK_TP *, TPN);


#endif      // BK_TP_H
