	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.globl	_board                          ; -- Begin function board
	.p2align	2
_board:                                 ; @board
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	.cfi_def_cfa_offset 64
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
Lloh0:
	adrp	x19, _t@GOTPAGE
Lloh1:
	ldr	x19, [x19, _t@GOTPAGEOFF]
	ldr	x0, [x19]
	bl	_time
	ldr	x0, [x19]
	bl	_ctime
	str	x0, [sp]
Lloh2:
	adrp	x0, l_.str@PAGE
Lloh3:
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	mov	x20, #0
Lloh4:
	adrp	x21, _b@GOTPAGE
Lloh5:
	ldr	x21, [x21, _b@GOTPAGEOFF]
Lloh6:
	adrp	x19, l_.str.1@PAGE
Lloh7:
	add	x19, x19, l_.str.1@PAGEOFF
LBB0_1:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x21]
	ldr	x8, [x8]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #8]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #16]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #24]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #32]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #40]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #48]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	ldr	x8, [x21]
	ldr	x8, [x8, #56]
	ldr	w8, [x8, x20]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	mov	w0, #10
	bl	_putchar
	add	x20, x20, #4
	cmp	x20, #32
	b.ne	LBB0_1
; %bb.2:
	mov	w0, #10
	bl	_putchar
	mov	w0, #0
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.loh AdrpAdd	Lloh6, Lloh7
	.loh AdrpLdrGot	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpLdrGot	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_chk                            ; -- Begin function chk
	.p2align	2
_chk:                                   ; @chk
	.cfi_startproc
; %bb.0:
	orr	w8, w1, w0
	cmp	w8, #7
	b.hi	LBB1_2
; %bb.1:
Lloh8:
	adrp	x8, _b@GOTPAGE
Lloh9:
	ldr	x8, [x8, _b@GOTPAGEOFF]
Lloh10:
	ldr	x8, [x8]
	ldr	x8, [x8, w0, uxtw #3]
	ldr	w8, [x8, w1, uxtw #2]
	cmp	w8, #0
	cset	w0, eq
	ret
LBB1_2:
	mov	w0, #0
	ret
	.loh AdrpLdrGotLdr	Lloh8, Lloh9, Lloh10
	.cfi_endproc
                                        ; -- End function
	.globl	_go                             ; -- Begin function go
	.p2align	2
_go:                                    ; @go
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #112
	.cfi_def_cfa_offset 112
	stp	x28, x27, [sp, #16]             ; 16-byte Folded Spill
	stp	x26, x25, [sp, #32]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #48]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #64]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #80]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	mov	x19, x2
	mov	x20, x1
Lloh11:
	adrp	x25, _b@GOTPAGE
Lloh12:
	ldr	x25, [x25, _b@GOTPAGEOFF]
	ldr	x8, [x25]
	ldr	x8, [x8, w1, sxtw #3]
	str	w0, [x8, w2, sxtw #2]
	cmp	w0, #64
	b.ne	LBB2_6
; %bb.1:
	cmp	w20, #2
	b.eq	LBB2_5
; %bb.2:
	cbz	w19, LBB2_5
; %bb.3:
	mov	w8, #2
	sub	w8, w8, w20
	subs	w9, w20, #2
	csel	w8, w8, w9, lt
	cmp	w19, #0
	cneg	w9, w19, mi
	add	w8, w8, w9
	cmp	w8, #3
	b.ne	LBB2_5
; %bb.4:
	bl	_board
	adrp	x8, _N@PAGE
	ldr	w9, [x8, _N@PAGEOFF]
	add	w9, w9, #1
	str	w9, [x8, _N@PAGEOFF]
	cmp	w9, #10
	b.eq	LBB2_24
LBB2_5:
	sxtw	x8, w20
	sxtw	x9, w19
	ldr	x10, [x25]
	ldr	x8, [x10, x8, lsl #3]
	str	wzr, [x8, x9, lsl #2]
	mov	w0, #0
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #32]             ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #112
	ret
LBB2_6:
	add	w21, w0, #1
	sub	w22, w19, #1
	add	w26, w19, #1
	sub	w23, w19, #2
	add	w27, w19, #2
	mov	w28, #-2
	mov	w8, w19
	str	x8, [sp, #8]                    ; 8-byte Folded Spill
	b	LBB2_9
LBB2_7:                                 ;   in Loop: Header=BB2_9 Depth=1
	mov	x0, x21
	mov	x1, x24
	bl	_go
LBB2_8:                                 ;   in Loop: Header=BB2_9 Depth=1
	add	w28, w28, #1
	cmp	w28, #3
	b.eq	LBB2_5
LBB2_9:                                 ; =>This Inner Loop Header: Depth=1
	cmp	w28, #0
	cneg	w8, w28, mi
	add	w24, w20, w28
	cmp	w8, #3
	b.eq	LBB2_17
; %bb.10:                               ;   in Loop: Header=BB2_9 Depth=1
	cmp	w8, #2
	b.eq	LBB2_19
; %bb.11:                               ;   in Loop: Header=BB2_9 Depth=1
	cmp	w8, #1
	b.ne	LBB2_8
; %bb.12:                               ;   in Loop: Header=BB2_9 Depth=1
	orr	w8, w23, w24
	cmp	w8, #7
	b.hi	LBB2_15
; %bb.13:                               ;   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x25]
	ldr	x8, [x8, x24, lsl #3]
	ldr	w8, [x8, x23, lsl #2]
	cbnz	w8, LBB2_15
; %bb.14:                               ;   in Loop: Header=BB2_9 Depth=1
	mov	x0, x21
	mov	x1, x24
	mov	x2, x23
	bl	_go
LBB2_15:                                ;   in Loop: Header=BB2_9 Depth=1
	orr	w8, w27, w24
	cmp	w8, #7
	b.hi	LBB2_8
; %bb.16:                               ;   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x25]
	ldr	x8, [x8, x24, lsl #3]
	ldr	w8, [x8, x27, lsl #2]
	mov	x2, x27
	cbnz	w8, LBB2_8
	b	LBB2_7
LBB2_17:                                ;   in Loop: Header=BB2_9 Depth=1
	orr	w8, w24, w19
	cmp	w8, #7
	b.hi	LBB2_8
; %bb.18:                               ;   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x25]
	ldr	x8, [x8, x24, lsl #3]
	ldr	x9, [sp, #8]                    ; 8-byte Folded Reload
	ldr	w8, [x8, x9, lsl #2]
	mov	x2, x19
	cbnz	w8, LBB2_8
	b	LBB2_7
LBB2_19:                                ;   in Loop: Header=BB2_9 Depth=1
	orr	w8, w22, w24
	cmp	w8, #7
	b.hi	LBB2_22
; %bb.20:                               ;   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x25]
	ldr	x8, [x8, x24, lsl #3]
	ldr	w8, [x8, x22, lsl #2]
	cbnz	w8, LBB2_22
; %bb.21:                               ;   in Loop: Header=BB2_9 Depth=1
	mov	x0, x21
	mov	x1, x24
	mov	x2, x22
	bl	_go
LBB2_22:                                ;   in Loop: Header=BB2_9 Depth=1
	orr	w8, w26, w24
	cmp	w8, #7
	b.hi	LBB2_8
; %bb.23:                               ;   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x25]
	ldr	x8, [x8, x24, lsl #3]
	ldr	w8, [x8, x26, lsl #2]
	mov	x2, x26
	cbnz	w8, LBB2_8
	b	LBB2_7
LBB2_24:
	mov	w0, #0
	bl	_exit
	.loh AdrpLdrGot	Lloh11, Lloh12
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	w0, #8
	bl	_malloc
Lloh13:
	adrp	x19, _t@GOTPAGE
Lloh14:
	ldr	x19, [x19, _t@GOTPAGEOFF]
	str	x0, [x19]
	bl	_time
	ldr	x0, [x19]
	bl	_ctime
	str	x0, [sp]
Lloh15:
	adrp	x0, l_.str@PAGE
Lloh16:
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	mov	w0, #8
	mov	w1, #8
	bl	_calloc
	mov	x19, x0
Lloh17:
	adrp	x8, _b@GOTPAGE
Lloh18:
	ldr	x8, [x8, _b@GOTPAGEOFF]
Lloh19:
	str	x0, [x8]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #8]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #16]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #24]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #32]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #40]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #48]
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	str	x0, [x19, #56]
	mov	w0, #1
	mov	w1, #2
	mov	w2, #0
	bl	_go
	mov	w0, #0
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.loh AdrpLdrGotStr	Lloh17, Lloh18, Lloh19
	.loh AdrpAdd	Lloh15, Lloh16
	.loh AdrpLdrGot	Lloh13, Lloh14
	.cfi_endproc
                                        ; -- End function
	.globl	_N                              ; @N
.zerofill __DATA,__common,_N,4,2
	.comm	_t,8,3                          ; @t
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"t: %s\n"

l_.str.1:                               ; @.str.1
	.asciz	" %02d"

	.comm	_b,8,3                          ; @b
.subsections_via_symbols
