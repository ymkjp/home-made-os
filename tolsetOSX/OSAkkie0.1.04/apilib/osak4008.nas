[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "osak4008.nas"]

		GLOBAL	_osak_freepicture

[SECTION .text]

_osak_freepicture:		; void osak_freepicture(int pic);
		MOV		EDX,0x4008
		MOV		EAX,[ESP+4]			; pic
		INT		0x40
		RET
