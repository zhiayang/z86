[bits 16]
[org 0xFFFF0000]

begin:
	mov bx, 0
	mov ax, 1
	mov cx, 20

.l:

	mov dx, bx
	mov bx, ax
	add ax, dx

	dec cx
	jnz .l

	jmp end

end:
	hlt

times 0xFFF0-($-$$) db 0

xor ax, ax
jmp begin
