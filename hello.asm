	global main
	extern ExitProcess
	extern putchar

section .text
_print:
	push rbp
	mov rbp, rsp
	xor rdx, rdx
	mov rax, rsp
	mov rcx, 16
	div rcx
	sub rsp, rdx
	sub rsp, 32
	mov rcx, qword [rbp + 16]
	call putchar
	mov rsp, rbp
	pop rbp
	ret

_digitToChar:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov rax, 48
	mov qword [rbp - 8], rax
	mov rax, qword [rbp + 16]
	mov qword [rbp - 16], rax
	mov rax, qword [rbp + 24]
	mov rcx, rax
	mov rax, qword [rbp - 16]
	add rax, rcx
	mov rcx, rax
	mov rax, qword [rbp - 8]
	add rax, rcx
.ret:
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	sub rsp, 8
	sub rsp, 16
	mov rax, 3
	mov qword [rsp + 0], rax
	mov rax, 1
	mov qword [rsp + 8], rax
	call _digitToChar
	add rsp, 16
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

