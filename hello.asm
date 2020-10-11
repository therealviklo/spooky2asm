	global main
	extern ExitProcess

section .text
main:
	push rbp
	mov rbp, rsp
	sub rsp, 24
	mov rax, 2
	mov qword [rbp - 16], rax
	mov qword [rbp - 8], rax
	mov rax, qword [rbp - 8]
	mov qword [rbp - 24], rax
	mov rax, qword [rbp - 16]
	mov rcx, rax
	mov rax, qword [rbp - 24]
	add rax, rcx
.ret:
	mov rsp, rbp
	pop rbp
	; xor rcx, rcx
	mov rcx, rax
	call ExitProcess

