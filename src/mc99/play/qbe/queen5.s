.text
.balign 4
.globl _print
_print:
	stp	x29, x30, [sp, -80]!
	mov	x29, sp
	str	x19, [x29, 72]
	str	x20, [x29, 64]
	str	x21, [x29, 56]
	str	x22, [x29, 48]
	str	x23, [x29, 40]
	str	x24, [x29, 32]
	str	x25, [x29, 24]
	mov	x19, x0
	mov	x0, x2
	mov	x2, #16
	sub	sp, sp, x2
	mov	x21, sp
	mov	x2, #16
	sub	sp, sp, x2
	mov	x22, sp
	mov	w2, #0
	str	w2, [x22]
	mov	x2, x0
	mov	x0, x19
	mov	w20, #0
L3:
	cmp	w20, w1
	bge	L14
	mov	x24, x2
	mov	w2, #0
	str	w2, [x21]
	mov	x2, x24
	mov	w19, #0
L6:
	cmp	w19, w1
	bge	L12
	sxtw	x3, w19
	mov	x25, x2
	mov	x2, #8
	mul	x2, x2, x3
	add	x2, x0, x2
	ldr	x2, [x2]
	sxtw	x4, w20
	mov	x3, #4
	mul	x3, x3, x4
	add	x2, x2, x3
	ldr	w2, [x2]
	cmp	w2, #0
	bne	L10
	mov	w24, w1
	adrp	x1, _s10@page
	add	x1, x1, _s10@pageoff
	mov	x23, x0
	mov	x0, x25
	bl	_fprintf
	mov	w1, w24
	mov	x0, x23
	mov	x2, x25
	b	L11
L10:
	mov	w24, w1
	adrp	x1, _s9@page
	add	x1, x1, _s9@pageoff
	mov	x23, x0
	mov	x0, x25
	bl	_fprintf
	mov	x2, x25
	mov	w1, w24
	mov	x0, x23
L11:
	mov	w3, #1
	add	w19, w19, w3
	str	w19, [x21]
	b	L6
L12:
	mov	x24, x2
	mov	w23, w1
	adrp	x1, _s11@page
	add	x1, x1, _s11@pageoff
	mov	x19, x0
	mov	x0, x24
	bl	_fprintf
	mov	x2, x24
	mov	w1, w23
	mov	x0, x19
	mov	w3, #1
	add	w20, w20, w3
	str	w20, [x22]
	b	L3
L14:
	mov	x0, x2
	adrp	x1, _s12@page
	add	x1, x1, _s12@pageoff
	bl	_fprintf
	ldr	x19, [x29, 72]
	ldr	x20, [x29, 64]
	ldr	x21, [x29, 56]
	ldr	x22, [x29, 48]
	ldr	x23, [x29, 40]
	ldr	x24, [x29, 32]
	ldr	x25, [x29, 24]
	mov sp, x29
	ldp	x29, x30, [sp], 80
	ret
/* end function print */

.text
.balign 4
.globl _chk
_chk:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	x4, #16
	sub	sp, sp, x4
	mov	x7, sp
	mov	x4, #16
	sub	sp, sp, x4
	mov	x4, sp
	mov	w5, #0
	str	w5, [x7]
	mov	w5, #0
	str	w5, [x4]
	mov	w5, #0
	mov	w6, #0
L20:
	cmp	w5, w3
	bge	L31
	sxtw	x9, w0
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x2, x8
	ldr	x8, [x8]
	sxtw	x10, w5
	mov	x9, #4
	mul	x9, x9, x10
	add	x8, x8, x9
	ldr	w8, [x8]
	add	w6, w6, w8
	str	w6, [x4]
	sxtw	x9, w5
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x2, x8
	ldr	x8, [x8]
	sxtw	x10, w1
	mov	x9, #4
	mul	x9, x9, x10
	add	x8, x8, x9
	ldr	w8, [x8]
	add	w6, w6, w8
	str	w6, [x4]
	add	w8, w0, w5
	cmp	w8, w3
	cset	w8, lt
	add	w9, w1, w5
	cmp	w9, w3
	cset	w9, lt
	and	w8, w8, w9
	cmp	w8, #0
	beq	L24
	add	w8, w0, w5
	sxtw	x9, w8
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x2, x8
	ldr	x8, [x8]
	add	w9, w1, w5
	sxtw	x10, w9
	mov	x9, #4
	mul	x9, x9, x10
	add	x8, x8, x9
	ldr	w8, [x8]
	add	w6, w6, w8
	str	w6, [x4]
