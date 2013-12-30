[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api023.nas"]

		GLOBAL	_api_fseek

[SECTION .text]

_api_fseek:		; int api_fseek(int fhandle, int offset, int mode);
		PUSH	EBX
		MOV		EDX,23
		MOV		EAX,[ESP+ 8]		; fhandle
		MOV		ECX,[ESP+12]		; offset
		MOV		EBX,[ESP+16]		; mode
		INT		0x40
		POP		EBX
		RET
