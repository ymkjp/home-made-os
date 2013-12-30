#include "apilib.h"

unsigned char rgb2pal(int r, int g, int b, int x, int y);

void HariMain(void)
{
	char buf[144 * 164 * osak_getbuflen()];
	int win, x, y;
	struct SYS_INFO sysinfo;
	int bpp;
	unsigned short *sp;

	tomo_sysinfo(&sysinfo);
	bpp = sysinfo.vmode;
	win = api_openwin(buf, 144, 164, -1, "color");
	if (bpp == 8) {
		for (y = 0; y < 128; y++) {
			for (x = 0; x < 128; x++) {
				buf[(x + 8) + (y + 28) * 144] = rgb2pal(0, y * 2, x * 2, x, y);
			}
		}
	} else if (bpp == 16) {
		sp = (unsigned short *) buf;
		for (y = 0; y < 128; y++) {
			for (x = 0; x < 128; x++) {
				sp[(x + 8) + (y + 28) * 144] = (((y * 2) << 3) & 0x07e0) | ((x * 2) >> 3);
			}
		}
	}
	api_refreshwin(win, 8, 28, 136, 156);
	api_getkey(1);
	api_end();
}

unsigned char rgb2pal(int r, int g, int b, int x, int y)
{
	static int table[4] = {3, 1, 0, 2};
	int i;
	x &= 1;	/* ‹ô”‚©Šï”‚© */
	y &= 1;
	i = table[x + y * 2];	/* ’†ŠÔF‚ğì‚é‚½‚ß‚Ì’è” */
	r = (r * 21) / 256;
	g = (g * 21) / 256;
	b = (b * 21) / 256;
	r = (r + i) / 4;
	g = (g + i) / 4;
	b = (b + i) / 4;
	return 16 + r + g*6 + b*36;
}
