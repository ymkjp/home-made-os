[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "tomo1005.nas"]

		GLOBAL	_tomo_systime

[SECTION .text]

_tomo_systime:	; void tomo_systime(struct TIME_INFO *info);
		MOV		EDX,0x1005
		MOV		EAX,[ESP+4]			; info
		INT		0x40
		RET
