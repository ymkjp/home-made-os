[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "tomo1003.nas"]

		GLOBAL	_tomo_setlang

[SECTION .text]

_tomo_setlang:	;void tomo_setlang(int lang);
 		PUSH	EBX
 		MOV		EDX,0x1003
 		MOV		EBX,[ESP+8]		; lang
 		INT		0x40
 		POP		EBX
		RET
