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

main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	call spookyInitGlobals
	mov rax, 10
	mov qword [rbp - 8], rax
.wls0:
	mov rax, qword [rbp - 8]
	cmp rax, 0
	je .wle0
	sub rsp, 8
	mov rax, qword [rbp - 8]
	mov qword [rsp + 0], rax
	call _printInt
	add rsp, 8
	sub rsp, 8
	mov rax, 10
	mov qword [rsp + 0], rax
	call _print
	add rsp, 8
	mov rax, qword [rbp - 8]
	mov qword [rbp - 16], rax
	mov rax, 1
	mov rcx, rax
	mov rax, qword [rbp - 16]
	sub rax, rcx
	mov qword [rbp - 8], rax
	jmp .wls0
.wle0:
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
