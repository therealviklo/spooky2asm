    global main
    extern MessageBoxA
	extern ExitProcess
	extern putchar

section .data
txt: db "test", 0

section .text
printInt:
	; Om parametern (rcx) är 0 så är det klart
	cmp rcx, 0
	jne L1
	ret
L1:
	; Lägg till shadowskiten
	sub rsp, 32
	; Dela rcx med 10
	xor rdx, rdx ; rdx nollas, tar emot resten sen
	mov rax, rcx ; Kopierar in rcx i rax, rax innehåller täljaren och tar emot kvoten
	mov rcx, 10 ; Nämnaren måste vara i ett register (verkar det som), och rcx används ej (är i rax nu)
	div rcx ; Dividera
	mov rcx, rax ; Kopiera resultatet (kvoten) till rcx (första parametern till funktionen)
	push rdx ; Resten, används sen så måste spara
	; Kör sig själv rekursivt
	call printInt
	; Återställ
	pop rdx
	add rsp, 32

	; Fixa med alignment och shadow
	sub rsp, 40

	; Beräkna tecknets värde
	mov rcx, 48 ; Tecken 48 är '0'
	add rcx, rdx ; Lägg till resten från innan
	; Kör putchar
	call putchar

	; Återställ
	add rsp, 40

	ret

main:
	sub rsp, 1

	xor rdx, rdx
	mov rax, rsp
	sub rax, 8
	mov rcx, 16
	div rcx
	sub rsp, rdx
	push rdx

	xor rcx, rcx
	mov rdx, txt
	mov r8, txt
	xor r9, r9

	sub rsp, 32
	call MessageBoxA
	add rsp, 32

	pop rdx
	add rsp, rdx

	add rsp, 1

	xor rcx, rcx
	call ExitProcess