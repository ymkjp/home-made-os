/* ------------------------------------------------------------------ */
/*   OSAkkie Application Launcher  Ver.0.2 (2007-01-05)               */
/*   stack: 120k, malloc:0k        Copyright (C) 2006-2007 Akkiesoft. */
/* ------------------------------------------------------------------ */

#include "apilib.h"

static int win, timer;

#define APPS		20

/* Zakkyｻﾝ ﾉ menu.c ﾉ ｺｰﾄﾞ ｦｻﾝｺｳ ﾆ ｼﾏｼﾀ｡（なぜここも半角カタカナ？） */
static struct {
	char filename[9];
	int arg;
	char filename2[22];
	char desc[5][30];
} list[APPS] = {
	{"1line   ", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 25ﾆﾁﾒ ﾏﾃﾞ task_a ", "ﾄｼﾃ ｱｯﾀ ｳｨﾝﾄﾞｳ ｦ ｱﾌﾟﾘｹｰｼｮﾝ ﾃﾞ", "ｻｲｹﾞﾝ ｼﾏｼﾀ｡                  ", "", ""}},
	{"bball   ", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 29ﾆﾁﾒﾖﾘ bballﾃﾞｽ｡", "ｾﾝ 1ﾎﾟﾝ 1ﾎﾟﾝ ﾉ ｴﾚｶﾞﾝﾄﾅ ｶｶﾞﾔｷｦ", "ｵﾀﾉｼﾐ ｸﾀﾞｻｲ｡                 ", "", ""}},
	{"bonno   ", 0, "", {"ﾎﾞﾝﾉｰ ｳﾁﾊﾗｲ ｼｴﾝ ｱﾌﾟﾘ ﾃﾞｽ｡    ", "ｽﾍﾟｰｽｷｰ ｦ ｵｼﾃ ﾎﾞﾝﾉｰ ｦ ｳﾁﾊﾗｵｳ!", "", "", ""}},
	{"clock   ", 0, "", {"ｹﾞﾝｻﾞｲ ﾉ ｼﾞｺｸ ｦ ﾋｮｳｼﾞ ｼﾏｽ｡   ", "･･･ｴ? ｽﾞﾚﾃ ｲﾏｽｶ? ｿﾚﾊ ｱﾅﾀ ﾉ PC", "ﾉ ﾄｹｲ ｶﾞ ｽﾞﾚﾃ ｲﾙﾉﾃﾞｽ!!!      ", "", ""}},
	{"color   ", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 25ﾆﾁﾒﾖﾘ colorﾃﾞｽ｡", "ﾎﾝﾃﾞﾊ ｱｶｹｲ ﾉ ｲﾛ ﾃﾞｼﾀｶﾞ､ ｱｯｷｨﾉ", "ｼｭﾐﾆﾖﾘ､ ｱｵｹｲﾆ ﾍﾝｺｳ ｻﾚﾃ ｲﾏｽ｡  ", "", ""}},
	{"csvv    ", 1, "csvv sample.csv", {"ｶﾝｲ CSV ﾋﾞｭｰｱ ﾃﾞｽ｡ ｶｰｿﾙ ｷｰ ﾔ､", "PageUp,Down ｷｰ ﾃﾞｿｳｻ ﾃﾞｷﾏｽ｡  ", "ｺﾉ ﾒﾆｭｰ ｶﾗ ﾊ ｻﾝﾌﾟﾙ ｦ ﾋﾗｷﾏｽ｡ ", "ﾌﾟﾛﾝﾌﾟﾄ ｶﾗ ｢csvv ﾌｧｲﾙﾒｲ｣ ﾉﾖｳﾆ", "ﾆｭｳﾘｮｸ ｼﾃﾓ ﾋﾗｹﾏｽ｡            "}},
	{"cvtg    ", 0, "", {"ｼｭｳﾍﾝｼﾔ ﾄｲｳﾓﾉ ｦ ｷﾀｴﾙ ｺﾄｶﾞ    ", "ﾃﾞｷﾙ ｱﾌﾟﾘ ﾃﾞｽ｡ ｶﾞﾒﾝ ﾆ ﾃﾞﾃｸﾙ  ", "ﾏﾙ ｼｶｸ ﾉ ｷｺﾞｳ ﾄ ｽｳｼﾞ ｦ ｾｲｹﾞﾝ ", "ｼﾞｶﾝ ﾉ ｳﾁﾆ ｵﾎﾞｴﾃ ｸﾀﾞｻｲ｡      ", ""}},
	{"gview   ", 1, "gview fukurou.jpg", {"OSｼﾞｻｸﾆｭｳﾓﾝ 30ﾆﾁﾒﾖﾘ ｶﾞｿﾞｳ    ", "ﾋﾞｭｰｱﾃﾞｽ｡ bmp, jpeg, ico ﾌｧｲﾙ", "ｦ ﾋﾗｸｺﾄｶﾞ ﾃﾞｷﾏｽ｡  ﾌﾟﾛﾝﾌﾟﾄ ｶﾗ ", "｢gview ﾌｧｲﾙﾒｲ｣ ﾄ ﾆｭｳﾘｮｸ ｼﾃﾓ  ", "ﾋﾗｹﾏｽ｡"}},
	{"invader ", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 30ﾆﾁﾒﾖﾘ ｲﾝﾍﾞｰﾀﾞｰ ", "ｹﾞｰﾑ ﾃﾞｽ｡ ｼﾝﾘｬｸ ｼﾃｸﾙ ｳﾁｭｳｼﾞﾝ ", "ｶﾗ ﾁｷｭｳ ｦ ﾏﾓﾛｳ!              ", "", ""}},
	{"keyview ", 0, "", {"ｱﾌﾟﾘ ﾆ ﾆｭｳﾘｮｸ ｻﾚﾀ ｷｰｺｰﾄﾞ ｦ   ", "ﾋｮｳｼﾞ ｽﾙﾀﾞｹ ﾉ ﾃｽﾄ ｱﾌﾟﾘ ﾃﾞｽ｡  ", "", "", ""}},
	{"kuha    ", 0, "", {"ﾑｶｼ ﾅﾂｶｼ NEC PC-98x1ｼﾘｰｽﾞ ﾉ  ", "ｴﾐｭﾚｰﾀ ﾃﾞｽ｡ ﾍｯﾎﾟｺ ﾅﾉﾃﾞ ﾒﾓﾘｰ  ", "ﾃｽﾄ ﾁｮｸｺﾞ ﾆ ｻｲｷﾄﾞｳ ｼﾃｼﾏｲﾏｽ｡  ", "ﾋﾞｰﾌﾟﾉ ﾅﾙ ｶﾝｷｮｳﾃﾞ ｵﾀﾉｼﾐｸﾀﾞｻｲ｡", ""}},
	{"luckynum", 0, "", {"ﾗｯｷｰﾅﾝﾊﾞｰ ﾄ ﾗｯｷｰｶﾗｰ ｦ ﾋｮｳｼﾞ  ", "ｼﾃｸﾚﾙ ｱﾌﾟﾘﾃﾞｽ｡ ｵﾃﾞｶｹﾏｴ ﾆ ﾁｪｯｸ", "ｼﾃﾐﾃ ｸﾀﾞｻｲ｡                  ", "", ""}},
	{"mmlplay ", 1, "mmlplay fujisan.mml", {"OSｼﾞｻｸﾆｭｳﾓﾝ 30ﾆﾁﾒﾖﾘ mmlﾌﾟﾚｰﾔｰ", "ﾃﾞｽ｡ ﾋﾞｰﾌﾟ ｵﾝ ﾉ ｼﾝﾌﾟﾙ ﾃﾞ ｿﾎﾞｸ", "ﾅ ﾒﾛﾃﾞｨｰ ｦｵﾀﾉｼﾐ ｸﾀﾞｻｲ･･････｡ ", "", ""}},
	{"mtorz   ", 0, "", {"orz ｶﾞ 10ﾆﾝ ﾔﾏ ｦ ﾂｸｯﾃ ｲﾏｽ｡   ", "ｼｶﾓ､ ﾅﾅｲﾛ ﾆ ﾋﾟｶﾋﾟｶ ｶｶﾞﾔｷ ﾏｽ｡ ", "ｱｷﾀﾗ ｼｭｳﾘｮｳ ｻｾﾃｸﾀﾞｻｲ｡        ", "", ""}},
	{"noodle  ", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 24ﾆﾁﾒﾖﾘ ﾗｰﾒﾝﾀｲﾏｰ ", "ﾃﾞｽ｡ ｱﾗ? ｿｺﾆ ﾁｮｳﾄﾞ ｶｯﾌﾟﾒﾝ ｶﾞ ", "ｱﾘﾏｽﾈ｡ ｼﾞｬｱ ﾁｮｯﾄ ｺﾉ ﾀｲﾏｰ ﾃﾞ  ", "ﾂｸｯﾃ ﾐﾏｼｮｳﾖ?                 ", ""}},
	{"stbeep  ", 0, "", {"ｻｯﾎﾟﾛｼｴｲﾁｶﾃﾂ ﾄｳﾎｳｾﾝ ﾉ ﾄﾞｱ ｶﾞ ", "ﾄｼﾞﾙﾄｷ ﾉ ｵﾄ ｦ ｻｲｹﾞﾝｼﾀ ｱﾌﾟﾘﾃﾞｽ", "｡ ﾋﾞｰﾌﾟﾃﾞﾉ ｻｲｹﾞﾝ ﾃﾞﾊ､ ｹﾞﾝｶｲｶﾞ", "ｱﾙﾖｳﾃﾞｽ｡ ｼﾞｯｷﾃﾞﾉ ｷﾄﾞｳｦ ｽｲｼｮｳ｡", "ｶﾞｿﾞｳ ﾖﾐｺﾐ API ﾉ ｻﾝﾌﾟﾙ｡      "}},
	{"osakcnt ", 0, "", {"OSAkkie ｶﾞ ｷﾄﾞｳ ｼﾃｶﾗﾉ ｼﾞｶﾝ ｦ ", "ﾋｮｳｼﾞ ｽﾙ ｱﾌﾟﾘ ﾃﾞｽ｡           ", "tomo_gettick(); ﾉ ｻﾝﾌﾟﾙ ｱﾌﾟﾘ｡", "", ""}},
	{"tview   ", 1, "tview sample.txt -w42", {"OSｼﾞｻｸﾆｭｳﾓﾝ 30ﾆﾁﾒﾖﾘ ﾃｷｽﾄﾋﾞｭｰｱ", "ﾃﾞｽ｡ ｻﾝﾌﾟﾙ ﾄ ｼﾃ ﾆﾎﾝｺｸ ｹﾝﾎﾟｳ  ", "ｾﾞﾝﾌﾞﾝ ｦ ﾖｳｲ ｼﾏｼﾀ｡           ", "", ""}},
	{"walk    ", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 23ﾆﾁﾒﾖﾘ ｷｰ ﾆｭｳﾘｮｸ", "ﾃｽﾄ ｱﾌﾟﾘ ﾃﾞｽ｡ ｴﾓｼﾞ ｦ ﾄｳｻｲ ｼﾃﾙ", "OSAkkie ﾃﾞﾊ､ ﾅｵﾐｻﾝ ｶﾞ ｱﾙｷﾏｽ｡ ", "", ""}},
	{"winhello", 0, "", {"OSｼﾞｻｸﾆｭｳﾓﾝ 22ﾆﾁﾒﾖﾘ ｳｨﾝﾄﾞｳ   ", "ﾋｮｳｼﾞ ﾃｽﾄ ﾃﾞｽ｡ OSAkkie ﾐﾆﾌｫﾝﾄ", "ﾉ ﾋｮｳｼﾞ ﾃｽﾄ ﾓ ﾔｯﾃ ﾐﾏｼﾀ｡      ", "", ""}}
};

void putstr(int px, int py, int l, char *s);
void putline(int spx, int spy, int epx, int epy, int c);
void putcursor(int py, int base, int c);
int  putitems(int base);
void wait(int time);

void HariMain(void)
{
	char buf[400 * 300 * osak_getbuflen()], s[2];
	static unsigned char title[] = "OSAkkie application launcher";
	int i, cursor = 0, cur_max, page_base = 0, page_max;

	tomo_setlang(0);
	win = api_openwin(buf, 400, 300, -1, "Welcome to OSAkkie.");
	api_boxfilwin(win,  6, 28, 392, 292, 7);
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	
	wait(10);
	for (i = 0; i < 3; i++) {
		wait(3);
		osak_putministrwin(win, 12, 40, 0xffffff, 37, title);
		wait(3);
		osak_putministrwin(win, 12, 40, 0x000000, 37, title);
	}
	wait(10);
	putline( 0,  25, 386,  25, 0);
	wait(10);
	putline( 0, 251,  90, 251, 0);
	wait(10);
	putline(90,  25,  90, 264, 0);

	page_max = APPS / 15;
	if (APPS % 15) {
		page_max++;
	}
	s[0] = '0' + page_max;
	s[1] = 0;
	putstr(1, 21, 14, "<<    /    >>");
	putstr(9, 21, 1, s);

	cur_max = putitems(page_base);

	for (;;) {
		i = api_getkey(1);
		if (i == 0xb8) {
			/* up */
			putcursor(cursor, page_base, 0xffffff);
			cursor--;
			if (cursor < 0) {
				cursor = cur_max - 1;
			}
			putcursor(cursor, page_base, 0x000000);
		}
		if (i == 0xb2) {
			/* down */
			putcursor(cursor, page_base, 0xffffff);
			cursor++;
			if (cur_max <= cursor) {
				cursor = 0;
			}
			putcursor(cursor, page_base, 0x000000);
		}
		if (i == 0xb4) {
			/* left */
			if (9 <= page_base) {
				page_base -= 15;
			} else {
				page_base = (page_max - 1) * 15;
			}
			cursor = 0;
			cur_max = putitems(page_base);
		}
		if (i == 0xb6) {
			/* right */
			if (page_base + cur_max < APPS) {
				page_base += 15;
			} else {
				page_base = 0;
			}
			cursor = 0;
			cur_max = putitems(page_base);
		}
		if (i == 0x0a) {
			if (list[page_base + cursor].arg == 0) {
				osak_exec(list[page_base + cursor].filename);
			} else {
				osak_exec(list[page_base + cursor].filename2);
			}
		}
	}
	api_closewin(win);
	api_end();
}

void putstr(int px, int py, int l, char *s)
{
	api_boxfilwin(win, 6 + px * 6, 28 + py * 12, 12 + px * 6 + 1, 28 + (py + 1) * 12, 7);
	osak_putministrwin(win, 6 + px * 6, 28 + py * 12, 0x000000, l, s);
	return;
}

void putline(int spx, int spy, int epx, int epy, int c)
{
	osak_linewin(win, 6 + spx, 28 + spy, 6 + epx, 28 + epy, c);
	return;
}

void putcursor(int py, int base, int c)
{
	int i;
	/* カーソル */
	py += 3;
	putline(15, py * 12 + 6, 15, py * 12 + 6, c);
	putline(14, py * 12 + 5, 14, py * 12 + 7, c);
	putline(13, py * 12 + 4, 13, py * 12 + 8, c);
	putline(12, py * 12 + 3, 12, py * 12 + 9, c);
	if (c == 0) {
		/* 説明 */
		py -= 3;
		api_boxfilwin(win, 108, 88, 281, 148, 7);
		for (i = 0; i < 5; i++) {
			if (list[py + base].desc[i][0] == 0) {
				break;
			}
			putstr(17,  i + 5, 29, list[py + base].desc[i]);
			wait(2);
		}
	}
	return;
}

int putitems(int base)
{
	int i;
	char s[2];
	s[0] = '0' + base / 15 + 1;
	s[1] = 0;
	putstr(5, 21, 1, s);
	api_boxfilwin(win, 108, 64, 280, 148, 7);
	for (i = 0; i < 15; i++) {
		api_boxfilwin(win,  18, 64 + i * 12,  72, 77 + i * 12, 7);
		wait(2);
	}
	for (i = 0; i < 15; i++) {
		if (APPS <= i + base) {
			break;
		}
		putstr(3, i + 3, 8, list[i + base].filename);
		wait(2);
	}
	putstr(17,  3, 13, "Description:");
	putcursor(0, base, 0x000000);
	return i;
}

void wait(int time)
{
	api_settimer(timer, time);
	while (api_getkey(1) != 128);
	return;
}
