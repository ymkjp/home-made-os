[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4009.nas"]

		GLOBAL	_osak_gettime

[SECTION .text]

_osak_gettime:		; int osak_gettime(int type);
		MOV		EDX,0x4009
		MOV		ECX,[ESP+4]			; type
		INT		0x40
		RET
