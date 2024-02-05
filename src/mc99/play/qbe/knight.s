.text
.balign 4
.globl _board
_board:
	stp	x29, x30, [sp, -48]!
	mov	x29, sp
	str	x19, [x29, 40]
	str	x20, [x29, 32]
	str	x21, [x29, 24]
	str	x22, [x29, 16]
	mov	x0, #16
	sub	sp, sp, x0
	mov	x21, sp
	mov	x0, #16
	sub	sp, sp, x0
	mov	x22, sp
	adrp	x0, _t@page
	add	x0, x0, _t@pageoff
	ldr	x0, [x0]
	bl	_time
	adrp	x0, _t@page
	add	x0, x0, _t@pageoff
	ldr	x0, [x0]
	bl	_ctime
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	x0, [x1]
	adrp	x0, _.s1@page
	add	x0, x0, _.s1@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	mov	w0, #0
	str	w0, [x22]
	mov	w20, #0
L3:
	cmp	w20, #8
	bge	L9
	mov	w0, #0
	str	w0, [x21]
	mov	w19, #0
L6:
	cmp	w19, #8
	bge	L8
	adrp	x0, _b@page
	add	x0, x0, _b@pageoff
	ldr	x0, [x0]
	sxtw	x2, w19
	mov	x1, #8
	mul	x1, x1, x2
	add	x0, x0, x1
	ldr	x0, [x0]
	sxtw	x2, w20
	mov	x1, #4
	mul	x1, x1, x2
	add	x0, x0, x1
	ldr	w0, [x0]
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	w0, [x1]
	adrp	x0, _.s2@page
	add	x0, x0, _.s2@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	mov	w0, #1
	add	w19, w19, w0
	str	w19, [x21]
	b	L6
L8:
	adrp	x0, _.s3@page
	add	x0, x0, _.s3@pageoff
	bl	_printf
	mov	w0, #1
	add	w20, w20, w0
	str	w20, [x22]
	b	L3
L9:
	adrp	x0, _.s4@page
	add	x0, x0, _.s4@pageoff
	bl	_printf
	mov	w0, #0
	ldr	x19, [x29, 40]
	ldr	x20, [x29, 32]
	ldr	x21, [x29, 24]
	ldr	x22, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 48
	ret
/* end function board */

.text
.balign 4
.globl _chk
_chk:
	stp	x29, x30, [sp, -16]!
	mov	x29, sp
	mov	w2, w0
	cmp	w2, #0
	blt	L17
	cmp	w2, #7
	bgt	L17
	cmp	w1, #0
	blt	L17
	cmp	w1, #7
	bgt	L17
	adrp	x0, _b@page
	add	x0, x0, _b@pageoff
	ldr	x0, [x0]
	sxtw	x3, w2
	mov	x2, #8
	mul	x2, x2, x3
	add	x0, x0, x2
	ldr	x0, [x0]
	sxtw	x2, w1
	mov	x1, #4
	mul	x1, x1, x2
	add	x0, x0, x1
	ldr	w0, [x0]
	cmp	w0, #0
	cset	w0, eq
	b	L18
L17:
	mov	w0, #0
L18:
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
	mov	w25, w2
	mov	w26, w1
	mov	x1, #16
	sub	sp, sp, x1
	mov	x22, sp
	mov	x1, #16
	sub	sp, sp, x1
	mov	x21, sp
	adrp	x1, _b@page
	add	x1, x1, _b@pageoff
	ldr	x1, [x1]
	sxtw	x3, w26
	mov	x2, #8
	mul	x2, x2, x3
	add	x1, x1, x2
	ldr	x1, [x1]
	sxtw	x3, w25
	mov	x2, #4
	mul	x2, x2, x3
	add	x1, x1, x2
	str	w0, [x1]
	cmp	w0, #64
	beq	L35
	mov	w1, #-2
	str	w1, [x22]
	mov	w19, #-2
L24:
	cmp	w19, #2
	bgt	L41
	mov	w1, #-2
	str	w1, [x21]
	mov	w20, #-2
L27:
	cmp	w20, #2
	bgt	L34
	mov	w23, w0
	mov	w0, w19
	bl	_abs
	mov	w18, w0
	mov	w0, w23
	mov	w23, w18
	mov	w24, w0
	mov	w0, w20
	bl	_abs
	mov	w1, w26
	mov	w2, w0
	mov	w0, w24
	add	w2, w23, w2
	cmp	w2, #3
	beq	L30
	mov	w2, w25
	b	L32
