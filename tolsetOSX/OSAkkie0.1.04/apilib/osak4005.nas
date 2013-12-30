[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4005.nas"]

		GLOBAL	_osak_getcolor

[SECTION .text]

_osak_getcolor:		; int osak_getcolor(int bpp, int col);
		PUSH	EBX
		MOV		EDX,0x4005
		MOV		EBX,[ESP+8]			; bpp
		MOV		ECX,[ESP+12]		; col
		INT		0x40
		POP		EBX
		RET
