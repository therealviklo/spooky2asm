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
	; Det här första är bara setup som t.ex. alginar stacken.
	; Det är .loop som faktiskt skriver ut saker.

	; Fixa med rbp
	push rbp
	mov rbp, rsp

	; Aligna stacken
	xor rdx, rdx
	mov rax, rsp
	mov r8, 16
	div r8
	sub rsp, rdx

	; Flytta in parametern i rax
	mov rax, qword [rbp + 16]

	; Funktionen fungerar inte för 0 så det finns en manuell check för 0
	cmp rax, 0
	je .zero
	jmp .nozero
.zero:
	; rcx = '0'
	mov rcx, 48
	; Shadow bytes och align
	sub rsp, 40
	; Skriv ut rcx
	call putchar

	; Returnera
	jmp .ret1
.nozero:
	; Behöver ett specialfall för negativa tal
	jl .neg
	jmp .noneg
.neg:
	; rcx = '-'
	mov rcx, 45
	; Spara rax
	push rax
	; Shadow bytes
	sub rsp, 32
	; Skriv ut rcx
	call putchar
	; Återställ
	add rsp, 32
	pop rax
	; Negera rax
	neg rax
.noneg:
	; Se till att stacken är alignad
	sub rsp, 8
	; Kör huvuddelen av funktionen
	call .loop
.ret1:
	; Använd rbp för att återställa rsp och därmed rensa upp stacken
	mov rsp, rbp
	; Och återställ rbp och returnera
	pop rbp
	ret
.loop:
	; Fixa med rbp
	push rbp
	mov rbp, rsp

	; Om rax är 0 ska den skippa allt och bara returnera
	cmp rax, 0
	je .ret2
	
	; Dividera rax med 10 (kvoten läggs i rax och resten läggs i rdx)
	xor rdx, rdx
	mov rcx, 10
	div rcx
	; rdx (= resten) behövs senare men kommer att skrivas över när .loop
	; körs igen så den sparas här
	push rdx

	; Rekursion. Behöver inte subtrahera 8 från rsp då rdx pushades, 
	; vilket alignar stacken.
	call .loop

	; Återställ rdx (= resten)
	pop rdx

	; rcx = '0' + rdx
	mov rcx, rdx
	add rcx, 48
	; Shadow bytes och align
	sub rsp, 40
	; Skriv ut rcx
	call putchar
.ret2:
	; Återställ rsp och returnera
	mov rsp, rbp
	pop rbp
	ret
	
_random:
	push rbp
	mov rbp, rsp
	xor rdx, rdx
	mov rax, rsp
	mov rcx, 16
	div rcx
	sub rsp, rdx
	xor rax, rax
	sub rsp, 40
	call rand
	add rsp, 40
	and rax, 0xFF
	sub rsp, 32
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	add rsp, 32
	mov rsp, rbp
	pop rbp
	ret

main:
	sub rsp, 8
	mov qword [rsp], -1
	call _printInt
	add rsp, 8

	xor rcx, rcx
	call ExitProcess