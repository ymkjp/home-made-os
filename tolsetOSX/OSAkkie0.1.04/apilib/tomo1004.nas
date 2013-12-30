[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "tomo1004.nas"]

		GLOBAL	_tomo_sysinfo

[SECTION .text]

_tomo_sysinfo:	; void tomo_sysinfo(struct SYS_INFO *info);
		MOV		EDX,0x1004
		MOV		EAX,[ESP+4]			; info
		INT		0x40
		RET
