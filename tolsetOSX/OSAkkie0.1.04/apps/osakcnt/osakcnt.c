#include <stdio.h>
#include "apilib.h"

void HariMain(void)
{
	char buf[216 * 52 * osak_getbuflen()], s[26];
	int win, timer;
	int bpx = 8, bpy = 28, x, y;
	int i, t;

	x = 25 *  8;
	y =  1 * 16;

	win = api_openwin(buf, x + 16, y + 36, -1, "OSAkkie Counter");
	api_boxfilwin(win, bpx, bpy, x + bpx, y + bpy, 0);
	timer = api_alloctimer();
	api_inittimer(timer, 128);

	for (;;) {
		t = tomo_gettick();
		sprintf(s, "%3dday %2dhour %2dmin %2dsec",
			t/8640000,
			(t/360000)%24,
			(t/6000)%60,
			(t/100)%60
		);
		api_boxfilwin(win + 1, bpx, bpy, x + bpx, y + bpy, 0);
		api_putstrwin(win + 1, bpx, bpy, 7, 25, s);
		api_refreshwin(win, bpx, bpy, x + bpx, y + bpy);

		api_settimer(timer, 25);	/* 0.25•b */
		for (;;) {
			i = api_getkey(1);
			if (i == 128 || i == 0x0a) {
				break;
			}
		}
		if (i == 0x0a) {
			break;
		}
	}
	api_closewin(win);
	api_end();
}
