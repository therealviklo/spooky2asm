	global main
	extern ExitProcess
	extern putchar

section .text
_print:
	push rbp
	mov rbp, rsp
	sub rsp, 32
	mov rcx, qword [rbp + 16]
	call putchar
	mov rsp, rbp
	pop rbp
	ret

_printDigit:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	sub rsp, 8
	mov rax, 48
	mov qword [rbp - 8], rax
	mov rax, qword [rbp + 16]
	mov rcx, rax
	mov rax, qword [rbp - 8]
	add rax, rcx
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
.ret:
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	sub rsp, 8
	mov rax, 8
	mov qword [rsp + 0], rax
	call _printDigit
	add rsp, 8
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

