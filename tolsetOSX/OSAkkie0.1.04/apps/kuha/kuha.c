/*   98 -kuha- for OSAkkie@HariboteOS   Ver.0.1(2006/10/18)  */
/*   Copyright (C) 2006 Akkiesoft.                           */

#include "apilib.h"

static int win, timer;

void wait(int time)
{
	api_settimer(timer, time);
	while (api_getkey(1) != 128);
	return;
}

void putstr(char *s, int x, int y, int l)
{
	api_boxfilwin(win + 1, x * 8 + 6, y * 16 + 26, (x + l) * 8 + 6, (y + 1) * 16 + 26, 0);
	api_putstrwin(win, x * 8 + 6, y * 16 + 26, 7, l, s);
	return;
}

void HariMain()
{
	int len = osak_getbuflen();
	char buf[496 * 340 * len];
	int i;

	win = api_openwin(buf, 480 + 16, 304 + 36, -1, "kuha");
	api_boxfilwin(win, 6, 26, 480 + 6, 304 + 26, 0);
	timer = api_alloctimer();
	api_inittimer(timer, 128);

reboot:
	api_boxfilwin(win, 6, 26, 25 * 8 + 6, 32 + 26, 0);
	wait(100);

	api_beep(2001000);
	wait(5);
	api_beep(1000500);
	wait(5);
	api_beep(0);
	wait(25);

	putstr("CPU MODE  High", 0, 0, 14);
	wait(10);
	putstr("MEMORY 128KB OK", 0, 1, 15);
	wait(15);
	putstr("256", 7, 1, 3);
	wait(15);
	putstr("384", 7, 1, 3);
	wait(15);
	putstr("512", 7, 1, 3);
	wait(15);
	putstr("640", 7, 1, 3);
	wait(15);

	putstr( "+     KB OK", 13, 1, 14);

	for (i = 1; i <= 40; i++) {
		char s[5];
		int j = 3, ic = i * 128;

		do {
			s[j--] = (ic % 10) + '0';
			if ((ic /= 10) == 0)
				break;
		} while (j >= 0);
		while (j >= 0)
			s[j--] = ' ';

		putstr(s, 15, 1, 4);
		wait(75 / 10);
	}
	wait(200);

	goto reboot;
}
