Bones Kernel provides
- symbol manager - i.e. interned length prefixed (max 64k) null terminated utf8 strings
- enum manager - groups of symbols
- type manager - create, inspect and compare types
- object manager - supports GC, compaction and CoW - IMMIX inspired, intended to be fast
- function selection - api to register and select functions from argument signatures
- reduced syntax tree - suitable for both C and bones
- RST interpreter with stepping


lifecycle nomenclature
create, build, new
trash, drop, discard, teardown, dismantle,



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




case styles for variable names within jones:
- t_integral_types
- equationvaluesarehandcasetokeepthemshort
- objects_are_snakecase
- *pointers_to_obejcts_are_not_typedeffed
- PyTypesArePascal
- pythonObjectsAreCamel
- CONSTANT_ARE_BLOCK_CAPITALS
- JonesTypesArePascalWRT

null terminated strings are unsigned char
t_utf8 indicates it maybe uft8


create
trash
init
break



