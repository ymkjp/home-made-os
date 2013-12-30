#include <stdio.h>
#include "apilib.h"

void make_textbox(int win, int x0, int y0, int sx, int sy);

void HariMain(void)
{
	char buf[416 * 52 * osak_getbuflen()], s[2];
	int win, timer, i, curc = 0, curx = 8;
	win = api_openwin(buf, 416, 50, -1, "1Line");
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	make_textbox(win, 8, 28, 400, 16);
	s[1] = '\0';
	api_settimer(timer, 50);	/* 0.5•b */
	for (;;) {
		i = api_getkey(1);
		if (i == 128) {
			if (curc == 0) {
				/* On */
				curc = 7;
			} else {
				/* Off */
				curc = 0;
			}
			api_boxfilwin(win, curx, 43, 7 + curx, 43, curc);
			api_settimer(timer, 50);	/* 0.5•b */
		}
		if (i == 0x08) {
			/* BackSpace */
			if (8 < curx) {
				api_boxfilwin(win, curx - 8, 28, curx + 7, 43, 7);
				curx -= 8;
				api_boxfilwin(win, curx, 43, 7 + curx, 43, curc);
			}
		}
		if (0x20 <= i && i <= 'z') {
			if (curx < 400) {
				api_boxfilwin(win, curx, 28 + 15, 7 + curx, 28 + 15,  7);
				s[0] = i;
				api_putstrwin(win, curx, 28, 0, 1, s);
				curx += 8;
				api_boxfilwin(win, curx, 28 + 15, 7 + curx, 28 + 15,  curc);
			}
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
