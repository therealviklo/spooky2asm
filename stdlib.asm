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

main:
	sub rsp, 8
	mov qword [rsp], 49
	call _print
	add rsp, 8

	xor rcx, rcx
	call ExitProcess