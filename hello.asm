	global main
	extern ExitProcess

section .text
_test:
	push rbp
	mov rbp, rsp
	sub rsp, 0
.ret:
	mov rsp, rbp
	pop rbp
	ret

_test2:
	push rbp
	mov rbp, rsp
	sub rsp, 0
.ret:
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 0
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

