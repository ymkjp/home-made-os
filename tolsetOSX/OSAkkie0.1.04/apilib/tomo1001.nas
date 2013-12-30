[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "tomo1001.nas"]

		GLOBAL	_tomo_gettick

[SECTION .text]

_tomo_gettick:	; int tomo_gettick(void);
		MOV		EDX,0x1001
		INT		0x40
		RET
