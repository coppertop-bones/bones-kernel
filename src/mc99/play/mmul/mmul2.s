.text
.balign 4
.globl _scalarProduct
_scalarProduct:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	adrp	x4, "Lfp0"@page
	add	x4, x4, "Lfp0"@pageoff
	ldr	d0, [x4]
	mov	w4, #0
L2:
	cmp	w4, w2
	bge	L4
	sxtw	x6, w4
	mov	x5, #8
	mul	x5, x5, x6
	add	x5, x0, x5
	ldr	d1, [x5]
	sxtw	x6, w4
	mov	x5, #8
	mul	x5, x5, x6
	add	x5, x1, x5
	ldr	x5, [x5]
	sxtw	x7, w3
	mov	x6, #8
	mul	x6, x6, x7
	add	x5, x5, x6
	ldr	d2, [x5]
	fmul	d1, d1, d2
	fadd	d0, d0, d1
	mov	w5, #1
	add	w4, w4, w5
	b	L2
L4:
	mov	w0, #0
	ldp	x29, x30, [sp], 16
	ret
/* end function scalarProduct */

.text
.balign 4
.globl _mmul
_mmul:
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
	mov	x26, x5
	mov	w19, #0
L8:
	cmp	w19, w2
	bge	L14
	mov	w20, #0
L10:
	cmp	w20, w4
	bge	L13
	mov	w25, w4
	sxtw	x4, w19
	mov	x21, x0
	mov	x0, #8
	mul	x0, x0, x4
	add	x0, x21, x0
	ldr	x0, [x0]
	mov	w24, w3
	mov	w3, w20
	mov	w23, w2
	mov	w2, w24
	mov	x22, x1
	bl	_scalarProduct
	mov	x5, x26
	mov	w4, w25
	mov	w3, w24
	mov	w2, w23
	mov	x1, x22
	mov	w7, w0
	mov	x0, x21
	sxtw	x8, w19
	mov	x6, #8
	mul	x6, x6, x8
	add	x6, x5, x6
	ldr	x6, [x6]
	sxtw	x9, w20
	mov	x8, #8
	mul	x8, x8, x9
	add	x6, x6, x8
	scvtf	d0, w7
	str	d0, [x6]
	mov	w6, #1
	add	w20, w20, w6
	mov	x26, x5
	b	L10
L13:
	mov	w5, #1
	add	w19, w19, w5
	b	L8
L14:
	mov	w0, #0
	ldr	x19, [x29, 72]
	ldr	x20, [x29, 64]
	ldr	x21, [x29, 56]
	ldr	x22, [x29, 48]
	ldr	x23, [x29, 40]
	ldr	x24, [x29, 32]
	ldr	x25, [x29, 24]
	ldr	x26, [x29, 16]
	ldp	x29, x30, [sp], 80
	ret
/* end function mmul */

.text
.balign 4
.globl _mmul2
_mmul2:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	w7, #0
L18:
	cmp	w7, w2
	bge	L27
	mov	w6, #0
L20:
	cmp	w6, w4
	bge	L26
	sxtw	x9, w7
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x5, x8
	ldr	x8, [x8]
	sxtw	x10, w6
	mov	x9, #8
	mul	x9, x9, x10
	add	x8, x8, x9
	adrp	x9, "Lfp0"@page
	add	x9, x9, "Lfp0"@pageoff
	ldr	d0, [x9]
	str	d0, [x8]
	sxtw	x9, w7
	mov	x8, #8
	mul	x8, x8, x9
	add	x8, x0, x8
	ldr	x9, [x8]
	mov	w8, #0
L23:
	cmp	w8, w3
	bge	L25
	sxtw	x11, w7
	mov	x10, #8
	mul	x10, x10, x11
	add	x10, x5, x10
	ldr	x10, [x10]
	sxtw	x12, w6
	mov	x11, #8
	mul	x11, x11, x12
	add	x10, x10, x11
	ldr	d0, [x10]
	sxtw	x11, w8
	mov	x10, #8
	mul	x10, x10, x11
	add	x10, x9, x10
	ldr	d1, [x10]
	sxtw	x11, w8
	mov	x10, #8
	mul	x10, x10, x11
	add	x10, x1, x10
	ldr	x10, [x10]
	sxtw	x12, w6
	mov	x11, #8
	mul	x11, x11, x12
	add	x10, x10, x11
	ldr	d2, [x10]
	fmul	d1, d1, d2
	fadd	d0, d0, d1
	sxtw	x11, w7
	mov	x10, #8
	mul	x10, x10, x11
	add	x10, x5, x10
	ldr	x10, [x10]
	sxtw	x12, w6
	mov	x11, #8
	mul	x11, x11, x12
	add	x10, x10, x11
	str	d0, [x10]
	mov	w10, #1
	add	w8, w8, w10
	b	L23
L25:
	mov	w8, #1
	add	w6, w6, w8
	b	L20
L26:
	mov	w6, #1
	add	w7, w7, w6
	b	L18
L27:
	mov	w0, #0
	ldp	x29, x30, [sp], 16
	ret
/* end function mmul2 */

.data
.balign 8
_g1:
	.int 0
/* end data */

/* floating point constants */
.section __TEXT,__literal8,8byte_literals
.p2align 3
Lfp0:
	.int 0
	.int 0 /* 0.000000 */

