.text
.balign 4
.globl _lf
_lf:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	w0, #10
	ldp	x29, x30, [sp], 16
	ret
/* end function lf */

.text
.balign 4
.globl _main
_main:
	stp	x29, x30, [sp, -32]!
	mov	x29, sp
	str	x19, [x29, 24]
	mov	x0, #16
	sub	sp, sp, x0
	mov	x1, sp
	mov	w0, #1
	str	w0, [x1]
	adrp	x1, _p@page
	add	x1, x1, _p@pageoff
	adrp	x0, _fprintf@gotpage
	ldr	x0, [x0, _fprintf@gotpageoff]
	str	x0, [x1]
	adrp	x0, ___stdoutp@gotpage
	ldr	x0, [x0, ___stdoutp@gotpageoff]
	ldr	x0, [x0]
	adrp	x1, _f@page
	add	x1, x1, _f@pageoff
	str	x0, [x1]
	adrp	x0, _p@page
	add	x0, x0, _p@pageoff
	ldr	x0, [x0]
	adrp	x1, _p2@page
	add	x1, x1, _p2@pageoff
	str	x0, [x1]
	adrp	x0, _f@page
	add	x0, x0, _f@pageoff
	ldr	x19, [x0]
	bl	_lf
	mov	w1, w0
	mov	x0, x19
	adrp	x2, _p2@page
	add	x2, x2, _p2@pageoff
	ldr	x2, [x2]
	sxtb	w1, w1
	mov	x3, #16
	sub	sp, sp, x3
	mov	x3, #8
	add	x3, sp, x3
	str	w1, [x3]
	mov	x1, #0
	add	x3, sp, x1
	mov	w1, #1
	str	w1, [x3]
	adrp	x1, _.s1@page
	add	x1, x1, _.s1@pageoff
	blr	x2
	mov	x0, #16
	add	sp, sp, x0
	ldr	x19, [x29, 24]
	mov sp, x29
	ldp	x29, x30, [sp], 32
	ret
/* end function main */

.data
.balign 8
_f:
	.quad 0
/* end data */

.data
.balign 8
_p:
	.quad 0
/* end data */

.data
.balign 8
_p2:
	.quad 0
/* end data */

.data
.balign 8
_.s1:
	.ascii "hello %d%c"
	.byte 0
/* end data */

