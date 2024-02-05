	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.globl	_board                          ; -- Begin function board
	.p2align	2
_board:                                 ; @board
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	adrp	x8, _t@GOTPAGE
	ldr	x8, [x8, _t@GOTPAGEOFF]
	str	x8, [sp, #16]                   ; 8-byte Folded Spill
	ldr	x0, [x8]
	bl	_time
	ldr	x8, [sp, #16]                   ; 8-byte Folded Reload
	ldr	x0, [x8]
	bl	_ctime
	mov	x8, sp
	str	x0, [x8]
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	stur	wzr, [x29, #-8]
	b	LBB0_1
LBB0_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_3 Depth 2
	ldur	w8, [x29, #-8]
	subs	w8, w8, #8
	cset	w8, ge
	tbnz	w8, #0, LBB0_8
	b	LBB0_2
LBB0_2:                                 ;   in Loop: Header=BB0_1 Depth=1
	stur	wzr, [x29, #-4]
	b	LBB0_3
LBB0_3:                                 ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldur	w8, [x29, #-4]
	subs	w8, w8, #8
	cset	w8, ge
	tbnz	w8, #0, LBB0_6
	b	LBB0_4
LBB0_4:                                 ;   in Loop: Header=BB0_3 Depth=2
	adrp	x8, _b@GOTPAGE
	ldr	x8, [x8, _b@GOTPAGEOFF]
	ldr	x8, [x8]
	ldursw	x9, [x29, #-4]
	ldr	x8, [x8, x9, lsl #3]
	ldursw	x9, [x29, #-8]
	ldr	w9, [x8, x9, lsl #2]
                                        ; implicit-def: $x8
	mov	x8, x9
	mov	x9, sp
	str	x8, [x9]
	adrp	x0, l_.str.1@PAGE
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_printf
	b	LBB0_5
LBB0_5:                                 ;   in Loop: Header=BB0_3 Depth=2
	ldur	w8, [x29, #-4]
	add	w8, w8, #1
	stur	w8, [x29, #-4]
	b	LBB0_3
LBB0_6:                                 ;   in Loop: Header=BB0_1 Depth=1
	adrp	x0, l_.str.2@PAGE
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_printf
	b	LBB0_7
LBB0_7:                                 ;   in Loop: Header=BB0_1 Depth=1
	ldur	w8, [x29, #-8]
	add	w8, w8, #1
	stur	w8, [x29, #-8]
	b	LBB0_1
LBB0_8:
	adrp	x0, l_.str.2@PAGE
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_printf
	mov	w0, #0
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_chk                            ; -- Begin function chk
	.p2align	2
_chk:                                   ; @chk
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	w0, [sp, #8]
	str	w1, [sp, #4]
	ldr	w8, [sp, #8]
	subs	w8, w8, #0
	cset	w8, lt
	tbnz	w8, #0, LBB1_4
	b	LBB1_1
LBB1_1:
	ldr	w8, [sp, #8]
	subs	w8, w8, #7
	cset	w8, gt
	tbnz	w8, #0, LBB1_4
	b	LBB1_2
LBB1_2:
	ldr	w8, [sp, #4]
	subs	w8, w8, #0
	cset	w8, lt
	tbnz	w8, #0, LBB1_4
	b	LBB1_3
LBB1_3:
	ldr	w8, [sp, #4]
	subs	w8, w8, #7
	cset	w8, le
	tbnz	w8, #0, LBB1_5
	b	LBB1_4
LBB1_4:
	str	wzr, [sp, #12]
	b	LBB1_6
LBB1_5:
	adrp	x8, _b@GOTPAGE
	ldr	x8, [x8, _b@GOTPAGEOFF]
	ldr	x8, [x8]
	ldrsw	x9, [sp, #8]
	ldr	x8, [x8, x9, lsl #3]
	ldrsw	x9, [sp, #4]
	ldr	w8, [x8, x9, lsl #2]
	subs	w8, w8, #0
	cset	w8, eq
	and	w8, w8, #0x1
	str	w8, [sp, #12]
	b	LBB1_6
LBB1_6:
	ldr	w0, [sp, #12]
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_go                             ; -- Begin function go
	.p2align	2
_go:                                    ; @go
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	w0, [x29, #-4]
	stur	w1, [x29, #-8]
	stur	w2, [x29, #-12]
	ldur	w8, [x29, #-4]
	adrp	x9, _b@GOTPAGE
	ldr	x9, [x9, _b@GOTPAGEOFF]
	ldr	x9, [x9]
	ldursw	x10, [x29, #-8]
	ldr	x9, [x9, x10, lsl #3]
	ldursw	x10, [x29, #-12]
	str	w8, [x9, x10, lsl #2]
	ldur	w8, [x29, #-4]
	subs	w8, w8, #64
	cset	w8, ne
	tbnz	w8, #0, LBB2_8
	b	LBB2_1
LBB2_1:
	ldur	w8, [x29, #-8]
	subs	w8, w8, #2
	cset	w8, eq
	tbnz	w8, #0, LBB2_7
	b	LBB2_2
LBB2_2:
	ldur	w8, [x29, #-12]
	subs	w8, w8, #0
	cset	w8, eq
	tbnz	w8, #0, LBB2_7
	b	LBB2_3
LBB2_3:
	ldur	w8, [x29, #-8]
	subs	w0, w8, #2
	bl	_abs
	str	w0, [sp, #8]                    ; 4-byte Folded Spill
	ldur	w0, [x29, #-12]
	bl	_abs
	mov	x8, x0
	ldr	w0, [sp, #8]                    ; 4-byte Folded Reload
	add	w8, w0, w8
	subs	w8, w8, #3
	cset	w8, ne
	tbnz	w8, #0, LBB2_7
	b	LBB2_4
LBB2_4:
	bl	_board
	adrp	x8, _N@PAGE
	ldr	w9, [x8, _N@PAGEOFF]
	add	w9, w9, #1
	str	w9, [x8, _N@PAGEOFF]
	ldr	w8, [x8, _N@PAGEOFF]
	subs	w8, w8, #10
	cset	w8, ne
	tbnz	w8, #0, LBB2_6
	b	LBB2_5
LBB2_5:
	mov	w0, #0
	bl	_exit
LBB2_6:
	b	LBB2_7
LBB2_7:
	b	LBB2_20
LBB2_8:
	mov	w8, #-2
	str	w8, [sp, #16]
	b	LBB2_9
LBB2_9:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB2_11 Depth 2
	ldr	w8, [sp, #16]
	subs	w8, w8, #2
	cset	w8, gt
	tbnz	w8, #0, LBB2_19
	b	LBB2_10
LBB2_10:                                ;   in Loop: Header=BB2_9 Depth=1
	mov	w8, #-2
	str	w8, [sp, #12]
	b	LBB2_11
LBB2_11:                                ;   Parent Loop BB2_9 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	w8, [sp, #12]
	subs	w8, w8, #2
	cset	w8, gt
	tbnz	w8, #0, LBB2_17
	b	LBB2_12
LBB2_12:                                ;   in Loop: Header=BB2_11 Depth=2
	ldr	w0, [sp, #16]
	bl	_abs
	str	w0, [sp, #4]                    ; 4-byte Folded Spill
	ldr	w0, [sp, #12]
	bl	_abs
	mov	x8, x0
	ldr	w0, [sp, #4]                    ; 4-byte Folded Reload
	add	w8, w0, w8
	subs	w8, w8, #3
	cset	w8, ne
	tbnz	w8, #0, LBB2_15
	b	LBB2_13
LBB2_13:                                ;   in Loop: Header=BB2_11 Depth=2
	ldur	w8, [x29, #-8]
	ldr	w9, [sp, #16]
	add	w0, w8, w9
	ldur	w8, [x29, #-12]
	ldr	w9, [sp, #12]
	add	w1, w8, w9
	bl	_chk
	subs	w8, w0, #0
	cset	w8, eq
	tbnz	w8, #0, LBB2_15
	b	LBB2_14
LBB2_14:                                ;   in Loop: Header=BB2_11 Depth=2
	ldur	w8, [x29, #-4]
	add	w0, w8, #1
	ldur	w8, [x29, #-8]
	ldr	w9, [sp, #16]
	add	w1, w8, w9
	ldur	w8, [x29, #-12]
	ldr	w9, [sp, #12]
	add	w2, w8, w9
	bl	_go
	b	LBB2_15
LBB2_15:                                ;   in Loop: Header=BB2_11 Depth=2
	b	LBB2_16
LBB2_16:                                ;   in Loop: Header=BB2_11 Depth=2
	ldr	w8, [sp, #12]
	add	w8, w8, #1
	str	w8, [sp, #12]
	b	LBB2_11
LBB2_17:                                ;   in Loop: Header=BB2_9 Depth=1
	b	LBB2_18
LBB2_18:                                ;   in Loop: Header=BB2_9 Depth=1
	ldr	w8, [sp, #16]
	add	w8, w8, #1
	str	w8, [sp, #16]
	b	LBB2_9
LBB2_19:
	b	LBB2_20
LBB2_20:
	adrp	x8, _b@GOTPAGE
	ldr	x8, [x8, _b@GOTPAGEOFF]
	ldr	x8, [x8]
	ldursw	x9, [x29, #-8]
	ldr	x8, [x8, x9, lsl #3]
	ldursw	x9, [x29, #-12]
	mov	w0, #0
	str	w0, [x8, x9, lsl #2]
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	adrp	x8, _t@GOTPAGE
	ldr	x8, [x8, _t@GOTPAGEOFF]
	str	x8, [sp, #8]                    ; 8-byte Folded Spill
	stur	wzr, [x29, #-4]
	mov	x0, #8
	str	x0, [sp, #16]                   ; 8-byte Folded Spill
	bl	_malloc
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	str	x0, [x8]
	ldr	x0, [x8]
	bl	_time
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x0, [x8]
	bl	_ctime
	mov	x8, sp
	str	x0, [x8]
	adrp	x0, l_.str@PAGE
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	ldr	x1, [sp, #16]                   ; 8-byte Folded Reload
	mov	x0, x1
	bl	_calloc
	adrp	x8, _b@GOTPAGE
	ldr	x8, [x8, _b@GOTPAGEOFF]
	str	x0, [x8]
	stur	wzr, [x29, #-8]
	b	LBB3_1
LBB3_1:                                 ; =>This Inner Loop Header: Depth=1
	ldur	w8, [x29, #-8]
	subs	w8, w8, #8
	cset	w8, ge
	tbnz	w8, #0, LBB3_4
	b	LBB3_2
LBB3_2:                                 ;   in Loop: Header=BB3_1 Depth=1
	mov	x0, #8
	mov	x1, #4
	bl	_calloc
	adrp	x8, _b@GOTPAGE
	ldr	x8, [x8, _b@GOTPAGEOFF]
	ldr	x8, [x8]
	ldursw	x9, [x29, #-8]
	str	x0, [x8, x9, lsl #3]
	b	LBB3_3
LBB3_3:                                 ;   in Loop: Header=BB3_1 Depth=1
	ldur	w8, [x29, #-8]
	add	w8, w8, #1
	stur	w8, [x29, #-8]
	b	LBB3_1
LBB3_4:
	mov	w0, #1
	mov	w1, #2
	mov	w2, #0
	bl	_go
	ldur	w0, [x29, #-4]
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
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
l_.str.2:                               ; @.str.2
	.asciz	"\n"

.subsections_via_symbols
