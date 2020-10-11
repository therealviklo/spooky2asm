	global main
	extern ExitProcess

section .text
main:
	push rbp
	mov rbp, rsp
	sub rsp, 32
	mov rax, 2
	mov qword [rbp - 16], rax
	mov rax, 2
	mov rcx, rax
	mov rax, qword [rbp - 16]
	add rax, rcx
	mov qword [rbp - 24], rax
	xor rax, rax
	mov qword [rbp - 32], rax
	mov rax, 4
	mov rcx, rax
	mov rax, qword [rbp - 32]
	sub rax, rcx
	mov rcx, rax
	mov rax, qword [rbp - 24]
	imul rax, rcx
	mov qword [rbp - 8], rax
.ret:
	mov rsp, rbp
	pop rbp
	; xor rcx, rcx
	mov rcx, rax
	call ExitProcess

