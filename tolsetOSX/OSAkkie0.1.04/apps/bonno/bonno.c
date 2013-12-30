#include <stdio.h>
#include "apilib.h"

#define BPX		8
#define BPY		28

void HariMain(void)
{
	char buf[256 * 164 * osak_getbuflen()], s[5];
	int win, i, mx;

	tomo_setlang(0);
	win = api_openwin(buf, 30 *  8 + 16, 8 * 16 + 36, -1, "Bonno");
	api_boxfilwin(win, 8, 28, 30 *  8 + BPX, 8 * 16 + BPY, 0);

	api_putstrwin(win,  2 * 8 + BPX, 0 * 16 + BPY, 7, 28, "108 Bonno                  \\");
	api_putstrwin(win,  0 * 8 + BPX, 1 * 16 + BPY, 7, 30, "------------------------------");
	api_putstrwin(win,  2 * 8 + BPX, 2 * 16 + BPY, 7, 17, "/        \\      |");
	api_putstrwin(win,  2 * 8 + BPX, 3 * 16 + BPY, 7, 17, "|        |      |");
	api_putstrwin(win,  2 * 8 + BPX, 4 * 16 + BPY, 7, 21, "|        |  ____|____");
	api_putstrwin(win,  2 * 8 + BPX, 5 * 16 + BPY, 7, 22, "|========| |______/- \\");
	api_putstrwin(win,  2 * 8 + BPX, 6 * 16 + BPY, 7, 22, "|________|      | \\= |");
	api_putstrwin(win, 18 * 8 + BPX, 7 * 16 + BPY, 7,  6, "@=[  \\");

	i  = 108;
	mx = 8;
	for (;;) {
		if (api_getkey(1) == 0x20) {
			if (i % 2) {
				mx += 8;
			} else {
				mx -= 8;
			}
			api_boxfilwin(win, 96 + BPX, 64 + BPY, 192 + BPX, 128 + BPY, 0);
			api_putstrwin(win, mx + 13 * 8 + BPX, 4 * 16 + BPY, 7, 10, "____|____ ");
			api_putstrwin(win, mx + 12 * 8 + BPX, 5 * 16 + BPY, 7, 12, "|______/- \\ ");
			api_putstrwin(win, mx + 17 * 8 + BPX, 6 * 16 + BPY, 7,  8, "| \\= | ");
			api_putstrwin(win, mx + 17 * 8 + BPX, 7 * 16 + BPY, 7,  8, "@=[  \\ ");
			i--;
			sprintf(s, "%3d", i);
			api_boxfilwin(win, 16 + BPX, BPY, 40 + BPX, 16 + BPY, 0);
			api_putstrwin(win, 16 + BPX, BPY, 7, 3, s);
			if (i <= 0) {
				break;
			}
		}
	}

	api_boxfilwin(win, 144 + BPX, 64 + BPY, 232 + BPX, 128 + BPY, 0);
	api_putstrwin(win, 19 * 8 + BPX, 2 * 16 + BPY, 7,  7, "A Happy");
	api_putstrwin(win, 20 * 8 + BPX, 3 * 16 + BPY, 7, 10, "New Year!!");
	api_putstrwin(win, 18 * 8 + BPX, 4 * 16 + BPY, 7,  6, "|_ ___");
	api_putstrwin(win, 13 * 8 + BPX, 5 * 16 + BPY, 7, 12, "|______/^ ^\\");
	api_putstrwin(win, 18 * 8 + BPX, 6 * 16 + BPY, 7,  7, "| | v |");
	api_putstrwin(win, 18 * 8 + BPX, 7 * 16 + BPY, 7,  9, "|/[@ @]\\");\

	while (api_getkey(1) != 0x20);

	api_closewin(win);
	api_end();
}
