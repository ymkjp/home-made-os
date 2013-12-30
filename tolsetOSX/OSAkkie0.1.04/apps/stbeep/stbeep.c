/* --- ST Beep  Ver.0.1 ----- Date: 2006-12-06 ---------------------- */
/* -------------------------------- Copyright (C) 2006 Akkiesoft. --- */

#include "apilib.h"

void waittimer(int timer, int time);

void HariMain(void)
{
	char buf[355 * 110 * osak_getbuflen()];
	int win, timer, st;

	/* 画像読み込み */
	st = osak_initpicture("stbeep.bmp");
	if (st < 0) {
		/* ファイルがない */
		api_putstr0("stbeep.bmp not found.\n");
		api_end();
	}

	/* ウィンドウの準備 */
	win = api_openwin(buf, 355, 100, -1, "ST Beep [Toho Line]");

	/* タイマの準備 */
	timer = api_alloctimer();
	api_inittimer(timer, 128);

	osak_drawpicture(win, st, 2, 21);
	osak_freepicture(st);

	api_beep(587400);
	waittimer(timer, 50);
	api_beep(0);
	waittimer(timer, 150);
	api_beep(98000);
	waittimer(timer, 300);

	api_beep(0);
	api_closewin(win);
	api_end();
}

void waittimer(int timer, int time)
{
	int i;
	api_settimer(timer, time);
	for (;;) {
		i = api_getkey(1);
		if (i == 'Q' || i == 'q') {
			api_beep(0);
			api_end();
		}
		if (i == 128) {
			return;
		}
	}
}
