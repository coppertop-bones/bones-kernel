## kernel

syms \
enums \
mm \
tm

VM buffer - over allocate buffer in VM and allow OS to allocate physical memory as and when required \
ID is an index into an array (fixed size elements) \
RP is a u32 offset into a buffer (variable size elements) - Relative Pointer


### syms
istrings - VM buffer of u16 length prefixed, null terminated utf8 strings for type name and sym interning - RP indexed \
istring_rp_hash - hash to get the rp for a string \
OPEN: in the future we should split type names from other syms? \
OPEN: read up on how other systems do interning


### enums
map syms to enums (linear probe to start with, maybe hash later) \
maintain local id and sort order


### bk objects
object slots are 16 byte aligned and bkheader (4 byte) prefixed, thus 12 bytes is largest 1 slot object, 28 bytes is
largest 2 slot object, etc. \
prefixing is chosen for better c-abi compatability and to help keep bkheader private (to minimise breaking changes)
16 byte alignment leaves 20 bits free, i.e. \
`| XXXX XXXX XXXX XXXX | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP XXXX |`

