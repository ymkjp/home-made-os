/* LuckyNumber for OSAkkie/HariboteOS  Ver.0.1 */
/* Copyright (C) 2006 Akkiesoft.    2006/10/19 */

#include "apilib.h"

#define srand(seed)			(void) (rand_seed = (seed))
extern unsigned int rand_seed;
int rand(void);

#define BPX		6
#define BPY		26

void HariMain(void)
{
	char buf[214 * 82 * osak_getbuflen()], s[2];
	int win, col;

	win = api_openwin(buf, 25 * 8 + 14, 3 * 16 + 34, -1, "LuckyNum");
	api_boxfilwin(win, BPX, BPY, 25 * 8 + BPX, 3 * 16 + BPY, 0);

	api_putstrwin(win, BPX,      BPY, 6, 12, "LuckyNumber=");
	api_putstrwin(win, BPX, 16 + BPY, 6, 12, "LuckyColor =");
	api_putstrwin(win, BPX, 32 + BPY, 3, 17, "Good Luck!(^o^)/~");

	srand(tomo_gettick()); /* seedを与える */

	s[0] = '0' + rand() % 10;
	s[1] = '\0';
	api_putstrwin(win, 13 * 8 + BPX, BPY, 6, 1, s);

	col = rand() % 9;	/* Hariboteは明るい色が前に集まっているので変なループはいらない */

	api_boxfilwin(win, 13 * 8 + BPX, 16 + BPY, 14 * 8 + BPX - 1, 32 + BPY - 1, col);

	while (api_getkey(1) != 0x0a);

	api_closewin(win);
	api_end();
}