L24:
	add	w8, w0, w5
	cmp	w8, w3
	cset	w8, lt
	sub	w9, w1, w5
	cmp	w9, #0
	cset	w9, ge
	and	w8, w8, w9
	cmp	w8, #0
	beq	L26
	add	w8, w0, w5
	sxtw	x9, w8
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x2, x8
	ldr	x8, [x8]
	sub	w9, w1, w5
	sxtw	x10, w9
	mov	x9, #4
	mul	x9, x9, x10
	add	x8, x8, x9
	ldr	w8, [x8]
	add	w6, w6, w8
	str	w6, [x4]
L26:
	sub	w8, w0, w5
	cmp	w8, #0
	cset	w8, ge
	add	w9, w1, w5
	cmp	w9, w3
	cset	w9, lt
	and	w8, w8, w9
	cmp	w8, #0
	beq	L28
	sub	w8, w0, w5
	sxtw	x9, w8
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x2, x8
	ldr	x8, [x8]
	add	w9, w1, w5
	sxtw	x10, w9
	mov	x9, #4
	mul	x9, x9, x10
	add	x8, x8, x9
	ldr	w8, [x8]
	add	w6, w6, w8
	str	w6, [x4]
L28:
	sub	w8, w0, w5
	cmp	w8, #0
	cset	w8, ge
	sub	w9, w1, w5
	cmp	w9, #0
	cset	w9, ge
	and	w8, w8, w9
	cmp	w8, #0
	beq	L30
	sub	w8, w0, w5
	sxtw	x9, w8
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x2, x8
	ldr	x8, [x8]
	sub	w9, w1, w5
	sxtw	x10, w9
	mov	x9, #4
	mul	x9, x9, x10
	add	x8, x8, x9
	ldr	w8, [x8]
	add	w6, w6, w8
	str	w6, [x4]
L30:
	mov	w8, #1
	add	w5, w5, w8
	str	w5, [x7]
	b	L20
L31:
	mov	w0, w6
	mov sp, x29
	ldp	x29, x30, [sp], 16
	ret
/* end function chk */

.text
.balign 4
.globl _go
_go:
	stp	x29, x30, [sp, -80]!
	mov	x29, sp
	str	x19, [x29, 72]
	str	x20, [x29, 64]
	str	x21, [x29, 56]
	str	x22, [x29, 48]
	str	x23, [x29, 40]
	str	x24, [x29, 32]
	str	x25, [x29, 24]
	str	x26, [x29, 16]
	mov	x19, x1
	mov	w1, w2
	mov	x2, x3
	mov	x3, #16
	sub	sp, sp, x3
	mov	x21, sp
	mov	x3, #16
	sub	sp, sp, x3
	mov	x22, sp
	mov	w3, #0
	str	w3, [x22]
	cmp	w0, w1
	beq	L43
	mov	w3, #0
	str	w3, [x21]
	mov	x3, x2
	mov	w2, w1
	mov	x1, x19
	mov	w19, #0
	mov	w20, #0
