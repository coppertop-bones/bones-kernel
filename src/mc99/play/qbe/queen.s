.text
.balign 4
.globl _print
_print:
	stp	x29, x30, [sp, -64]!
	mov	x29, sp
	str	x19, [x29, 56]
	str	x20, [x29, 48]
	str	x21, [x29, 40]
	str	x22, [x29, 32]
	str	x23, [x29, 24]
	mov	x1, #16
	sub	sp, sp, x1
	mov	x21, sp
	mov	x1, #16
	sub	sp, sp, x1
	mov	x22, sp
	mov	w1, #0
	str	w1, [x22]
	mov	w20, #0
L3:
	adrp	x1, _Q@page
	add	x1, x1, _Q@pageoff
	ldr	w1, [x1]
	cmp	w20, w1
	bge	L12
	mov	w1, #0
	str	w1, [x21]
	mov	w19, #0
L6:
	adrp	x1, _Q@page
	add	x1, x1, _Q@pageoff
	ldr	w1, [x1]
	cmp	w19, w1
	bge	L11
	sxtw	x2, w19
	mov	x1, #8
	mul	x1, x1, x2
	add	x1, x0, x1
	ldr	x1, [x1]
	sxtw	x3, w20
	mov	x2, #4
	mul	x2, x2, x3
	add	x1, x1, x2
	ldr	w1, [x1]
	cmp	w1, #0
	bne	L9
	mov	x23, x0
	adrp	x0, _.s2@page
	add	x0, x0, _.s2@pageoff
	bl	_printf
	mov	x0, x23
	b	L10
L9:
	mov	x23, x0
	adrp	x0, _.s1@page
	add	x0, x0, _.s1@pageoff
	bl	_printf
	mov	x0, x23
L10:
	mov	w1, #1
	add	w19, w19, w1
	str	w19, [x21]
	b	L6
L11:
	mov	x19, x0
	adrp	x0, _.s3@page
	add	x0, x0, _.s3@pageoff
	bl	_printf
	mov	x0, x19
	mov	w1, #1
	add	w20, w20, w1
	str	w20, [x22]
	b	L3
L12:
	adrp	x0, _.s4@page
	add	x0, x0, _.s4@pageoff
	bl	_printf
	ldr	x19, [x29, 56]
	ldr	x20, [x29, 48]
	ldr	x21, [x29, 40]
	ldr	x22, [x29, 32]
	ldr	x23, [x29, 24]
	mov sp, x29
	ldp	x29, x30, [sp], 64
	ret
/* end function print */

.text
.balign 4
.globl _chk
_chk:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	x3, #16
	sub	sp, sp, x3
	mov	x6, sp
	mov	x3, #16
	sub	sp, sp, x3
	mov	x3, sp
	mov	w4, #0
	str	w4, [x6]
	mov	w4, #0
	str	w4, [x3]
	mov	w4, #0
	mov	w5, #0
L17:
	adrp	x7, _Q@page
	add	x7, x7, _Q@pageoff
	ldr	w9, [x7]
	cmp	w4, w9
	bge	L28
	sxtw	x8, w0
	mov	x7, #8
	mul	x7, x7, x8
	add	x7, x2, x7
	ldr	x7, [x7]
	sxtw	x10, w4
	mov	x8, #4
	mul	x8, x8, x10
	add	x7, x7, x8
	ldr	w7, [x7]
	add	w5, w5, w7
	str	w5, [x3]
	sxtw	x8, w4
	mov	x7, #8
	mul	x7, x7, x8
	add	x7, x2, x7
	ldr	x7, [x7]
	sxtw	x10, w1
	mov	x8, #4
	mul	x8, x8, x10
	add	x7, x7, x8
	ldr	w7, [x7]
	add	w5, w5, w7
	str	w5, [x3]
	add	w7, w0, w4
	cmp	w7, w9
	cset	w7, lt
	add	w8, w1, w4
	cmp	w8, w9
	cset	w8, lt
	and	w7, w7, w8
	cmp	w7, #0
	beq	L21
	add	w7, w0, w4
	sxtw	x8, w7
	mov	x7, #8
	mul	x7, x7, x8
	add	x7, x2, x7
	ldr	x7, [x7]
	add	w8, w1, w4
	sxtw	x10, w8
	mov	x8, #4
	mul	x8, x8, x10
	add	x7, x7, x8
	ldr	w7, [x7]
	add	w5, w5, w7
	str	w5, [x3]
