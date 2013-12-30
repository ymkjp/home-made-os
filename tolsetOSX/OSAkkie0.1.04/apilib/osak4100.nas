[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4100.nas"]

		GLOBAL	_osak_putstrwin

[SECTION .text]

_osak_putstrwin:		; void osak_putstrwin(int win, int x, int y, int col, int len, char *str);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,0x4100
		MOV		EBX,[ESP+20]		; win
		MOV		ESI,[ESP+24]		; x
		MOV		EDI,[ESP+28]		; y
		MOV		EAX,[ESP+32]		; col
		MOV		ECX,[ESP+36]		; len
		MOV		EBP,[ESP+40]		; str
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET
