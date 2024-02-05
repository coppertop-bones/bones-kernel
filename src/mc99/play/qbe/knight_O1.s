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
LBB0_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_2 Depth 2
	mov	x22, #0
LBB0_2:                                 ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x8, [x21]
	ldr	x8, [x8, x22]
	ldr	w8, [x8, x20, lsl #2]
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	add	x22, x22, #8
	cmp	x22, #64
	b.ne	LBB0_2
; %bb.3:                                ;   in Loop: Header=BB0_1 Depth=1
	mov	w0, #10
	bl	_putchar
	add	x20, x20, #1
	cmp	x20, #8
	b.ne	LBB0_1
; %bb.4:
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
	ldr	x8, [x8, w0, sxtw #3]
	ldr	w8, [x8, w1, sxtw #2]
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
                                        ; kill: def $w2 killed $w2 def $x2
                                        ; kill: def $w1 killed $w1 def $x1
Lloh11:
	adrp	x21, _b@GOTPAGE
Lloh12:
	ldr	x21, [x21, _b@GOTPAGEOFF]
	ldr	x8, [x21]
	sxtw	x22, w1
	ldr	x8, [x8, w1, sxtw #3]
	sxtw	x9, w2
	str	w0, [x8, w2, sxtw #2]
	cmp	w0, #64
	str	x9, [sp, #8]                    ; 8-byte Folded Spill
	b.ne	LBB2_6
; %bb.1:
	cmp	w1, #2
	b.eq	LBB2_5
; %bb.2:
	cbz	w2, LBB2_5
; %bb.3:
	mov	w8, #2
	sub	w8, w8, w1
	subs	w9, w1, #2
	csel	w8, w8, w9, lt
	cmp	w2, #0
	cneg	w9, w2, mi
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
	b.eq	LBB2_15
LBB2_5:
	ldr	x8, [x21]
	ldr	x8, [x8, x22, lsl #3]
	ldr	x9, [sp, #8]                    ; 8-byte Folded Reload
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
	add	w19, w0, #1
	sub	x23, x9, #2
	mov	x25, #-2
	mov	w26, w2
	b	LBB2_8
LBB2_7:                                 ;   in Loop: Header=BB2_8 Depth=1
	add	x25, x25, #1
	cmp	x25, #3
	b.eq	LBB2_5
LBB2_8:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB2_10 Depth 2
	cmp	w25, #0
	cneg	w27, w25, mi
	add	x20, x25, x22
	mov	x28, #-2
	mov	x24, x23
	b	LBB2_10
LBB2_9:                                 ;   in Loop: Header=BB2_10 Depth=2
	add	x28, x28, #1
	add	x24, x24, #1
	cmp	x28, #3
	b.eq	LBB2_7
LBB2_10:                                ;   Parent Loop BB2_8 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	cmp	w28, #0
	cneg	w8, w28, mi
	add	w8, w8, w27
	cmp	w8, #3
	b.ne	LBB2_9
; %bb.11:                               ;   in Loop: Header=BB2_10 Depth=2
	orr	w8, w24, w20
	tst	x8, #0xfffffff8
	b.eq	LBB2_13
; %bb.12:                               ;   in Loop: Header=BB2_10 Depth=2
	mov	w8, #0
	cbnz	w8, LBB2_14
	b	LBB2_9
LBB2_13:                                ;   in Loop: Header=BB2_10 Depth=2
	ldr	x8, [x21]
	ldr	x8, [x8, x20, lsl #3]
	ldr	w8, [x8, x24, lsl #2]
	cmp	w8, #0
	cset	w8, eq
	cbz	w8, LBB2_9
LBB2_14:                                ;   in Loop: Header=BB2_10 Depth=2
	add	w2, w26, w28
	mov	x0, x19
	mov	x1, x20
	bl	_go
	b	LBB2_9
LBB2_15:
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
	mov	x19, #0
Lloh17:
	adrp	x20, _b@GOTPAGE
Lloh18:
	ldr	x20, [x20, _b@GOTPAGEOFF]
	str	x0, [x20]
LBB3_1:                                 ; =>This Inner Loop Header: Depth=1
	mov	w0, #8
	mov	w1, #4
	bl	_calloc
	ldr	x8, [x20]
	str	x0, [x8, x19]
	add	x19, x19, #8
	cmp	x19, #64
	b.ne	LBB3_1
; %bb.2:
	mov	w0, #1
	mov	w1, #2
	mov	w2, #0
	bl	_go
	mov	w0, #0
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.loh AdrpLdrGot	Lloh17, Lloh18
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