L21:
	add	w7, w0, w4
	cmp	w7, w9
	cset	w7, lt
	sub	w8, w1, w4
	cmp	w8, #0
	cset	w8, ge
	and	w7, w7, w8
	cmp	w7, #0
	beq	L23
	add	w7, w0, w4
	sxtw	x8, w7
	mov	x7, #8
	mul	x7, x7, x8
	add	x7, x2, x7
	ldr	x7, [x7]
	sub	w8, w1, w4
	sxtw	x10, w8
	mov	x8, #4
	mul	x8, x8, x10
	add	x7, x7, x8
	ldr	w7, [x7]
	add	w5, w5, w7
	str	w5, [x3]
L23:
	sub	w7, w0, w4
	cmp	w7, #0
	cset	w7, ge
	add	w8, w1, w4
	cmp	w8, w9
	cset	w8, lt
	and	w7, w7, w8
	cmp	w7, #0
	beq	L25
	sub	w7, w0, w4
	sxtw	x8, w7
	mov	x7, #8
	mul	x7, x7, x8
	add	x7, x2, x7
	ldr	x7, [x7]
	add	w8, w1, w4
	sxtw	x9, w8
	mov	x8, #4
	mul	x8, x8, x9
	add	x7, x7, x8
	ldr	w7, [x7]
	add	w5, w5, w7
	str	w5, [x3]
L25:
	sub	w7, w0, w4
	cmp	w7, #0
	cset	w7, ge
	sub	w8, w1, w4
	cmp	w8, #0
	cset	w8, ge
	and	w7, w7, w8
	cmp	w7, #0
	beq	L27
	sub	w7, w0, w4
	sxtw	x8, w7
	mov	x7, #8
	mul	x7, x7, x8
	add	x7, x2, x7
	ldr	x7, [x7]
	sub	w8, w1, w4
	sxtw	x9, w8
	mov	x8, #4
	mul	x8, x8, x9
	add	x7, x7, x8
	ldr	w7, [x7]
	add	w5, w5, w7
	str	w5, [x3]
L27:
	mov	w7, #1
	add	w4, w4, w7
	str	w4, [x6]
	b	L17
L28:
	mov	w0, w5
	mov sp, x29
	ldp	x29, x30, [sp], 16
	ret
/* end function chk */

.text
.balign 4
.globl _go
_go:
	stp	x29, x30, [sp, -48]!
	mov	x29, sp
	str	x19, [x29, 40]
	str	x20, [x29, 32]
	str	x21, [x29, 24]
	str	x22, [x29, 16]
	mov	x19, x1
	mov	x1, #16
	sub	sp, sp, x1
	mov	x20, sp
	adrp	x1, _Q@page
	add	x1, x1, _Q@pageoff
	ldr	w1, [x1]
	cmp	w0, w1
	beq	L39
	mov	w1, #0
	str	w1, [x20]
	mov	x1, x19
	mov	w19, #0
L35:
	adrp	x2, _Q@page
	add	x2, x2, _Q@pageoff
	ldr	w2, [x2]
	cmp	w19, w2
	bge	L41
	mov	x2, x1
	mov	x22, x1
	mov	w1, w0
	mov	w21, w0
	mov	w0, w19
	bl	_chk
	mov	x1, x22
	mov	w2, w0
	mov	w0, w21
	cmp	w2, #0
	bne	L38
	sxtw	x3, w19
	mov	x2, #8
	mul	x2, x2, x3
	add	x2, x1, x2
	ldr	x2, [x2]
	sxtw	x4, w0
	mov	x3, #4
	mul	x3, x3, x4
	add	x3, x2, x3
	ldr	w2, [x3]
	mov	w4, #1
	add	w2, w2, w4
	str	w2, [x3]
	mov	w21, w0
	mov	w0, #1
	add	w0, w21, w0
	mov	x22, x1
	bl	_go
	mov	x1, x22
	mov	w0, w21
	sxtw	x3, w19
	mov	x2, #8
	mul	x2, x2, x3
	add	x2, x1, x2
	ldr	x2, [x2]
	sxtw	x4, w0
	mov	x3, #4
	mul	x3, x3, x4
	add	x3, x2, x3
	ldr	w2, [x3]
	mov	w4, #1
	sub	w2, w2, w4
	str	w2, [x3]
