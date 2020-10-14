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

main:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	mov rax, 1
	cmp rax, 0
	je .iff0
	sub rsp, 8
	mov rax, 1
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	jmp .ife0
.iff0:
	mov rax, 0
	cmp rax, 0
	je .iff1
	sub rsp, 8
	mov rax, 2
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	jmp .ife1
.iff1:
	mov rax, 0
	cmp rax, 0
	je .iff2
	jmp .ife2
.iff2:
	sub rsp, 8
	mov rax, 3
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
.ife2:
.ife1:
.ife0:
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

