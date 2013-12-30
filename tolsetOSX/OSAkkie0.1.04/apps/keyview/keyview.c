#include <stdio.h>
#include "apilib.h"

#define BPX		8
#define BPY		28

void HariMain(void)
{
	char buf[124 * 52 * osak_getbuflen()], s[5];
	int win, i;

	win = api_openwin(buf, 108 + 16, 16 + 36, -1, "KeyView");
	api_boxfilwin(win, 8, 28, 108 + BPX, 16 + BPY, 0);

	for (;;) {
		i = api_getkey(1);
		if (i) {
			sprintf(s, "0x%02x", i);
			api_boxfilwin(win + 1, BPX, BPY, 40 + BPX, 16 + BPY, 0);
			api_putstrwin(win, BPX, BPY, 7, 10, s);
		}
	}
}
