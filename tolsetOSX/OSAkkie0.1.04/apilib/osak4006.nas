[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4006.nas"]

		GLOBAL	_osak_initpicture

[SECTION .text]

_osak_initpicture:		; int osak_initpicture(char *name);
		PUSH	EBP
		MOV		EDX,0x4006
		MOV		EBP,[ESP+8]		; name
		INT		0x40
		POP		EBP
		RET