L38:
	cmp	w19, w2
	bge	L42
	mov	x26, x3
	mov	w3, w2
	mov	w25, w2
	mov	x2, x1
	mov	x24, x1
	mov	w1, w0
	mov	w23, w0
	mov	w0, w19
	bl	_chk
	mov	x3, x26
	mov	w2, w25
	mov	x1, x24
	mov	w4, w0
	mov	w0, w23
	cmp	w4, #0
	bne	L41
	sxtw	x5, w19
	mov	x4, #8
	mul	x4, x4, x5
	add	x4, x1, x4
	ldr	x4, [x4]
	sxtw	x6, w0
	mov	x5, #4
	mul	x5, x5, x6
	add	x5, x4, x5
	ldr	w4, [x5]
	mov	w6, #1
	add	w4, w4, w6
	str	w4, [x5]
	mov	w23, w0
	mov	w0, #1
	add	w0, w23, w0
	mov	x26, x3
	mov	w25, w2
	mov	x24, x1
	bl	_go
	mov	x3, x26
	mov	w2, w25
	mov	x1, x24
	mov	w4, w0
	mov	w0, w23
	add	w20, w20, w4
	str	w20, [x22]
	sxtw	x5, w19
	mov	x4, #8
	mul	x4, x4, x5
	add	x4, x1, x4
	ldr	x4, [x4]
	sxtw	x6, w0
	mov	x5, #4
	mul	x5, x5, x6
	add	x5, x4, x5
	ldr	w4, [x5]
	mov	w6, #1
	sub	w4, w4, w6
	str	w4, [x5]
L41:
	mov	w4, #1
	add	w19, w19, w4
	str	w19, [x21]
	b	L38
L42:
	mov	w0, w20
	b	L45
L43:
	mov	x0, x19
	bl	_print
	mov	w0, #1
L45:
	ldr	x19, [x29, 72]
	ldr	x20, [x29, 64]
	ldr	x21, [x29, 56]
	ldr	x22, [x29, 48]
	ldr	x23, [x29, 40]
	ldr	x24, [x29, 32]
	ldr	x25, [x29, 24]
	ldr	x26, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 80
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
L49:
	cmp	w19, w0
	bge	L51
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
	b	L49
L51:
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
	stp	x29, x30, [sp, -48]!
	mov	x29, sp
	str	x19, [x29, 40]
	str	x20, [x29, 32]
	str	x21, [x29, 24]
	mov	x2, #16
	sub	sp, sp, x2
	mov	x20, sp
	mov	x2, #16
	sub	sp, sp, x2
	mov	x19, sp
	mov	x2, #16
	sub	sp, sp, x2
	mov	x21, sp
	mov	w2, #8
	str	w2, [x21]
	cmp	w0, #2
	bge	L57
	mov	w2, #8
	b	L58
L57:
	mov	x0, #8
	add	x0, x1, x0
	ldr	x0, [x0]
	bl	_atoi
	mov	w2, w0
	str	w2, [x21]
L58:
	mov	w21, w2
	mov	w0, w21
	bl	_newBoard
	mov	w2, w21
	mov	x1, x0
	str	x1, [x20]
	adrp	x0, ___stdoutp@gotpage
	ldr	x0, [x0, ___stdoutp@gotpageoff]
	ldr	x3, [x0]
	mov	w0, #0
	bl	_go
	mov	w1, w0
	str	w1, [x19]
	adrp	x0, ___stdoutp@gotpage
	ldr	x0, [x0, ___stdoutp@gotpageoff]
	ldr	x0, [x0]
	mov	x2, #16
	sub	sp, sp, x2
	mov	x2, #0
	add	x2, sp, x2
	str	w1, [x2]
	adrp	x1, _s17@page
	add	x1, x1, _s17@pageoff
	bl	_fprintf
	mov	x0, #16
	add	sp, sp, x0
	ldr	x19, [x29, 40]
	ldr	x20, [x29, 32]
	ldr	x21, [x29, 24]
	mov sp, x29
	ldp	x29, x30, [sp], 48
	ret
/* end function main */

.data
.balign 8
_s9:
	.ascii " Q"
	.byte 0
/* end data */

.data
.balign 8
_s10:
	.ascii " ."
	.byte 0
/* end data */

.data
.balign 8
_s11:
	.ascii "\n"
	.byte 0
/* end data */

.data
.balign 8
_s12:
	.ascii "\n"
	.byte 0
/* end data */

.data
.balign 8
_s17:
	.ascii "found %d solutions\n"
	.byte 0
/* end data */

