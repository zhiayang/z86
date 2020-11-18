[bits 16]
[org 0xFFFF0000]

begin:
; 	mov bx, 0
; 	mov ax, 1
; 	mov cx, 20

; .l:
; 	mov dx, bx
; 	mov bx, ax
; 	add ax, dx

; 	dec cx
; 	jnz .l

	; jmp end

end:
	; setup some fake code to halt
	; jmp 0x7C0:0x0
	jmp far [cs:owo]
	add ax, 1
	hlt

owo:
	dw 0xC00
	dw 0x700

times 0xFFF0-($-$$) db 0
jmp begin
