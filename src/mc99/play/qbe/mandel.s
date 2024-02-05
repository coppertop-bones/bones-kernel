.text
.balign 4
.globl _plot
_plot:
	stp	x29, x30, [sp, -32]!
	mov	x29, sp
	str	x19, [x29, 24]
	str	x20, [x29, 16]
	mov	w2, w1
	mov	w1, w0
	mov	x0, #16
	sub	sp, sp, x0
	mov	x7, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x3, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x0, sp
	mov	x4, #16
	sub	sp, sp, x4
	mov	x8, sp
	mov	x4, #16
	sub	sp, sp, x4
	mov	x9, sp
	mov	x4, #16
	sub	sp, sp, x4
	mov	x10, sp
	mov	x4, #16
	sub	sp, sp, x4
	mov	x11, sp
	adrp	x4, _W@page
	add	x4, x4, _W@pageoff
	ldr	w5, [x4]
	mov	w4, #2
	sdiv	w4, w5, w4
	sub	w4, w1, w4
	mov	w6, #4000
	mul	w4, w4, w6
	sdiv	w5, w4, w5
	str	w5, [x3]
	adrp	x3, _H@page
	add	x3, x3, _H@pageoff
	ldr	w4, [x3]
	mov	w3, #2
	sdiv	w3, w4, w3
	sub	w3, w2, w3
	mov	w6, #4000
	mul	w3, w3, w6
	sdiv	w6, w3, w4
	str	w6, [x0]
	str	w5, [x8]
	str	w6, [x9]
	mov	w0, #0
	str	w0, [x7]
	mov	w0, w1
	mov	w1, w2
	mov	w4, #0
	mov	w2, w5
	mov	w3, w6
L3:
	cmp	w4, #200
	bge	L7
	mul	w12, w2, w2
	mul	w13, w3, w3
	add	w12, w12, w13
	mov	w13, #2304
	movk	w13, #0x3d, lsl #16
	cmp	w12, w13
	bgt	L6
	mov	w12, w2
	mul	w2, w2, w2
	mov	w13, #1000
	sdiv	w2, w2, w13
	mul	w13, w3, w3
	mov	w14, #1000
	sdiv	w13, w13, w14
	sub	w2, w2, w13
	add	w2, w2, w5
	str	w2, [x10]
	mul	w3, w12, w3
	mov	w12, #500
	sdiv	w3, w3, w12
	add	w3, w3, w6
	str	w3, [x11]
	str	w2, [x8]
	str	w3, [x9]
	mov	w12, #1
	add	w4, w4, w12
	str	w4, [x7]
	b	L3
L6:
	mov	w2, w1
	mov	w1, w0
	b	L8
L7:
	mov	w2, w1
	mov	w1, w0
L8:
	adrp	x0, _col@page
	add	x0, x0, _col@pageoff
	ldr	x0, [x0]
	sxtw	x4, w4
	mov	x3, #4
	mul	x3, x3, x4
	add	x0, x0, x3
	mov	w20, w2
	ldr	w2, [x0]
	str	w2, [x7]
	adrp	x0, _rnd@page
	add	x0, x0, _rnd@pageoff
	ldr	x0, [x0]
	mov	w4, #255
	mov	w3, w2
	mov	w19, w1
	mov	w1, #100
	bl	_SDL_SetRenderDrawColor
	mov	w2, w20
	mov	w1, w19
	adrp	x0, _rnd@page
	add	x0, x0, _rnd@pageoff
	ldr	x0, [x0]
	bl	_SDL_RenderDrawPoint
	ldr	x19, [x29, 24]
	ldr	x20, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 32
	ret
/* end function plot */

