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
	str	x24, [x29, 16]
	mov	x2, #16
	sub	sp, sp, x2
	mov	x21, sp
	mov	x2, #16
	sub	sp, sp, x2
	mov	x22, sp
	mov	w2, #0
	str	w2, [x22]
	mov	w20, #0
L3:
	cmp	w20, w1
	bge	L14
	mov	w23, w1
	mov	w1, #0
	str	w1, [x21]
	mov	w1, w23
	mov	w19, #0
L6:
	cmp	w19, w1
	bge	L12
	sxtw	x2, w19
	mov	w24, w1
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
	bne	L10
	mov	x23, x0
	adrp	x0, _s10@page
	add	x0, x0, _s10@pageoff
	bl	_printf
	mov	x0, x23
	mov	w1, w24
	b	L11
L10:
	mov	x23, x0
	adrp	x0, _s9@page
	add	x0, x0, _s9@pageoff
	bl	_printf
	mov	w1, w24
	mov	x0, x23
L11:
	mov	w2, #1
	add	w19, w19, w2
	str	w19, [x21]
	b	L6
L12:
	mov	w23, w1
	mov	x19, x0
	adrp	x0, _s11@page
	add	x0, x0, _s11@pageoff
	bl	_printf
	mov	w1, w23
	mov	x0, x19
	mov	w2, #1
	add	w20, w20, w2
	str	w20, [x22]
	b	L3
L14:
	adrp	x0, _s12@page
	add	x0, x0, _s12@pageoff
	bl	_printf
	ldr	x19, [x29, 56]
	ldr	x20, [x29, 48]
	ldr	x21, [x29, 40]
	ldr	x22, [x29, 32]
	ldr	x23, [x29, 24]
	ldr	x24, [x29, 16]
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
L19:
	cmp	w5, w3
	bge	L30
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
	beq	L23
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
L23:
	add	w8, w0, w5
	cmp	w8, w3
	cset	w8, lt
	sub	w9, w1, w5
	cmp	w9, #0
	cset	w9, ge
	and	w8, w8, w9
	cmp	w8, #0
	beq	L25
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
L25:
	sub	w8, w0, w5
	cmp	w8, #0
	cset	w8, ge
	add	w9, w1, w5
	cmp	w9, w3
	cset	w9, lt
	and	w8, w8, w9
	cmp	w8, #0
	beq	L27
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
L27:
	sub	w8, w0, w5
	cmp	w8, #0
	cset	w8, ge
	sub	w9, w1, w5
	cmp	w9, #0
	cset	w9, ge
	and	w8, w8, w9
	cmp	w8, #0
	beq	L29
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
L29:
	mov	w8, #1
	add	w5, w5, w8
	str	w5, [x7]
	b	L19
L30:
	mov	w0, w6
	mov sp, x29
	ldp	x29, x30, [sp], 16
	ret
/* end function chk */

.text
.balign 4
.globl _go
_go:
	stp	x29, x30, [sp, -64]!
	mov	x29, sp
	str	x19, [x29, 56]
	str	x20, [x29, 48]
	str	x21, [x29, 40]
	str	x22, [x29, 32]
	str	x23, [x29, 24]
	str	x24, [x29, 16]
	mov	x21, x1
	mov	w1, w3
	mov	x3, #16
	sub	sp, sp, x3
	mov	x20, sp
	cmp	w0, w1
	beq	L42
	mov	w19, w2
	mov	w2, #0
	str	w2, [x20]
	mov	w3, w1
	mov	x1, x21
	mov	w2, w19
	mov	w19, #0
L37:
	cmp	w19, w3
	bge	L41
	mov	w24, w3
	mov	w23, w2
	mov	x2, x1
	mov	x22, x1
	mov	w1, w0
	mov	w21, w0
	mov	w0, w19
	bl	_chk
	mov	w3, w24
	mov	w2, w23
	mov	x1, x22
	mov	w4, w0
	mov	w0, w21
	cmp	w4, #0
	bne	L40
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
	mov	w21, w0
	mov	w0, #1
	add	w0, w21, w0
	mov	w23, w3
	mov	x22, x1
	bl	_go
	mov	w3, w23
	mov	x1, x22
	mov	w2, w0
	mov	w0, w21
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
L40:
	mov	w4, #1
	add	w19, w19, w4
	str	w19, [x20]
	b	L37
L41:
	mov	w0, w2
	b	L44
L42:
	mov	w19, w2
	mov	x0, x21
	bl	_print
	mov	w2, w19
	mov	w0, #1
	add	w0, w2, w0
L44:
	ldr	x19, [x29, 56]
	ldr	x20, [x29, 48]
	ldr	x21, [x29, 40]
	ldr	x22, [x29, 32]
	ldr	x23, [x29, 24]
	ldr	x24, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 64
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
L48:
	cmp	w19, w0
	bge	L50
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
	b	L48
L50:
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
	bge	L56
	mov	w3, #8
	b	L57
L56:
	mov	x0, #8
	add	x0, x1, x0
	ldr	x0, [x0]
	bl	_atoi
	mov	w3, w0
	str	w3, [x21]
L57:
	mov	w21, w3
	mov	w0, w21
	bl	_newBoard
	mov	w3, w21
	mov	x1, x0
	str	x1, [x20]
	mov	w2, #0
	mov	w0, #0
	bl	_go
	str	w0, [x19]
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _s17@page
	add	x0, x0, _s17@pageoff
	bl	_printf
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

