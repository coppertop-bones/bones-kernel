# NOTES

Generally we should be okay using C-ABI. QBE and MIR both implement C-ABI for several platforms (not sure if there are 
any holes).

It would be nice to have zero-cost exceptions, i.e. itanium, but I think setjmp might be sufficient, or at least to 
start with.


## BLOCKS

TODO
- review and understand the two windows calling conventions, also the macos aarch64 abi
- skim QBE and MIR to assess difficulty in adapting the ABI code


### MOTIVATION AND ESSENCE

A C-style-block, or C-block, is a sequence of code that can be conditionally executed by jmping into it or jmping 
around it, used in conjunction with control flow statements in a language, for example, if-else, while, pattern 
matching etc.

Unlike C-style blocks, Smalltalk blocks (which we just term blocks) are first class and thus can be used to factor 
behaviour in ways that languages that only have C-style blocks cannot. They are usually intimately tied to the 
enclosing function, only existing for its lifetime and able to share its locals.

Smalltalk like blocks we believe are an important enabling feature of Smalltalk and thus implemented in bones. However,
we believe we can drop the long-lived closure part without losing the essential qualities that blocks provide.

Since they can share the locals the compiler must be aware of them. The problem for the compiler is it can't 
necessarily reason about the control flow as this is factored out, usually into a higher-order function we term a
flow-controller function, or even, in the case of Smalltalk booleans / conditionals, into the dispatch mechanism.

Thus blocks need some careful consideration as sometimes they are no longer just simple jmps.


### CANONICAL EXAMPLE

```
caller: {
    b: 1
    3 timesRepeat: [b: b + fn(a)]
}

timesRepeat: {[n, block]
    i: 1
    [i <= n] whileTrue: [
        block(i)
        i: i + 1
    ]
    n
}

caller()
```

`whileTrue:` is stdlib provided in this example \
`fn` is some other


### NOMENCLATURE

- we frame the problem in terms of three functions, the caller function, the flow-controller function and the block 
  function
- "locals", "block-locals" and "flow-control-locals" refers to the local variables defined in the caller, block and
  flow-controller respectively
- "intermediate-layers" means all the code between leaving the caller, inside the flow-controller until calling the 
  block, _together with_ all code the block executes that is outside the caller's scope.


### ESSENCE

A block is effectively a subroutine, i.e. rather than completing to a known location it returns to its caller. 
Additionally, it may take arguments.

The return location, or next jmp label (if we are doing continuation style etc), and potential arguments might mean we 
need to be able to shuffle registers and stack for the block. Thus the compiler needs to be involved.

Since our blocks are not long-lived, i.e. outside the callers stack frame, they can be stack based.


### OPTION 1 - INLINE FLOW-CONTROLLER AND BLOCK

We can inline the flow-controller and block when compiling the caller.

upside
  - we know it works
  - we don't need to spill / unspill
  - potentially a bit faster

downside
  - flow-controller code including locals becomes part of the compilation of the caller
  - block code is not reused which bloats, and the bloat may have other impact such as reducing locality
  - if a block is passed through many functions then inlining could be significant compilation work


### OPTION 2 - INLINE FLOW CONTROLLER - NO ARGS

If a block takes no arguments we can call it like a C function with no arguments.

upside
  - code reuse (reducing bloat)
  - we know it works
  - don't need to spill / unspill
  - potentially a bit faster still due to better code locality

downside
  - may increase branch misprediction as jmp must happen rather than be inlined, e.g. consider [a + b].
  - QBE needs extending to handle this, MIR may do, wouldn't have thought LLVM would
  - flow-controller code including locals becomes part of the compilation of the caller

need to check that jmp/ret style, as opposed to a jmp/jmp style fits within the optimisation model of QBE, MIR, etc

### OPTION 3 - INLINE FLOW CONTROLLER - WITH ARGS

If a block takes arguments we could build in a front-end / RST convention to pass arguments as locals and again call 
it like a C function with no arguments. Same analysis as before but we now have to design the block argument passing
RST convention.


### OPTION 4 - SUBROUTINE CALL ABI

When a block takes arguments, we could theoretically restore all state to look like when we left the caller for the 
flow-controller, but with arguments on the stack or in locals if the compiler could spare them.

upside
  - block code is reused
  - perhaps as efficient as jmps

downside
  - have to create and implement ABI (means thoroughly understanding ABIs we may interact with)
  - flow-controller must always spill as it can make no assumptions about which registers are in use and must restore 
    register state before calling the block - this may be slower that the C-ABI which can use registers for arguments (
    or may not as the compiler doesn't need to switch registers around)
  - issues around segregating the stack of intermediate-layers from caller and block stack

Let's try it.


#### STRAWMAN DESIGN

The caller has certain registers assigned by the ABI so we don't want to force those being used for block arguments. 

The stack pointer must not be changed be used as locals in the caller are relative to it. Also the call to the 
flow-controller uses normal ABI thus SP is changed. 

The stack between the flow-controller being called from the caller, and entering the block must be untouched by all.

This is starting to feel like a setjmp / continuation ABI - making blocks first class is quite an impact. :)

The block itself is struct in the caller's stack-frame that has jmp pointer and an args area.

We thus do not allow blocks to call something that is defined later (thus preventing cycles)

But we don't know where the block is being invoked so we would need to save all registers to allow a block to access any

...

### OPTION 5 - C-FUNCTION CALL WITH SHARED LOCALS BACKED WITH STACK MEMORY

Instead of creating a new ABI / calling convention, lets try saying that locals a block must be implemented like C 
variables, i.e. backed by stack memory, whose address can be taken. Then the block can update the memory, thus it 
looks just like a C-ABI function but one that has been passed a pointer to the local.

upside
  - we know it works

downside
  - we have to ensure that the backend-compiler does no attempt to optimise away the backed-memory
  - it may reduce performance as shared local variables need backing can can't just be register based




### CONCLUSION

Backing the shared locals, OPTION 5, with memory and using the C-ABI is generic. We can inline the flow-controller and 
block, OPTION 1, as long as the flow-control exposes a RST template. An ABI that calls back into the block with all 
registers preserved, OPTION 4, is likely to be fiddly as the intermediate stuff must not damage the state (e.g. stack) 
the block sees and the block must not damage the state intermediate code sees. Jumping back up the stack to the state 
the block could see is like a continuation or co-routine and less friendly with a general stack.

OPTIONS 2 and 3 need some investigating.


### CONCLUSION

We will go with OPTIONS 1 and 5 to start with and look to doing 2 and 3. OPTION 4 is shelved until we have more
experience of implementing the other options.

If we need to extend QBE or MIR we need to consider how we keep close if our extensions are rejected from the main 
project.


