/* C.V.T.G. for OSAkkie/Haribote  Ver.0.1 */
/* Copyright (C) 2006 Akkiesoft.          */
/* SpecialThanks to: sugi                 */
/* ���e
  ��ʏ�ɏo�����E���E�������o���āA
  �ǂ��ɉ����\������Ă������𓖂Ă�Q�[���B
  ���x���ɂ���Ė��̕\�����Ԃ��ς��B  */
/* malloc:950KB */

#include "apilib.h"

#define srand(seed)			(void) (rand_seed = (seed))
extern unsigned int rand_seed;
int rand(void);

#define BPX		6
#define BPY		26

static int win, timer;

static int px[] = {10, 38, 25, 10, 38};
static int py[] = {2, 2, 7, 12, 12};
static int speed, hispeed;

void putform(char *s, int y, int l);
void putform1(int type, int x, int y,int c);
void putspeed(int mode);
void putstr(int x, int y, int c, int bc, int len, char *s);
void clearstr(int x, int y, int len);
void mytimer(int a, int b, int c);
void setdec3a(unsigned int i, char *s, int j);

void HariMain(void)
{
	int q[5], i, sig, active, miss;	/* q..0=����,1=�E��,2=����,3=����,4=�E�� */
	int answer[5] = {-1, -1, -1, -1, -1};	/* active..0�`4=��(q�Ɠ�������) */
	char s[3];
	char buf[414 * 290 * osak_getbuflen()];

	win = api_openwin(buf, 50 * 8 + 14, 16 * 16 + 34, -1, "C.V.T.G.");
	api_boxfilwin(win, BPX, BPY, 50 * 8 + BPX, 16 * 16 + BPY, 0);
	timer = api_alloctimer();
	api_inittimer(timer, 128);

	srand(tomo_gettick());

	hispeed = 420;	// �N�����Ă����x�����ݒ�
title:
	speed = 999;	// �X�R�A���̏�����
	miss = 0;

	putstr(21, 3, 2, 0,  8, "C.V.T.G.");
	putstr(19, 5, 3, 0, 11, "VERSION 0.3");
	putstr(14, 8, 4, 6, 22, ">> PRESS ENTER KEY. <<");

	/* �G���^�[�L�[ */
	while (api_getkey(1) != 0x0a);

	clearstr(21, 3,  8);
	clearstr(19, 5, 11);
	clearstr(14, 8, 22);

start:
	putspeed(0); // �\��
	for (i = 3; i > 0; i--) {
		s[0] = '0' + i;
		s[1] = 0;
		putstr(25, 6, 2, 0, 1, s);
		mytimer(0, 1, 0);
	}
	clearstr(25, 6, 1);
	putspeed(1); // �폜

	for (i = 0; i < 5; i++) {
		if (i == 2) {
			q[2] = rand() %10;
			s[0] = '0' + q[2];
			s[1] = 0;
			putstr(px[2], py[2], 2, 0, 2, s);
			continue;
		}
		q[i] = rand() % 2;
		putform1(q[i], px[i], py[i], 2);
	}
	mytimer(speed, 0, 0);	/* Lv.�ɂ���ĕς�� */

	putform("  ", 0, 2);	/* form clear */
	mytimer(0, 1, 0);
	putform("__", 0, 2);	/* input form */
	putspeed(0); // �X�s�[�h�\��

	/* �����ʒu�ƃw���v�̐ݒ� */
	putstr(px[0]-1, py[0], 3, 0, 1, ">");
	putstr(0, 15, 6, 0, 42, "HELP: 0=   1=   TAB=MOVE BS=CLEAR ENTER=OK");
	putform1(0, 8, 15, 6);
	putform1(1, 13, 15, 6);
	active = 0;

	for (;;) {	/* keyloop */
		sig = api_getkey(1);

		if ('0' <= sig && sig <= '9') {	/* 0�`9���� */
			sig -= '0';
			if (active == 2) {	/* ���� */
				answer[2] = sig;
				s[0] = '0' + answer[2];
				s[1] = ' ';
				s[2] = 0;
				putstr(px[2], py[2], 2, 0, 2, s);
			} else {			/* ��or�� */
				if (sig > 1)
					continue;
				answer[active] = sig;
				putform1(answer[active], px[active], py[active], 2);
			}
			sig = 9;	/* ����tab */
		}
		if (sig == 8) { /* BackSpace */
			answer[active] = -1;
			putstr(px[active], py[active], 2, 0, 2, "__");
			continue;
		}
		if (sig == 9) { /* Tab */
			clearstr(px[active]-1, py[active], 1); /* ����>������ */
			active++;
			if (active == 5)
				active = 0;
			putstr(px[active]-1, py[active], 3, 0, 1, ">");
			/* �w���v�̕ύX */
			if (active == 2) {
				putstr(6, 15, 6, 0, 9, "0-9=NUM  ");
			} else if (active == 3) {
				putstr(6, 15, 6, 0, 7, "0=   1=");
				putform1(0, 8, 15, 6);
				putform1(1, 13, 15, 6);
			}
			continue;
		}
		if (sig == 10) { /* Enter */
			clearstr(px[active]-1, py[active], 1); /* >������ */
			if (q[0] == answer[0] && q[1] == answer[1] && q[2] == answer[2] && q[3] == answer[3] && q[4] == answer[4]) {
				putstr(24, 10, 1, 6, 4, "HIT!");
				speed = speed * 3 / 4;
			}
			else {
				putstr(23, 10, 4, 6, 5, "MISS!");
				for (i = 0; i < 5; i++) {
					if (i == 2) {
						s[0] = '0' + q[2];
						s[1] = 0;
						putstr(px[i], py[i]+1, 1, 0, 2, s);
					} else {
						putform1(q[i], px[i], py[i]+1, 1);
					}
				}
				miss++;
				putstr(44 + miss, 15, 5, 0, 1, "*");
				if (miss == 3) {
					putstr(21, 10, 4, 6, 10, "GAME OVER!");
					mytimer(0, 1, 0);
				}
			}
			break;
		}
	}
	mytimer(0, 1, 0);
	if (miss < 3) {
		putstr(0, 15, 6, 0, 19, "HELP: [ENTER]=RETRY");
	} else {
		putstr(0, 15, 6, 0, 19, "PRESS ENTER KEY.   ");
	}
	clearstr(19, 15, 23);	/* HELP�̌��Ɏc���������J�X */

	for (;;) {
		if (api_getkey(1) == 10)
			break;
	}

	for (i = 0; i < 5; i++) {
		answer[i] = -1;
	}
	putform("  ", 0, 2);
	putform("  ", 1, 2);
	clearstr(21, 10, 10);	/* HIT!�Ƃ�MISS!�Ƃ�GAME OVER!�̕��� */
	clearstr( 0, 15, 19);	/* HELP */
	if (miss == 3) {
		putspeed(1); 			/* �X�s�[�h�폜   */
		clearstr(45, 15, 3);	/* �~�X�̃J�E���g */
		goto title;
	}
	goto start;
}

