Bones Kernel provides
- symbols and enums, i.e. interned strings and groups of them
- type manager - provides api to create and inspect types
- function selection - api to register and query overloads from function argument signatures
- fast memory management with GC, compaction and CoW support - IMMIX inspired
- reduced syntax tree - suitable for both C and bones


walker - walks objects to determine live object closure, needs to know if a type contains a pointer


Bones Types are represented as a u16 - since we'd like the type system to be cache hot




assumptions
the type system will be used frequently and thus will be cache hot
it should be as small as possible
cache misses on frequently used function selection tables will negatively impact performance

none of this may be true and a u32 may be fine but we've designed for this contingency


types are stored in SOA form




memory

each object is aligned to 16 bytes and prefixes with the type



