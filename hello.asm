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

_isPrime:
	push rbp
	mov rbp, rsp
	sub rsp, 56
	mov rax, 1
	mov qword [rbp - 8], rax
	mov rax, 2
	mov qword [rbp - 16], rax
.fls0:
	mov rax, qword [rbp - 16]
	mov qword [rbp - 24], rax
	mov rax, qword [rbp - 16]
	mov rcx, rax
	mov rax, qword [rbp - 24]
	imul rax, rcx
	mov qword [rbp - 32], rax
	mov rax, qword [rbp + 16]
	mov rcx, qword [rbp - 32]
	sub rcx, rax
	xor rax, rax
	cmp rcx, 0
	setle al
	cmp rax, 0
	je .fle0
	mov rax, qword [rbp + 16]
	mov qword [rbp - 48], rax
	mov rax, qword [rbp - 16]
	mov rcx, rax
	mov rax, qword [rbp - 48]
	xor rdx, rdx
	idiv rcx
	mov rax, rdx
	mov qword [rbp - 56], rax
	mov rax, 0
	mov rcx, rax
	mov rax, qword [rbp - 56]
	sub rcx, rax
	xor rax, rax
	cmp rcx, 0
	sete al
	cmp rax, 0
	je .iff0
	xor rax, rax
	jmp .ret
.iff0:
	mov rax, qword [rbp - 16]
	mov qword [rbp - 40], rax
	mov rax, 1
	mov rcx, rax
	mov rax, qword [rbp - 40]
	add rax, rcx
	mov qword [rbp - 16], rax
	jmp .fls0
.fle0:
	mov rax, 1
	jmp .ret
.ret:
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 24
	call spookyInitGlobals
	mov rax, 0
	mov qword [rbp - 8], rax
.fls1:
	mov rax, qword [rbp - 8]
	mov qword [rbp - 16], rax
	mov rax, 100
	mov rcx, qword [rbp - 16]
	sub rcx, rax
	xor rax, rax
	cmp rcx, 0
	setl al
	cmp rax, 0
	je .fle1
	sub rsp, 8
	mov rax, qword [rbp - 8]
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 32
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	sub rsp, 8
	sub rsp, 8
	mov rax, qword [rbp - 8]
	mov qword [rsp + 0], rax
	call _isPrime
	add rsp, 8
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	mov rax, qword [rbp - 8]
	mov qword [rbp - 24], rax
	mov rax, 1
	mov rcx, rax
	mov rax, qword [rbp - 24]
	add rax, rcx
	mov qword [rbp - 8], rax
	jmp .fls1
.fle1:
.ret:
	mov rsp, rbp
	pop rbp
	xor rcx, rcx
	call ExitProcess

spookyInitGlobals:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	mov rsp, rbp
	pop rbp
	ret
section .bss