L30:
	mov	w23, w0
	add	w0, w1, w19
	mov	w24, w1
	add	w1, w25, w20
	bl	_chk
	mov	w2, w25
	mov	w1, w24
	mov	w3, w0
	mov	w0, w23
	cmp	w3, #0
	beq	L32
	mov	w23, w0
	mov	w0, #1
	add	w0, w23, w0
	mov	w24, w1
	add	w1, w1, w19
	mov	w25, w2
	add	w2, w2, w20
	bl	_go
	mov	w2, w25
	mov	w1, w24
	mov	w0, w23
L32:
	mov	w3, #1
	add	w20, w20, w3
	str	w20, [x21]
	mov	w25, w2
	mov	w26, w1
	b	L27
L34:
	mov	w1, #1
	add	w19, w19, w1
	str	w19, [x22]
	b	L24
L35:
	cmp	w26, #2
	beq	L41
	cmp	w25, #0
	beq	L41
	mov	w0, #2
	sub	w0, w26, w0
	bl	_abs
	mov	w19, w0
	mov	w0, w25
	bl	_abs
	add	w0, w19, w0
	cmp	w0, #3
	bne	L41
	bl	_board
	adrp	x0, _N@page
	add	x0, x0, _N@pageoff
	ldr	w0, [x0]
	mov	w1, #1
	add	w0, w0, w1
	adrp	x1, _N@page
	add	x1, x1, _N@pageoff
	str	w0, [x1]
	cmp	w0, #10
	bne	L41
	mov	w0, #0
	bl	_exit
L41:
	adrp	x0, _b@page
	add	x0, x0, _b@pageoff
	ldr	x0, [x0]
	sxtw	x2, w26
	mov	x1, #8
	mul	x1, x1, x2
	add	x0, x0, x1
	ldr	x0, [x0]
	sxtw	x2, w25
	mov	x1, #4
	mul	x1, x1, x2
	add	x1, x0, x1
	mov	w0, #0
	str	w0, [x1]
	mov	w0, #0
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
.globl _main
_main:
	stp	x29, x30, [sp, -32]!
	mov	x29, sp
	str	x19, [x29, 24]
	str	x20, [x29, 16]
	mov	x0, #16
	sub	sp, sp, x0
	mov	x20, sp
	mov	w0, #8
	bl	_malloc
	adrp	x1, _t@page
	add	x1, x1, _t@pageoff
	str	x0, [x1]
	bl	_time
	adrp	x0, _t@page
	add	x0, x0, _t@pageoff
	ldr	x0, [x0]
	bl	_ctime
	mov	x1, #16
	sub	sp, sp, x1
	mov	x1, #0
	add	x1, sp, x1
	str	x0, [x1]
	adrp	x0, _.s5@page
	add	x0, x0, _.s5@pageoff
	bl	_printf
	mov	x0, #16
	add	sp, sp, x0
	mov	w1, #8
	mov	w0, #8
	bl	_calloc
	adrp	x1, _b@page
	add	x1, x1, _b@pageoff
	str	x0, [x1]
	mov	w0, #0
	str	w0, [x20]
	mov	w19, #0
L46:
	cmp	w19, #8
	bge	L48
	mov	w1, #8
	mov	w0, #8
	bl	_calloc
	adrp	x1, _b@page
	add	x1, x1, _b@pageoff
	ldr	x1, [x1]
	sxtw	x3, w19
	mov	x2, #8
	mul	x2, x2, x3
	add	x1, x1, x2
	str	x0, [x1]
	mov	w0, #1
	add	w19, w19, w0
	str	w19, [x20]
	b	L46
L48:
	mov	w2, #0
	mov	w1, #2
	mov	w0, #1
	bl	_go
	ldr	x19, [x29, 24]
	ldr	x20, [x29, 16]
	mov sp, x29
	ldp	x29, x30, [sp], 32
	ret
/* end function main */

.data
.balign 8
_N:
	.int 0
/* end data */

.data
.balign 8
_b:
	.quad 0
/* end data */

.data
.balign 8
_t:
	.quad 0
/* end data */

.data
.balign 8
_.s1:
	.ascii "t: %s\n"
	.byte 0
/* end data */

.data
.balign 8
_.s2:
	.ascii " %02d"
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
	.ascii "t: %s\n"
	.byte 0
/* end data */

