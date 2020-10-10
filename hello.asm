	global main
	extern ExitProcess

section .text
main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov rax, 2
	mov qword [rbp - 8], rax
	mov rax, 2
	mov rcx, rax
	mov rax, qword [rbp - 8]
	add rax, rcx
	mov qword [rbp - 16], rax
	mov rax, 4
	mov rcx, rax
	mov rax, qword [rbp - 16]
	imul rax, rcx
.ret:
	mov rsp, rbp
	pop rbp
	;xor rcx, rcx
	mov rcx, rax
	call ExitProcess

