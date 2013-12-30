#include <stdio.h>
#include "apilib.h"

void make_textbox(int win, int x0, int y0, int sx, int sy);

void HariMain(void)
{
	char buf[184 * 52 * osak_getbuflen()], naomi[3] = { 0x80, 0x81, 0 }, s[20];
	int win, timer, i, c = 0;
	struct TIME_INFO t;
	tomo_setlang(0);
	win = api_openwin(buf, 184, 50, -1, "Clock");
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	make_textbox(win, 8, 28, 168, 16);
	api_settimer(timer, 50);	/* 0.5•b */
	for (;;) {
		i = api_getkey(1);
		if (i == 128) {
			if (c == 0) {
				c = 7;
			} else {
				c = 0;
			}
			tomo_systime(&t);
			sprintf(s, "%04d/%02d/%02d %02d:%02d:%02d",
				t.year, t.month, t.day, t.hour, t.minutes, t.second);
			api_boxfilwin(win + 1, 8, 28, 160, 44, 7);
			api_putstrwin(win, 8, 28, 0, 19, s);
			api_putstrwin(win, 160, 28, c, 2, naomi);
			api_settimer(timer, 50);	/* 0.5•b */
		}
	}
	api_closewin(win);
	api_end();
}

void make_textbox(int win, int x0, int y0, int sx, int sy)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	api_boxfilwin(win, x0 - 2, y0 - 3, x1 + 1, y0 - 3, 15);
	api_boxfilwin(win, x0 - 3, y0 - 3, x0 - 3, y1 + 1, 15);
	api_boxfilwin(win, x0 - 3, y1 + 2, x1 + 1, y1 + 2,  7);
	api_boxfilwin(win, x1 + 2, y0 - 3, x1 + 2, y1 + 2,  7);
	api_boxfilwin(win, x0 - 1, y0 - 2, x1 + 0, y0 - 2,  0);
	api_boxfilwin(win, x0 - 2, y0 - 2, x0 - 2, y1 + 0,  0);
	api_boxfilwin(win, x0 - 2, y1 + 1, x1 + 0, y1 + 1,  8);
	api_boxfilwin(win, x1 + 1, y0 - 2, x1 + 1, y1 + 1,  8);
	api_boxfilwin(win, x0 - 1, y0 - 1, x1 + 0, y1 + 0,  7);
	return;
}
