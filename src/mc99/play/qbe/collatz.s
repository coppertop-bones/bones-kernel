.text
.balign 4
.globl _main
_main:
	stp	x29, x30, [sp, -64]!
	mov	x29, sp
	str	x19, [x29, 56]
	str	x20, [x29, 48]
	str	x21, [x29, 40]
	str	x22, [x29, 32]
	str	x23, [x29, 24]
	mov	x0, #16
	sub	sp, sp, x0
	mov	x19, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x21, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x20, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x22, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x23, sp
	mov	w0, #32000
	bl	_malloc
	mov	x4, x0
	str	x4, [x23]
	mov	w0, #0
	str	w0, [x22]
	mov	w0, #1
	str	w0, [x21]
	mov	w0, #0
	mov	w1, #1
L3:
	cmp	w1, #1000
	bge	L19
	str	w1, [x19]
	mov	w2, #0
	str	w2, [x20]
	mov	w3, w1
	mov	w2, w0
	mov	w0, #0
L6:
	cmp	w1, #1
	beq	L14
	cmp	w1, w3
	blt	L12
	mov	w5, #1
	and	w5, w1, w5
	cmp	w5, #0
	bne	L10
	mov	w5, #2
	sdiv	w1, w1, w5
	str	w1, [x19]
	b	L11
L10:
	mov	w5, #3
	mul	w1, w5, w1
	mov	w5, #1
	add	w1, w1, w5
	str	w1, [x19]
L11:
	mov	w5, #1
	add	w0, w0, w5
	str	w0, [x20]
	b	L6
L12:
	mov	w18, w1
	mov	w1, w3
	mov	w3, w18
	sxtw	x3, w3
	mov	w5, w3
	mov	x3, #4
	mul	x3, x3, x5
	add	x3, x4, x3
	ldr	w3, [x3]
	add	w0, w0, w3
	str	w0, [x20]
	b	L15
L14:
	mov	w1, w3
L15:
	sxtw	x3, w1
	mov	w5, w3
	mov	x3, #4
	mul	x3, x3, x5
	add	x3, x4, x3
	str	w0, [x3]
	cmp	w2, w0
	blt	L17
	mov	w0, w2
	b	L18
L17:
	str	w0, [x22]
L18:
	mov	w3, w1
	mov	w1, #1
	add	w1, w3, w1
	str	w1, [x21]
	b	L3
L19:
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _g7@page
	add	x0, x0, _g7@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	ldr	x19, [x29, 56]
	ldr	x20, [x29, 48]
	ldr	x21, [x29, 40]
	ldr	x22, [x29, 32]
	ldr	x23, [x29, 24]
	mov sp, x29
	ldp	x29, x30, [sp], 64
	ret
/* end function main */

.data
.balign 8
_g7:
	.ascii "should print 178: %d\n"
	.byte 0
/* end data */