.text
.balign 4
.globl _main
_main:
	stp	x29, x30, [sp, -80]!
	mov	x29, sp
	str	x19, [x29, 72]
	str	x20, [x29, 64]
	str	x21, [x29, 56]
	str	x22, [x29, 48]
	str	x23, [x29, 40]
	str	x24, [x29, 32]
	str	x25, [x29, 24]
	mov	x0, #16
	sub	sp, sp, x0
	mov	x19, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x20, sp
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
	mov	x25, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x22, sp
	adrp	x1, _W@page
	add	x1, x1, _W@pageoff
	mov	w0, #800
	str	w0, [x1]
	adrp	x1, _H@page
	add	x1, x1, _H@pageoff
	mov	w0, #800
	str	w0, [x1]
	mov	w0, #32
	bl	_SDL_Init
	adrp	x0, _W@page
	add	x0, x0, _W@pageoff
	ldr	w3, [x0]
	adrp	x0, _H@page
	add	x0, x0, _H@pageoff
	ldr	w4, [x0]
	mov	w5, #0
	mov	w2, #0
	mov	w1, #0
	adrp	x0, _.s1@page
	add	x0, x0, _.s1@pageoff
	bl	_SDL_CreateWindow
	str	x0, [x22]
	mov	w2, #0
	mov	w1, #-1
	mov	x22, x0
	bl	_SDL_CreateRenderer
	mov	x1, x0
	mov	x0, x22
	adrp	x2, _rnd@page
	add	x2, x2, _rnd@pageoff
	str	x1, [x2]
	mov	x22, x0
	mov	w0, #56
	bl	_malloc
	mov	x18, x0
	mov	x0, x22
	mov	x22, x18
	str	x22, [x25]
	str	x22, [x24]
	mov	x24, x0
	mov	w0, #1608
	bl	_malloc
	mov	x1, x0
	mov	x0, x24
	adrp	x2, _col@page
	add	x2, x2, _col@pageoff
	str	x1, [x2]
	mov	w1, #20
	str	w1, [x19]
	mov	w1, #0
	str	w1, [x20]
	mov	x24, x0
	mov	w0, #20
	mov	w1, #0
L13:
	cmp	w1, #200
	bge	L15
	adrp	x2, _col@page
	add	x2, x2, _col@pageoff
	ldr	x2, [x2]
	sxtw	x4, w1
	mov	x3, #4
	mul	x3, x3, x4
	add	x2, x2, x3
	str	w0, [x2]
	mov	w2, #255
	sub	w2, w2, w0
	mov	w3, #8
	sdiv	w2, w2, w3
	add	w0, w0, w2
	str	w0, [x19]
	mov	w2, #1
	add	w1, w1, w2
	str	w1, [x20]
	b	L13
L15:
	mov	w2, w1
	mov	x0, x24
	adrp	x1, _col@page
	add	x1, x1, _col@pageoff
	ldr	x1, [x1]
	sxtw	x3, w2
	mov	x2, #4
	mul	x2, x2, x3
	add	x2, x1, x2
	mov	w1, #30
	str	w1, [x2]
	mov	x19, x0
	adrp	x0, _rnd@page
	add	x0, x0, _rnd@pageoff
	ldr	x0, [x0]
	bl	_SDL_RenderClear
	mov	x0, x19
	mov	w1, #0
	str	w1, [x23]
	mov	x24, x0
	mov	w20, #0
L18:
	adrp	x0, _W@page
	add	x0, x0, _W@pageoff
	ldr	w0, [x0]
	cmp	w20, w0
	bge	L24
	mov	w0, #0
	str	w0, [x21]
	mov	w19, #0
L21:
	adrp	x0, _H@page
	add	x0, x0, _H@pageoff
	ldr	w0, [x0]
	cmp	w19, w0
	bge	L23
	mov	w1, w19
	mov	w0, w20
	bl	_plot
	mov	w0, #1
	add	w19, w19, w0
	str	w19, [x21]
	b	L21
L23:
	mov	w0, #1
	add	w20, w20, w0
	str	w20, [x23]
	b	L18
L24:
	mov	x0, x24
	mov	x19, x0
	adrp	x0, _rnd@page
	add	x0, x0, _rnd@pageoff
	ldr	x0, [x0]
	bl	_SDL_RenderPresent
	mov	x0, x19
	mov	x24, x0
L27:
	mov	x0, x22
	bl	_SDL_PollEvent
	cmp	w0, #0
	beq	L27
	ldr	w0, [x22]
	cmp	w0, #769
	bne	L27
	mov	x0, x24
	mov	x19, x0
	adrp	x0, _rnd@page
	add	x0, x0, _rnd@pageoff
	ldr	x0, [x0]
	bl	_SDL_DestroyRenderer
	mov	x0, x19
	bl	_SDL_DestroyWindow
	bl	_SDL_Quit
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
/* end function main */

.data
.balign 8
_rnd:
	.quad 0
/* end data */

.data
.balign 8
_W:
	.int 0
/* end data */

.data
.balign 8
_H:
	.int 0
/* end data */

.data
.balign 8
_col:
	.quad 0
/* end data */

.data
.balign 8
_.s1:
	.ascii "Mandelbrot"
	.byte 0
/* end data */

