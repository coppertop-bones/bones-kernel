.text
.balign 4
.globl _main
_main:
    stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x2, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x8, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x5, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x6, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x7, sp
	mov	w0, #1
	str	w0, [x8]
	mov	w4, #1
L3:
	cmp	w4, #1000
	bge	L15
	mov	w0, #1
	add	w3, w4, w0
	str	w3, [x5]
L5:
	cmp	w3, #1000
	bge	L14
	mul	w0, w4, w4
	mul	w1, w3, w3
	add	w1, w0, w1
	str	w1, [x7]
	mov	w0, #0
	str	w0, [x2]
	mov	w0, #0
L8:
	cmp	w0, #1000
	bge	L13
	mul	w9, w0, w0
	cmp	w9, w1
	beq	L11
	mov	w9, #1
	add	w0, w0, w9
	str	w0, [x2]
	b	L8
L11:
	str	w0, [x6]
	cmp	w3, w0
	cset	w1, lt
	add	w9, w4, w3
	add	w9, w9, w0
	cmp	w9, #1000
	cset	w9, eq
	add	w1, w1, w9
	cmp	w1, #0
	beq	L13
	mul	w1, w4, w3
	mul	w0, w1, w0
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _g2@page
	add	x0, x0, _g2@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	mov	w0, #0
	b	L15
L13:
	mov	w0, #1
	add	w3, w3, w0
	str	w3, [x5]
	b	L5
L14:
	mov	w0, #1
	add	w4, w4, w0
	str	w4, [x8]
	b	L3
L15:
	mov sp, x29
	ldp	x29, x30, [sp], 16
	ret
/* end function main */

.data
.balign 8
_g2:
	.ascii "%d\n"
	.byte 0
/* end data */

