.text
.balign 4
.globl _LF
_LF:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	x0, #32
	sub	sp, sp, x0
	mov	x0, #16
	add	x1, sp, x0
	mov	w0, #10
	str	w0, [x1]
	mov	x0, #8
	add	x1, sp, x0
	mov	w0, #3082
	str	w0, [x1]
	mov	x0, #0
	add	x1, sp, x0
	mov	w0, #3082
	str	w0, [x1]
	adrp	x0, _gLF@page
	add	x0, x0, _gLF@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	adrp	x0, "Lfp0"@page
	add	x0, x0, "Lfp0"@pageoff
	ldr	d0, [x0]
	ldp	x29, x30, [sp], 16
	ret
/* end function LF */

.text
.balign 4
.globl _LF2
_LF2:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	w0, #10
	ldp	x29, x30, [sp], 16
	ret
/* end function LF2 */

.text
.balign 4
.globl _main
_main:
	stp	x29, x30, [sp, -32]!
	mov	x29, sp
	str	x19, [x29, 24]
	bl	_LF
	mov	x19, x0
	mov	x0, #32
	sub	sp, sp, x0
	mov	x0, #16
	add	x1, sp, x0
	mov	w0, #10
	str	w0, [x1]
	mov	x0, #8
	add	x0, sp, x0
	str	x19, [x0]
	mov	x0, #0
	add	x0, sp, x0
	str	x19, [x0]
	adrp	x0, _gpA@page
	add	x0, x0, _gpA@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	ldrh	w19, [x19]
	mov	x0, #32
	sub	sp, sp, x0
	mov	x0, #16
	add	x1, sp, x0
	mov	w0, #10
	str	w0, [x1]
	mov	x0, #8
	add	x0, sp, x0
	str	w19, [x0]
	mov	x0, #0
	add	x0, sp, x0
	str	w19, [x0]
	adrp	x0, _gA@page
	add	x0, x0, _gA@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	mov	x0, #32
	sub	sp, sp, x0
	mov	x0, #16
	add	x1, sp, x0
	mov	w0, #10
	str	w0, [x1]
	mov	x0, #8
	add	x0, sp, x0
	str	w19, [x0]
	mov	x0, #0
	add	x0, sp, x0
	str	w19, [x0]
	adrp	x0, _gA@page
	add	x0, x0, _gA@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	uxtb	w0, w19
	uxtb	w1, w19
	mov	x2, #32
	sub	sp, sp, x2
	mov	x2, #16
	add	x3, sp, x2
	mov	w2, #10
	str	w2, [x3]
	mov	x2, #8
	add	x2, sp, x2
	str	w1, [x2]
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _gA@page
	add	x0, x0, _gA@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	uxtb	w0, w19
	mov	x1, #32
	sub	sp, sp, x1
	mov	x1, #16
	add	x2, sp, x1
	mov	w1, #10
	str	w1, [x2]
	mov	x1, #8
	add	x1, sp, x1
	str	w0, [x1]
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _gB@page
	add	x0, x0, _gB@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	bl	_LF2
	mov	w1, w0
	uxtb	w0, w1
	uxtb	w1, w1
	mov	x2, #32
	sub	sp, sp, x2
	mov	x2, #16
	add	x3, sp, x2
	mov	w2, #10
	str	w2, [x3]
	mov	x2, #8
	add	x2, sp, x2
	str	w1, [x2]
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _gC@page
	add	x0, x0, _gC@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	bl	_LF3
	mov	w1, w0
	uxtb	w0, w1
	uxtb	w1, w1
	mov	x2, #32
	sub	sp, sp, x2
	mov	x2, #16
	add	x3, sp, x2
	mov	w2, #10
	str	w2, [x3]
	mov	x2, #8
	add	x2, sp, x2
	str	w1, [x2]
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _gD@page
	add	x0, x0, _gD@pageoff
	bl	_printf
	mov	x0, #32
	add	sp, sp, x0
	mov	w0, #0
	ldr	x19, [x29, 24]
	ldp	x29, x30, [sp], 32
	ret
/* end function main */

.data
.balign 8
_gLF:
	.ascii "LF: 0x%.8x %d %c"
	.byte 0
/* end data */

.data
.balign 8
_gpA:
	.ascii "pA: 0x%.16x %llu %c"
	.byte 0
/* end data */

.data
.balign 8
_gA:
	.ascii "A:  0x%.8x %d %c"
	.byte 0
/* end data */

.data
.balign 8
_gB:
	.ascii "B:  0x%.8x %d %c"
	.byte 0
/* end data */

.data
.balign 8
_gC:
	.ascii "C:  0x%.8x %d %c"
	.byte 0
/* end data */

.data
.balign 8
_gD:
	.ascii "D:  0x%.8x %d %c"
	.byte 0
/* end data */

/* floating point constants */
.section __TEXT,__literal8,8byte_literals
.p2align 3
Lfp0:
	.int 3082
	.int 0 /* 0.000000 */