L38:
	mov	w2, #1
	add	w19, w19, w2
	str	w19, [x20]
	b	L35
L39:
	mov	x0, x19
	bl	_print
	adrp	x0, _nSolutions@page
	add	x0, x0, _nSolutions@pageoff
	ldr	w0, [x0]
	mov	w1, #1
	add	w0, w0, w1
	adrp	x1, _nSolutions@page
	add	x1, x1, _nSolutions@pageoff
	str	w0, [x1]
L41:
	ldr	x19, [x29, 40]
	ldr	x20, [x29, 32]
	ldr	x21, [x29, 24]
	ldr	x22, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 48
	ret
/* end function go */

.text
.balign 4
.globl _newBoard
_newBoard:
	stp	x29, x30, [sp, -48]!
	mov	x29, sp
	str	x19, [x29, 40]
	str	x20, [x29, 32]
	str	x21, [x29, 24]
	str	x22, [x29, 16]
	mov	x1, #16
	sub	sp, sp, x1
	mov	x20, sp
	mov	x1, #16
	sub	sp, sp, x1
	mov	x21, sp
	mov	w1, #8
	mov	w19, w0
	bl	_calloc
	mov	x18, x0
	mov	x0, x19
	mov	x19, x18
	str	x19, [x20]
	mov	w1, #0
	str	w1, [x21]
	mov	x20, x19
	mov	w19, #0
L45:
	cmp	w19, w0
	bge	L47
	mov	w1, #8
	mov	w22, w0
	bl	_calloc
	mov	x1, x0
	mov	w0, w22
	sxtw	x3, w19
	mov	x2, #8
	mul	x2, x2, x3
	add	x2, x20, x2
	str	x1, [x2]
	mov	w1, #1
	add	w19, w19, w1
	str	w19, [x21]
	b	L45
L47:
	mov	x0, x20
	ldr	x19, [x29, 40]
	ldr	x20, [x29, 32]
	ldr	x21, [x29, 24]
	ldr	x22, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 48
	ret
/* end function newBoard */

.text
.balign 4
.globl _main
_main:
	stp	x29, x30, [sp, -32]!
	mov	x29, sp
	str	x19, [x29, 24]
	mov	x2, #16
	sub	sp, sp, x2
	mov	x19, sp
	adrp	x3, _Q@page
	add	x3, x3, _Q@pageoff
	mov	w2, #8
	str	w2, [x3]
	cmp	w0, #2
	bge	L53
	mov	w0, #8
	b	L54
L53:
	mov	x0, #8
	add	x0, x1, x0
	ldr	x0, [x0]
	bl	_atoi
	adrp	x1, _Q@page
	add	x1, x1, _Q@pageoff
	str	w0, [x1]
L54:
	bl	_newBoard
	mov	x1, x0
	str	x1, [x19]
	adrp	x2, _nSolutions@page
	add	x2, x2, _nSolutions@pageoff
	mov	w0, #0
	str	w0, [x2]
	mov	w0, #0
	bl	_go
	adrp	x0, _nSolutions@page
	add	x0, x0, _nSolutions@pageoff
	ldr	w0, [x0]
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _.s5@page
	add	x0, x0, _.s5@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	ldr	x19, [x29, 24]
	mov sp, x29
	ldp	x29, x30, [sp], 32
	ret
/* end function main */

.data
.balign 8
_Q:
	.int 0
/* end data */

.data
.balign 8
_nSolutions:
	.int 0
/* end data */

.data
.balign 8
_.s1:
	.ascii " Q"
	.byte 0
/* end data */

.data
.balign 8
_.s2:
	.ascii " ."
	.byte 0
/* end data */

.data
.balign 8
_.s3:
	.ascii "\n"
	.byte 0
/* end data */

.data
.balign 8
_.s4:
	.ascii "\n"
	.byte 0
/* end data */

.data
.balign 8
_.s5:
	.ascii "found %d solutions\n"
	.byte 0
/* end data */

