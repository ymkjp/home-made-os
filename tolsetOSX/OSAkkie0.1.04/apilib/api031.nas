[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api031.nas"]

		GLOBAL	_api_sendkey

[SECTION .text]

_api_sendkey:       ; int api_sendkey(char *);
        PUSH    EBX
        MOV     EDX,31
        MOV     EBX,[ESP+8]
        INT     0x40
        POP     EBX
        RET
