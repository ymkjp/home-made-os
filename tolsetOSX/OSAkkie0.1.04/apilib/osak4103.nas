[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4103.nas"]

		GLOBAL	_osak_linewin

[SECTION .text]

_osak_linewin:		; void osak_linewin(int win, int x0, int y0, int x1, int y1, int col);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,0x4103
		MOV		EBX,[ESP+20]		; win
		MOV		EAX,[ESP+24]		; x0
		MOV		ECX,[ESP+28]		; y0
		MOV		ESI,[ESP+32]		; x1
		MOV		EDI,[ESP+36]		; y1
		MOV		EBP,[ESP+40]		; col
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET
