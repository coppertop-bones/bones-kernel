pointers - 8 byte aligned
| ---- ---- PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP P--- |
11 spare bits

pointers to bk objects which are 16 byte aligned
| ---- ---- PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP PPPP | PPPP PPPP PPPP ---- |
12 spare bits


IEEE Doubles - NaN boxing - 52 spare bits

| S\[--Exponent-][-------------------------------Mantissa-------------------------------]
| SEEE EEEE EEEE MMMM | MMMM MMMM MMMM MMMM | MMMM MMMM MMMM MMMM | MMMM MMMM MMMM MMMM |
| S111 1111 1111 0000 | 0000 0000 0000 0000 | 0000 0000 0000 0000 | 0000 0000 0000 0000 |   +- infinity
| -111 1111 1111 1--- | ---- ---- ---- ---- | ---- ---- ---- ---- | ---- ---- ---- ---- |   non-signalling NaN
| -111 1111 1111 0--- | ---- ---- ---- ---- | ---- ---- ---- ---- | ---- ---- ---- ---- |   signalling NaN but mantissa > 0

if double + pStr - 7 spare bits

any struct > 2 things is going to be a pointer - so no real overhead in a 64bit box?
symbols
unboxed types
symbols (length prefixed utf8), strings (utf8), numbers u8, i8, u64, i64, f64, bool, small structs

I don't think this can help us
