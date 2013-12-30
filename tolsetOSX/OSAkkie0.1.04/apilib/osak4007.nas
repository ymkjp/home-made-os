[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4007.nas"]

		GLOBAL	_osak_drawpicture

[SECTION .text]

_osak_drawpicture:		; int osak_drawpicture(int win, int pic, int px, int py);
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,0x4007
		MOV		EBX,[ESP+16]		; win
		MOV		EAX,[ESP+20]		; pic
		MOV		ECX,[ESP+24]		; px
		MOV		ESI,[ESP+28]		; py
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		RET
