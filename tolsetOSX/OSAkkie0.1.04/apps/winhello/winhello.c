#include "apilib.h"

void HariMain(void)
{
	char buf[150 * 80 * osak_getbuflen()];
	int win, i;
	win = api_openwin(buf, 150, 80, -1, "hello!");
	api_boxfilwin(win,  8, 28, 141, 73, 240);	/* OSAkkieîwåiêF */
	api_putstrwin(win, 16, 30, 7/* White */, 16, "OSAkkie world!?");
	osak_putministrwin(win, 16, 50, 0xffffff/* White */, 16, "OSAkkie world!?");

	for (;;) {
		i = api_getkey(1);
		if (i == 0x0a) {
			break;
		}
	}

	api_closewin(win);
	api_end();
}
