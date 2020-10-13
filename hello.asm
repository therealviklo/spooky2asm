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

_printInt:
	push rbp
	mov rbp, rsp
	xor rdx, rdx
	mov rax, rsp
	mov r8, 16
	div r8
	sub rsp, rdx
	mov rax, qword [rbp + 16]
	cmp rax, 0
	je .zero
	jmp .nozero
.zero:
	mov rcx, 48
	sub rsp, 40
	call putchar
	jmp .ret1
.nozero:
	jl .neg
	jmp .noneg
.neg:
	mov rcx, 45
	push rax
	sub rsp, 32
	call putchar
	add rsp, 32
	pop rax
	neg rax
.noneg:
	sub rsp, 8
	call .loop
.ret1:
	mov rsp, rbp
	pop rbp
	ret
.loop:
	push rbp
	mov rbp, rsp
	cmp rax, 0
	je .ret2
	xor rdx, rdx
	mov rcx, 10
	div rcx
	push rdx
	call .loop
	pop rdx
	mov rcx, rdx
	add rcx, 48
	sub rsp, 40
	call putchar
.ret2:
	mov rsp, rbp
	pop rbp
	ret

_test:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	sub rsp, 8
	xor rax, rax
	mov qword [rbp - 8], rax
	mov rax, 420
	mov rcx, rax
	mov rax, qword [rbp - 8]
	sub rax, rcx
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	jmp .ret
	sub rsp, 8
	mov rax, 420
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
.ret:
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	sub rsp, 0
	call _test
	add rsp, 0
	sub rsp, 8
	mov rax, 69
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	sub rsp, 8
	mov rax, 1
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	sub rsp, 8
	mov rax, 0
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	sub rsp, 8
	xor rax, rax
	mov qword [rbp - 8], rax
	mov rax, 1
	mov rcx, rax
	mov rax, qword [rbp - 8]
	sub rax, rcx
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	sub rsp, 8
	xor rax, rax
	mov qword [rbp - 16], rax
	mov rax, 69
	mov rcx, rax
	mov rax, qword [rbp - 16]
	sub rax, rcx
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

