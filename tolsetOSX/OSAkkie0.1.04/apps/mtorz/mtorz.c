#include "apilib.h"

void HariMain(void)
{
	char buf[152 * 100 * osak_getbuflen()], col;
	int win, timer;
	int bpx = 8, bpy = 26;

	win = api_openwin(buf, 152, 100, -1, "Mt.orz");
	timer = api_alloctimer();
	api_inittimer(timer, 128);

	for (;;) {
		for (col = 8; col < 16; col++) {
			if (col == 8) { col = 0; }	/* OSASK‚ÌƒpƒŒƒbƒg‚Æ‚Í”÷–­‚Éˆá‚¤‚½‚ß */
			api_putstrwin(win, 7 * 8 + bpx, 0      + bpy, col,  3, "orz");
			api_putstrwin(win, 5 * 8 + bpx,     16 + bpy, col,  7, "orz orz");
			api_putstrwin(win, 3 * 8 + bpx, 2 * 16 + bpy, col, 11, "orz orz orz");
			api_putstrwin(win,     8 + bpx, 3 * 16 + bpy, col, 15, "orz orz orz orz");
			if (col == 0) { col = 8; }	/* ã‚Æ“¯‚¶ */
			api_settimer(timer, 50);	/* 0.5•b */
			if (api_getkey(1) != 128) {
				goto end;
			}
		}
	}

end:
	api_closewin(win);
	api_end();
}
