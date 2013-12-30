[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4003.nas"]

		GLOBAL	_osak_exec

[SECTION .text]

_osak_exec:		; void osak_exec(char *name);
		PUSH	EBP
		MOV		EDX,0x4003
		MOV		EBP,[ESP+8]		; name
		INT		0x40
		POP		EBP
		RET
