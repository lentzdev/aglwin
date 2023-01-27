COMMENT @

----------------------------------------------------------------------------

TASM 1.0, Standard C calling, Large model

Author: David Burki
Adapted: Jac Kersing

See Tech Specialist Volume 1, No. 3 for the original article
----------------------------------------------------------------------------

END COMMENT @

.model LARGE

; --------------------------
; MACROS and EQUates
; --------------------------

sim_int macro num
	pushf
	call	cs:orig_&num
endm

FALSE	equ	0
TRUE	equ	1

public _key_capture
public _key_release
public _key_getbrk


; --------------------------
; The Code
; --------------------------
.code
orig_16h	dd	0
orig_1bh	dd	0
replaced	db	FALSE
brk_flag	dw	0

assume ds:@curseg

install_vectors proc	near
	mov	ax,3516h
	int	21h
	mov	word ptr orig_16h, bx
	mov	word ptr orig_16h+2, es

	lea	dx, sixteen_handler
	mov	ax,2516h
	int	21h


	mov	ax,351bh
	int	21h
	mov	word ptr orig_1bh, bx
	mov	word ptr orig_1bh+2, es

	lea	dx, int1b_handler
	mov	ax,251bh
	int	21h

	ret

install_vectors endp


_key_capture  proc

	push	bp
	mov	bp,sp

	push	ds
	push	es

	push	cs
	pop	ds

	cmp	replaced,TRUE
	jz	capture_exit

	call	near ptr install_vectors
	mov	byte ptr replaced, TRUE

capture_exit:
	pop	es
	pop	ds
	pop	bp
	ret
_key_capture endp


_key_release proc

	push	ds
	push	dx

	cmp	cs:replaced,TRUE
	jnz	release_exit

	lds	dx,cs:orig_16h
	mov	ax,2516h
	int	21h

	lds	dx,cs:orig_1bh
	mov	ax,251bh
	int	21h

	mov	cs:replaced, FALSE

release_exit:
	pop	dx
	pop	ds
	ret

_key_release endp


_key_getbrk proc

	mov	ax, cs:brk_flag
	mov	cs:brk_flag, 0
	ret

_key_getbrk endp


assume	cs:@curseg, ds:nothing, es:nothing

save_funct	db	?

sixteen_handler proc	far

	mov	cs:save_funct, ah
	and	ah,11101111b
	cmp	ah,2
	jb	not_shift_status_req

	mov	ah,cs:save_funct
	jmp	cs:orig_16h

not_shift_status_req:
	cmp	ah,1
	jz	keyready_call

get_key_call:
	mov	ah,cs:save_funct
	sim_int 16h

	cmp	ax,2e03h
	jnz	iret_back

	inc	cs:brk_flag
	jmp	get_key_call

keyready_call:
	mov	ah,cs:save_funct
	sim_int 16h
	jz	ok_to_go_back

	cmp	ax,2e03h
	jnz	ok_to_go_back

	inc	cs:brk_flag
	mov	ah,0
	sim_int 16h
	jmp	keyready_call

ok_to_go_back:
	ret	2

iret_back:
	iret

sixteen_handler endp


int1b_handler proc far
	iret
int1b_handler endp


end
