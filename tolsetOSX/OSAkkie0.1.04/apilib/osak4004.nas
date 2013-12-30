[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4004.nas"]

		GLOBAL	_osak_getbuflen

[SECTION .text]

_osak_getbuflen:	; int osak_getbuflen(void);
		MOV		EDX,0x4004
		INT		0x40
		RET
