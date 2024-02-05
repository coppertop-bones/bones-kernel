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
	str	x24, [x29, 16]
	mov	x0, #16
	sub	sp, sp, x0
	mov	x23, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x21, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x24, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x22, sp
	mov	w0, #0
	str	w0, [x24]
	mov	w0, #2
	str	w0, [x23]
	mov	w20, #2
	mov	w19, #0
L3:
	mov	w0, #5000
	cmp	w20, w0
	bge	L16
	mov	w0, #2
	str	w0, [x21]
	mov	w0, #1
	str	w0, [x22]
	mov	w1, #1
	mov	w0, #2
L6:
	mul	w2, w0, w0
	cmp	w2, w20
	bgt	L11
	sdiv	w18, w20, w0
	msub	w2, w18, w0, w20
	cmp	w2, #0
	bne	L10
	mov	w1, #0
	str	w1, [x22]
	mov	w1, #0
L10:
	mov	w2, #1
	add	w0, w0, w2
	str	w0, [x21]
	b	L6
L11:
	cmp	w1, #0
	beq	L15
	mov	w0, #10
	sdiv	w18, w19, w0
	msub	w0, w18, w0, w19
	cmp	w0, #0
	cset	w0, eq
	add	w0, w19, w0
	cmp	w0, #0
	beq	L14
	adrp	x0, _.s1@page
	add	x0, x0, _.s1@pageoff
	bl	_printf
L14:
	mov	x0, #16
	sub	sp, sp, x0
	mov	x0, #0
	add	x0, sp, x0
	str	w20, [x0]
	adrp	x0, _.s2@page
	add	x0, x0, _.s2@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	mov	w0, #1
	add	w19, w19, w0
	str	w19, [x24]
L15:
	mov	w0, #1
	add	w20, w20, w0
	str	w20, [x23]
	b	L3
L16:
	adrp	x0, _.s3@page
	add	x0, x0, _.s3@pageoff
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
/* end function main */

.data
.balign 8
_.s1:
	.ascii "\n"
	.byte 0
/* end data */

.data
.balign 8
_.s2:
	.ascii "%4d "
	.byte 0
/* end data */

.data
.balign 8
_.s3:
	.ascii "\n"
	.byte 0
/* end data */

