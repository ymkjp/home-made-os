[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4102.nas"]

		GLOBAL	_osak_point

[SECTION .text]

_osak_point:			; void osak_point(int win, int x, int y, int c);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,0x4102
		MOV		EBX,[ESP+16]		; win
		MOV		ESI,[ESP+20]		; x
		MOV		EDI,[ESP+24]		; y
		MOV		EAX,[ESP+28]		; col
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET
