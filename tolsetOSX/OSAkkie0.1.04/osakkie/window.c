/* Window関係 */

#include "bootpack.h"

void make_window(unsigned int *buf, int xsize, int ysize, char *title, int icon, char act)
{
	boxfill(buf, xsize, 0xc6c6c6, 0      , 0      , xsize-2, 0      );	// 上薄灰色
	boxfill(buf, xsize, 0xc6c6c6, 0      , 0      , 0,       ysize-2);	// 左薄灰色
	boxfill(buf, xsize, 0xFFFFFF, 1      , 1      , xsize-2, 1      );	// 上白色
	boxfill(buf, xsize, 0xFFFFFF, 1      , 1      , 1,       ysize-2);	// 左白色
	boxfill(buf, xsize, 0xc6c6c6, 2      , 2      , xsize-3, ysize-3);	// 薄灰色（ウィンドウ本体）
	boxfill(buf, xsize, 0x848484, xsize-2, 1      , xsize-2, ysize-2);	// 右濃灰色
	boxfill(buf, xsize, 0x848484, 1      , ysize-2, xsize-2, ysize-2);	// 下濃灰色
	boxfill(buf, xsize, 0x000000, xsize-1, 0      , xsize-1, ysize-1);	// 右黒色
	boxfill(buf, xsize, 0x000000, 0      , ysize-1, xsize-1, ysize-1);	// 下黒色
	make_wtitle(buf, xsize, title, icon, act);
	return;
}

void make_wtitle(unsigned int *buf, int xsize, char *title, int icon, char act)
{
	static unsigned char btn[14][31] = {
		"hhhhhhhhhhhhhhh hhhhhhhhhhhhhhh",
		"h             h h             h",
		"h             h h             h",
		"h             h h             h",
		"h             h h   hh   hh   h",
		"h             h h    hh hh    h",
		"h             h h     hhh     h",
		"h             h h     hhh     h",
		"h     hhh     h h    hh hh    h",
		"h   hhhhhhh   h h   hh   hh   h",
		"h  hh     hh  h h             h",
		"h hh       hh h h             h",
		"h             h h             h",
		"hhhhhhhhhhhhhhh hhhhhhhhhhhhhhh",
	};
	struct TASK *task = task_now();
	int tc, tbc, oldlang;
	unsigned char logo[3];

	if (act) {
		tc  = 0xffffff;
		tbc = 0x0080ff;
	} else {
		tc  = 0xc6c6c6;
		tbc = 0x848484;
	}
	boxfill(buf, xsize, tbc, 3, 3, xsize-4, 20);	// タイトルバー
	logo[0] = 0x80 + icon * 2;
	logo[1] = 0x81 + icon * 2;
	logo[2] = 0;

	/* 一度langmodeを0にして絵文字描画 */
	oldlang = task->langmode;
	task->langmode = 0;
	putfonts(buf, xsize,  8, 4, tc, logo);
	task->langmode = oldlang;
	putfonts(buf, xsize, 24, 4, tc, title);
	
	picdata(buf, xsize, xsize - 36, 5, &btn[0][0], 31, 14, -1);
	return;
}

void make_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	int *p = (unsigned int *) (sht->buf);
	boxfill(p, sht->bxsize, 0x848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill(p, sht->bxsize, 0x848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill(p, sht->bxsize, 0xffffff, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill(p, sht->bxsize, 0xffffff, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill(p, sht->bxsize, 0x000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill(p, sht->bxsize, 0x000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill(p, sht->bxsize, 0xc6c6c6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill(p, sht->bxsize, 0xc6c6c6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill(p, sht->bxsize, c,        x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void putfonts_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	struct TASK *task = task_now();
	int *p = (unsigned int *) (sht->buf);
	boxfill(p, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts(p, sht->bxsize, x, y, c, s);
	if (task->langmode != 0 && task->langbyte1 != 0) {
		sheet_refresh(sht, x - 8, y, x + l * 8, y + 16);
	} else {
		sheet_refresh(sht, x, y, x + l * 8, y + 16);
	}
	return;
}

void change_wtitle(struct SHEET *sht, int act)
{
	int x, y, xsize = sht->bxsize;
	int c, tc[2], tbc[2];
	int bpp = get_bpp();
	unsigned short *sbuf = (unsigned short *) (sht->buf);
	unsigned int   *ibuf = (unsigned int   *) (sht->buf);

	tc[0]  = get_color(bpp, 0xc6c6c6);	// 非アクティブ
	tbc[0] = get_color(bpp, 0x848484);
	tc[1]  = get_color(bpp, 0xffffff);	// アクティブ
	tbc[1] = get_color(bpp, 0x0080ff);

	if (bpp == 8) {
		for (y = 3; y < 21; y++) {
			for (x = 3; x < xsize - 3; x++) {
				c = sht->buf[y * xsize + x];
				if (c == tc[1 - act]) {
					c = tc[act];
				} else if (c == tbc[1 - act]) {
					c = tbc[act];
				}
				sht->buf[y * xsize + x] = c;
			}
		}
	} else if (bpp == 16) {
		for (y = 3; y < 21; y++) {
			for (x = 3; x < xsize - 3; x++) {
				c = sbuf[y * xsize + x];
				if (c == tc[1 - act]) {
					c = tc[act];
				} else if (c == tbc[1 - act]) {
					c = tbc[act];
				}
				sbuf[y * xsize + x] = c;
			}
		}
	} else if (bpp == 24) {
		for (y = 3; y < 21; y++) {
			for (x = 3; x < xsize - 3; x++) {
				c = ibuf[y * xsize + x];
				if (c == tc[1 - act]) {
					c = tc[act];
				} else if (c == tbc[1 - act]) {
					c = tbc[act];
				}
				ibuf[y * xsize + x] = c;
			}
		}
	}
	sheet_refresh(sht, 3, 3, xsize, 21);
	return;
}

struct BALLOON *make_balloon(struct MEMMAN *memman, struct SHEET *sht,
								int px, int py, int smode, int sx, int sy)
{
	static unsigned char map[5][5];
	static unsigned char corner[5][5] = {
		"...aa",
		"..ahh",
		".ahhh",
		"ahhhh",
		"ahhhh"
	};
	static unsigned char fukidashi[7][9] = {
		"hhhhhaaaa",
		"aahhha...",
		"..ahhha..",
		"...aaha..",
		".....aha.",
		"......aa.",
		"........a",
	};
	struct BALLOON *balloon = (struct BALLOON *) memman_alloc_4k(memman, sizeof (struct BALLOON));
	unsigned short *sp = (unsigned short *) (sht->buf);
	unsigned int   *ip = (unsigned int   *) (sht->buf);
	int bpp = get_bpp();
	int i, x, y;
	int psx = sx * smode + 10, psy = sy * smode + 10;
	struct POS {
		int x, y;
	} pos[4] = {
		{px, py}, {px+psx-5, 0}, {px+psx-5, py+psy-4}, {0, py+psy-4}
	};

	balloon->sht   = sht;
	balloon->px    = px;
	balloon->py    = py;
	balloon->smode = smode;
	balloon->sx    = sx;
	balloon->sy    = sy;

	if (bpp == 8) {
		for (y = py; y < py+psy; y++) {
			for (x = px; x < px+psx; x++) {
				sht->buf[(py+psy+y) * sht->bxsize + x] = sht->col_inv;
			}
		}
	} else if (bpp == 16) {
		for (y = 0; y < 7; y++) {
			for (x = 0; x < px+psx; x++) {
				sp[(py+psy+y) * sht->bxsize + x] = sht->col_inv;
			}
		}
	} else if (bpp == 24) {
		for (y = py; y < py+psy; y++) {
			for (x = px; x < px+psx; x++) {
				ip[(py+psy+y) * sht->bxsize + x] = sht->col_inv;
			}
		}
	}
	boxfill(ip, sht->bxsize, 0x000000, px  , py,   px+psx-1, py+psy  );	// 黒色全体
	boxfill(ip, sht->bxsize, 0xffffff, px+1, py+1, px+psx-2, py+psy-1);	// 白色内側
	/* コーナーの描画(四隅) */
	for (i = 0; i < 4; i++) {
		picdata(ip, sht->bxsize, pos[i].x, pos[i].y, &corner[0][0], 5, 5, sht->col_inv);
		for (y = 0; y <= 4 ; y++) {
			for (x = 0; x <= 4; x++) {
				map[y][x] = corner[y][x];	/* mapにコピー */
			}
		}
		for (y = 0; y <= 4; y++) {
			for (x = 0; x <= 4; x++) {
				corner[y][x] = map[4-x][y];	/* 回転 */
			}
		}
	}
	/* 吹き出し */
	picdata(ip, sht->bxsize, sht->bxsize - 40, psy, &fukidashi[0][0], 9, 7, sht->col_inv);
	return balloon;
}

void putminifonts_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	unsigned int *p = (unsigned int *) (sht->buf);
	boxfill(p, sht->bxsize, b, x, y, x + l * 6 - 1, y + 11);
	putminifonts(p, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 6, y + 12);
	return;
}


void make_omnaomi(struct SHEET *sht, int px, int py)
{
	unsigned char omu[16][31] = {
		"...............aaaaaaa.........",
		".........aaaaaadddddadaaaa.....",
		"......aaaaddddddddddaddddaa....",
		".....aaaddddddddddddaaaaaaaa...",
		"...aaddaadddddddddddaadddddaa..",
		"..aaaaaaaddddaaaaadddaaaaaada..",
		".aaadddaadddaabbbaddddddddddaa.",
		".adaaaaaddddabaaabadddddddddda.",
		"adddddddddddaba.abadddddddadda.",
		"adddddddddddabaaabaddddddddddda",
		"adadddddddddaabbbaaddddddddddda",
		"addddddddddddaaaaaaaaddddddddda",
		"addddddddddaaa......aaaddddddda",
		"addddddddaaa..........aaddddda.",
		".addddddaa.............aaaaaa..",
		".aaaaaaaa......................",
	};
	unsigned int *p = (unsigned int *) (sht->buf);
	picdata(p, sht->bxsize, px, py, &omu[0][0], 31, 16, sht->col_inv);
	return;
}
