	global main
	extern ExitProcess
	extern putchar

section .text
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

_add:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	mov rax, qword [rbp + 16]
	mov qword [rbp - 8], rax
	mov rax, 1
	mov rcx, rax
	mov rax, qword [rbp - 8]
	add rax, rcx
	jmp .ret
.ret:
	mov rsp, rbp
	pop rbp
	ret

_add2:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	sub rsp, 8
	mov rax, qword [rbp + 16]
	mov qword [rsp + 0], rax
	call _add
	add rsp, 8
.ret:
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 24
	xor rax, rax
	mov qword [rbp - 8], rax
	mov rax, 3
	mov rcx, rax
	mov rax, qword [rbp - 8]
	sub rax, rcx
	mov qword [rbp - 16], rax
	xor rax, rax
	mov qword [rbp - 24], rax
	mov rax, 2
	mov rcx, rax
	mov rax, qword [rbp - 24]
	sub rax, rcx
	mov rcx, qword [rbp - 16]
	sub rcx, rax
	xor rax, rax
	cmp rcx, 0
	setl al
	cmp rax, 0
	je .iff0
	sub rsp, 8
	mov rax, 69
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
.iff0:
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