void putform(char *s, int y, int l)
/* �t�H�[���Ɉꊇ��s���o�́By=1�Ŗ͔͉𓚕\�� */
{
	int i;
	for (i = 0; i < 5; i++){
		putstr(px[i],  py[i] + y, 2, 0, l, s);
	}
	return;
}

void putform1(int type, int x, int y, int c)
/* a(0=��, 1=��)��pos(x,y)�ɕ\������ */
{
	static unsigned int maru[16] = {
		/* maru1 */
		0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0
	};
	int i, j;

	if (type == 0) {
		/* �� */
		api_boxfilwin(win, x * 8 + BPX, y * 16 + BPY, (x+2) * 8 + BPX - 1, (y+1) * 16 + BPY - 1, c);
	} else if (type == 1) {
		/* �� */
		clearstr(x, y, 2);	/* ������ */
		for (j = 0; j < 16; j++) {
			for (i = 0; i < 16; i++) {
				if ((maru[j] >> (15 - i)) & 0x01) {
					api_point(win+1, x * 8 + BPX + i, y * 16 + BPY + j, c);
				}
			}
		}
		api_refreshwin(win, x * 8 + BPX, y * 16 + BPY, (x+2) * 8 + BPX, (y+1) * 16 + BPY);
	}
	return;
}

void putspeed(int mode)
/* speed�\�� */
{
	char spd[5];

	if (mode) /* �N���A */
		clearstr(0, 0, 29);
	else {
		putstr(0, 0, 7, 0, 6, "SPEED:");
		putstr(13, 0, 7, 0, 11, "HIGH-SPEED:");
		spd[4] = '\0';
		setdec3a(speed, spd, 4);
		putstr(7, 0, 7, 0, 4, spd);
		if (speed < hispeed) 
			hispeed = speed;	// high-speed�X�V
		setdec3a(hispeed, spd, 4);
		putstr(25, 0, 7, 0, 4, spd);
	}
	return;
}

void putstr(int x, int y, int c, int bc, int len, char *s)
/* �����o�� */
{
	int px, py;
	px = x *  8 + BPX;
	py = y * 16 + BPY;
	api_boxfilwin(win + 1, px, py, px + len * 8 - 1, py + 16 - 1, bc);
	api_putstrwin(win + 1, x * 8 + BPX, y * 16 + BPY, c, len, s);
	api_refreshwin(win, px, py, px + len * 8, py + 16);
	return;
}

void clearstr(int x, int y, int len)
/* �������������� */
{
	int px, py;
	px = x *  8 + BPX;
	py = y * 16 + BPY;
	api_boxfilwin(win, px, py, px + len * 8 - 1, py + 16 - 1, 0);
	return;
}

void mytimer(int a, int b, int c)
/* �^�C�}�[���I���܂ő��̃L�[��W�Q�Blib_settimertime�Ɠ����d�l */
{
	if (a >= 1000) {
		b += a / 1000;
		a = a % (b * 1000);

	}
	api_settimer(timer, b*100 + a/10);
	for (;;) {
		if (api_getkey(1) == 128)
			break;
	}
	return;
}

void setdec3a(unsigned int i, char *s, int j)
/* ���l�ϐ��𕶎���ɁBj�͌������w�肷��B */
{
	j--;
	do {
		s[j--] = (i % 10) + '0';
		if ((i /= 10) == 0)
			break;
	} while (j >= 0);
	while (j >= 0)
		s[j--] = ' ';
	return;
}
