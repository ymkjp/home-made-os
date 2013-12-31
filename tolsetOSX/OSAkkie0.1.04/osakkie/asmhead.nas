; haribote-os boot asm
; TAB=4

[INSTRSET "i486p"]

; ��΂ɓ������l���w�肵�Ȃ�
VBEMODE1	EQU		0x117			; ��1��� XGA16bit�J���[
VBEMODE2	EQU		0x105			; ��2��� XGA 8bit�J���[

BOTPAK	EQU		0x00280000		; bootpack�̃��[�h��
DSKCAC	EQU		0x00100000		; �f�B�X�N�L���b�V���̏ꏊ
DSKCAC0	EQU		0x00008000		; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO�֌W
CYLS	EQU		0x0ff0			; �u�[�g�Z�N�^���ݒ肷��
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; �F���Ɋւ�����B���r�b�g�J���[���H
SCRNX	EQU		0x0ff4			; �𑜓x��X
SCRNY	EQU		0x0ff6			; �𑜓x��Y
VRAM	EQU		0x0ff8			; �O���t�B�b�N�o�b�t�@�̊J�n�Ԓn

		ORG		0xc200			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; ��ʐݒ�::VBE�����邩
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT 	0x10
		CMP		AX,0x004f
		JNE		scrn320

; ��ʐݒ�::VBE�o�[�W�����`�F�b�N
		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320

; ��ʐݒ�::VBEMODE�Ŏw�肵�����[�h�̏����擾
		MOV		CX,VBEMODE1				; ��1���
scrn_retry:								; ��2���͂�������
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn_next				; ���̌���

; ��ʐݒ�::��ʃ��[�h���̊m�F
		MOV		AX,[ES:DI+0x00]			; 8bit, 16bit ���ʃ`�F�b�N����
		AND		AX,0x0080
		JZ		scrn_next
		MOV		AX,CX
		AND		AX,0x0010				; 8�r�b�g�J���[��16�r�b�g�J���[�����ׂ�
		CMP		AX,0x0010
		JE		scrn_16bit				; 16�r�b�g�̏ꍇ�̓W�����v
		MOV		DL,8					;  8�r�b�g�̉�ʃ��[�h�m�F
		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn_next
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn_next

; ��ʐݒ�::���[�h�ؑ�
scrn_set:
		ADD		CX,0x4000
		MOV		BX,CX					;	MOV		BX,VBEMODE1+0x4000
		SUB		CX,0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],DL			; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

; ���؂�ւ�
scrn_next:
		CMP		CX,VBEMODE1				; 2��ڂ�
		JE		scrn_next1to2
		CMP		CX,VBEMODE2				; 2��ڂ����߂Ȃ̂�scrn320��
		JE		scrn320
scrn_next1to2:
		MOV		CX,VBEMODE2
		JMP		scrn_retry

; 16�r�b�g�̉�ʃ��[�h�m�F
scrn_16bit:
		MOV		DL,16
		CMP		BYTE [ES:DI+0x19],16
		JNE		scrn_next
		CMP		BYTE [ES:DI+0x1b],6
		JNE		scrn_next
		JMP		scrn_set

; ��ʐݒ�::VBEMODE�Ŏw�肵�����[�h���g���Ȃ�����
scrn320:
		MOV		AL,0x13					; VGA�O���t�B�b�N�X�A320x200x8bit�J���[
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8			; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

; �L�[�{�[�h��LED��Ԃ�BIOS�ɋ����Ă��炤
keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; PIC����؂̊��荞�݂��󂯕t���Ȃ��悤�ɂ���
;	AT�݊��@�̎d�l�ł́APIC�̏�����������Ȃ�A
;	������CLI�O�ɂ���Ă����Ȃ��ƁA���܂Ƀn���O�A�b�v����
;	PIC�̏������͂��Ƃł��

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; OUT���߂�A��������Ƃ��܂������Ȃ��@�킪����炵���̂�
		OUT		0xa1,AL

		CLI						; �����CPU���x���ł����荞�݋֎~

; CPU����1MB�ȏ�̃������ɃA�N�Z�X�ł���悤�ɁAA20GATE��ݒ�

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; �v���e�N�g���[�h�ڍs


		LGDT	[GDTR0]			; �b��GDT��ݒ�
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31��0�ɂ���i�y�[�W���O�֎~�̂��߁j
		OR		EAX,0x00000001	; bit0��1�ɂ���i�v���e�N�g���[�h�ڍs�̂��߁j
		MOV		CR0,EAX
		JMP		pipelineflash
pipelineflash:
		MOV		AX,1*8			;  �ǂݏ����\�Z�O�����g32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack�̓]��

		MOV		ESI,bootpack	; �]����
		MOV		EDI,BOTPAK		; �]����
		MOV		ECX,512*1024/4
		CALL	memcpy

; ���łɃf�B�X�N�f�[�^���{���̈ʒu�֓]��

; �܂��̓u�[�g�Z�N�^����

		MOV		ESI,0x7c00		; �]����
		MOV		EDI,DSKCAC		; �]����
		MOV		ECX,512/4
		CALL	memcpy

; �c��S��

		MOV		ESI,DSKCAC0+512	; �]����
		MOV		EDI,DSKCAC+512	; �]����
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; �V�����_������o�C�g��/4�ɕϊ�
		SUB		ECX,512/4		; IPL�̕�������������
		CALL	memcpy

; asmhead�ł��Ȃ���΂����Ȃ����Ƃ͑S�����I������̂ŁA
;	���Ƃ�bootpack�ɔC����

; bootpack�̋N��

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; �]������ׂ����̂��Ȃ�
		MOV		ESI,[EBX+20]	; �]����
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; �]����
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; �X�^�b�N�����l
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND�̌��ʂ�0�łȂ����waitkbdout��
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; �����Z�������ʂ�0�łȂ����memcpy��
		RET
; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

		ALIGNB	16
GDT0:
		RESB	8				; �k���Z���N�^
		DW		0xffff,0x0000,0x9200,0x00cf	; �ǂݏ����\�Z�O�����g32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; ���s�\�Z�O�����g32bit�ibootpack�p�j

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
