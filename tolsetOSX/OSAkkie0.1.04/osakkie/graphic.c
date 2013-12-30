/* グラフィック処理関係 */

#include "bootpack.h"

static unsigned char table_rgb[16 * 3] = {
	0x00, 0x00, 0x00,	/*  0:黒 */
	0xff, 0x00, 0x00,	/*  1:明るい赤 */
	0x00, 0xff, 0x00,	/*  2:明るい緑 */
	0xff, 0xff, 0x00,	/*  3:明るい黄色 */
	0x00, 0x00, 0xff,	/*  4:明るい青 */
	0xff, 0x00, 0xff,	/*  5:明るい紫 */
	0x00, 0xff, 0xff,	/*  6:明るい水色 */
	0xff, 0xff, 0xff,	/*  7:白 */
	0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
	0x84, 0x00, 0x00,	/*  9:暗い赤 */
	0x00, 0x84, 0x00,	/* 10:暗い緑 */
	0x84, 0x84, 0x00,	/* 11:暗い黄色 */
	0x00, 0x00, 0x84,	/* 12:暗い青 */
	0x84, 0x00, 0x84,	/* 13:暗い紫 */
	0x00, 0x84, 0x84,	/* 14:暗い水色 */
	0x84, 0x84, 0x84,	/* 15:暗い灰色 */
};
static unsigned char table_rgb_osakkie[2 * 3] = {
	/* OSAkkie Only */
	0x3a, 0x6e, 0xa5,	/* 240:WinXPな背景色 */
	0x00, 0x80, 0xff	/* 241:タイトルバー */
};

void init_palette(void)
{
	unsigned char table_rgb2[216 * 3];

	if (get_bpp() != 8) {
		/* 8bitカラーじゃないやつは帰ってもらう */
		return;
	}

	int r, g, b, i;
	for (b = 0; b < 6; b++) {
		for (g = 0; g < 6; g++) {
			for (r = 0; r < 6; r++) {
				i = (r + g * 6 + b* 36) * 3;
				table_rgb2[i    ] = r * 51;
				table_rgb2[i + 1] = g * 51;
				table_rgb2[i + 2] = b * 51;
			}
		}
	}
	set_palette(0, 15, table_rgb);
	set_palette(16, 231, table_rgb2);
	set_palette(240, 241, table_rgb_osakkie);
	return;

	/* static char 命令は、データにしか使えないけどDB命令相当 */
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 割り込み許可フラグの値を記録する */
	io_cli(); 					/* 許可フラグを0にして割り込み禁止にする */
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 割り込み許可フラグを元に戻す */
	return;
}

int get_bpp(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	return binfo->vmode;
}

int get_color(int bpp, int col)
{
	/* 0x??????から各種ビットカラーコードに変換する */
	/* この関数は最下層の関数でのみ使われるべき */
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int i = 0, r, g, b;
	static unsigned int pallet[16] = {
		0x000000, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff, 
		0xc6c6c6, 0x840000, 0x008400, 0x848400, 0x000084, 0x840084, 0x008484, 0x848484
	};
	if (bpp == 0) {
		bpp = binfo->vmode;
	}
	if (bpp == 1) {
		/* 8bitパレット番号を0x??????形式に変換 */
		if (0 <= col && col <= 15) {
			/* 16色 */
			for (i = 0; i < 16; i++) {
				if (col == i) {
					i = pallet[i];
					break;
				}
			}
		} else if (16 <= col && col <= 231) {
			/* 216色 */
			for (b = 0; b < 6; b++) {
				for (g = 0; g < 6; g++) {
					for (r = 0; r < 6; r++) {
						if ((16 + r + g * 6 + b* 36) == col) {
							i = (r << 16) | (g << 8) | b;
							goto skip;
						}
					}
				}
			}
		} else {
			/* OSAkkieカラー */
			if (col == 240) { i = 0x3a6ea5; } else
			if (col == 241) { i = 0x0080ff; }
		}
	} else if (bpp == 8) {
		for (i = 0; i < 16; i++) {
			if (col == pallet[i]) { goto skip; }
		}
		/* OSAkkieカラー */
		if (col == 0x3a6ea5) { i = 240; } else
		if (col == 0x0080ff) { i = 241; }
		else {
			/* 216色(近い色が選ばれる) */
			i = ((col >> 16) + ((col >> 8) & 0xff) + (col & 0xff)) / 51;
		}
	} else if (bpp == 16) {
		i = (((col >> 16) << 8) & 0xf800) | ((((col >> 8) & 0xff) << 3) & 0x07e0) | ((col & 0xff) >> 3);
	}
skip:
	return i;
}


/* 8bit, 16bit自動判別関数 ------------------------------------------ */

void boxfill(unsigned int *vram, int xsize, int c, int x0, int y0, int x1, int y1)
{
	int bpp, col, x, y;
	unsigned char  *cp = (unsigned char  *) vram;
	unsigned short *sp = (unsigned short *) vram;
	bpp = get_bpp();
	col = get_color(bpp, c);

	if (bpp == 8) {
		for (y = y0; y <= y1; y++) {
			for (x = x0; x <= x1; x++)
				cp[y * xsize + x] = col;
		}
	} else if (bpp == 16) {
		for (y = y0; y <= y1; y++) {
			for (x = x0; x <= x1; x++)
				sp[y * xsize + x] = col;
		}
	} else if (bpp == 24) {
		for (y = y0; y <= y1; y++) {
			for (x = x0; x <= x1; x++)
				vram[y * xsize + x] = col;
		}
	}
	return;
}

void init_screen(unsigned int *vram, int x, int y)
{
	boxfill(vram, x, 0x3a6ea5, 0, 0, x - 1, y - 1);	// デスクトップ背景
	return;
}

void putfont(unsigned int *vram, int xsize, int x, int y, int c, char *font)
{
	char d;	/* data */
	int col, bpp, i;
	unsigned char  *cp;
	unsigned short *sp;
	unsigned int   *ip;
	bpp = get_bpp();
	col = get_color(bpp, c);
	if (bpp == 8) {
		for (i = 0; i < 16; i++) {
			cp = (unsigned char *) vram + (y + i) * xsize + x;
			d = font[i];
			if ((d & 0x80) != 0) { cp[0] = col; }
			if ((d & 0x40) != 0) { cp[1] = col; }
			if ((d & 0x20) != 0) { cp[2] = col; }
			if ((d & 0x10) != 0) { cp[3] = col; }
			if ((d & 0x08) != 0) { cp[4] = col; }
			if ((d & 0x04) != 0) { cp[5] = col; }
			if ((d & 0x02) != 0) { cp[6] = col; }
			if ((d & 0x01) != 0) { cp[7] = col; }
		}
	} else if (bpp == 16) {
		for (i = 0; i < 16; i++) {
			sp = (unsigned short *) vram + (y + i) * xsize + x;
			d = font[i];
			if ((d & 0x80) != 0) { sp[0] = col; }
			if ((d & 0x40) != 0) { sp[1] = col; }
			if ((d & 0x20) != 0) { sp[2] = col; }
			if ((d & 0x10) != 0) { sp[3] = col; }
			if ((d & 0x08) != 0) { sp[4] = col; }
			if ((d & 0x04) != 0) { sp[5] = col; }
			if ((d & 0x02) != 0) { sp[6] = col; }
			if ((d & 0x01) != 0) { sp[7] = col; }
		}
	} else if (bpp == 24) {
		for (i = 0; i < 16; i++) {
			ip = vram + (y + i) * xsize + x;
			d = font[i];
			if ((d & 0x80) != 0) { ip[0] = col; }
			if ((d & 0x40) != 0) { ip[1] = col; }
			if ((d & 0x20) != 0) { ip[2] = col; }
			if ((d & 0x10) != 0) { ip[3] = col; }
			if ((d & 0x08) != 0) { ip[4] = col; }
			if ((d & 0x04) != 0) { ip[5] = col; }
			if ((d & 0x02) != 0) { ip[6] = col; }
			if ((d & 0x01) != 0) { ip[7] = col; }
		}
	}
	return;
}

void putfonts(unsigned int *vram, int xsize, int x, int y, int c, unsigned char *s)
{
	extern char hankaku[4096];
	struct TASK *task = task_now();
	char *nihongo = (char *) *((int *) 0x0fe8), *font;
	int k, t;

	if (task->langmode == 0) {
		for (; *s != 0x00; s++) {
			putfont(vram, xsize, x, y, c, hankaku + *s * 16);
			x += 8;
		}
	}
	if (task->langmode == 1) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				/* 1バイト目 */
				if ((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc)) {
					/* 2バイト目がある（全角） */
					task->langbyte1 = *s;
				} else {
					 /* 半角文字 */
					putfont(vram, xsize, x, y, c, nihongo + *s * 16);
				}
			} else {
				/* 2バイト目 */
				if (0x81 <= task->langbyte1 && task->langbyte1 <= 0x9f) {
					k = (task->langbyte1 - 0x81) * 2;
				} else {
					k = (task->langbyte1 - 0xe0) * 2 + 62;
				}
				if (0x40 <= *s && *s <= 0x7e) {
					t = *s - 0x40;		/* 小さいほうの区(0x40～0x7e) */
				} else if (0x80 <= *s && *s <= 0x9e) {
					t = *s - 0x80 + 63;	/* 小さいほうの区(0x80～0x9e) */
				} else {
					t = *s - 0x9f;		/* 大きいほうの区(0x9f～0xfc) */
					k++;
				}
				task->langbyte1 = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont(vram, xsize, x - 8, y, c, font     );	/* 左側 */
				putfont(vram, xsize, x    , y, c, font + 16);	/* 右側 */
			}
			x += 8;
		}
	}
	if (task->langmode == 2) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				/* 1バイト目 */
				if (0x81 <= *s && *s <= 0xfe) {
					task->langbyte1 = *s;	/* 2バイト目がある（全角） */
				} else {
					putfont(vram, xsize, x, y, c, nihongo + *s * 16);	 /* 半角文字 */
				}
			} else {
				/* 2バイト目 */
				k = task->langbyte1 - 0xa1;
				t = *s - 0xa1;
				task->langbyte1 = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont(vram, xsize, x - 8, y, c, font     );	/* 左側 */
				putfont(vram, xsize, x    , y, c, font + 16);	/* 右側 */
			}
			x += 8;
		}
	}
	return;
}

void init_mouse_cursor(unsigned int *mouse, int bc)
{
	static unsigned char data[16][16] = {
		"ah.....hahahah..",
		"hah....haaaaah..",
		".hah..hahhhhhah.",
		"..hahhahaahaahah",
		"...haaahahhahhah",
		"....hhaaahahhhah",
		"......hahaaahah.",
		"......hhahahah..",
		".....hahahhhahah",
		"....hahaahhhaaha",
		".....hahahhhahah",
		"......haahhhaah.",
		".....haaaaaaaaah",
		".....hahahahahah",
		"......hahahahah.",
		".......haaaaah.."
	};
	picdata(mouse, 16, 0, 0, &data[0][0], 16, 16, bc);
	return;
}

void putminifont(unsigned int *vram, int xsize, int x, int y, int c, char *font)
{
	char d;	/* data */
	int col, bpp, i;
	unsigned char  *cp;
	unsigned short *sp;
	unsigned int   *ip;
	bpp = get_bpp();
	col = get_color(bpp, c);
	if (bpp == 8) {
		for (i = 0; i < 12; i++) {
			cp = (unsigned char *) vram + (y + i) * xsize + x;
			d = font[i];
			if ((d & 0x20) != 0) { cp[0] = col; }
			if ((d & 0x10) != 0) { cp[1] = col; }
			if ((d & 0x08) != 0) { cp[2] = col; }
			if ((d & 0x04) != 0) { cp[3] = col; }
			if ((d & 0x02) != 0) { cp[4] = col; }
			if ((d & 0x01) != 0) { cp[5] = col; }
		}
	} else if (bpp == 16) {
		for (i = 0; i < 12; i++) {
			sp = (unsigned short *) vram + (y + i) * xsize + x;
			d = font[i];
			if ((d & 0x20) != 0) { sp[0] = col; }
			if ((d & 0x10) != 0) { sp[1] = col; }
			if ((d & 0x08) != 0) { sp[2] = col; }
			if ((d & 0x04) != 0) { sp[3] = col; }
			if ((d & 0x02) != 0) { sp[4] = col; }
			if ((d & 0x01) != 0) { sp[5] = col; }
		}
	} else if (bpp == 24) {
		for (i = 0; i < 12; i++) {
			ip = vram + (y + i) * xsize + x;
			d = font[i];
			if ((d & 0x20) != 0) { ip[0] = col; }
			if ((d & 0x10) != 0) { ip[1] = col; }
			if ((d & 0x08) != 0) { ip[2] = col; }
			if ((d & 0x04) != 0) { ip[3] = col; }
			if ((d & 0x02) != 0) { ip[4] = col; }
			if ((d & 0x01) != 0) { ip[5] = col; }
		}
	} else 
	return;
}

void putminifonts(unsigned int *vram, int xsize, int x, int y, int c, unsigned char *s)
{
	extern char minifnt[4096];
	for (; *s != 0x00; s++) {
		putminifont(vram, xsize, x, y, c, minifnt + *s * 12);
		x += 6;
	}
	return;
}


void put_naomi(unsigned int *vram)
{
	static unsigned char naomi[26][28] = {
		"ooooooooooooaaaaoooooooooooo",
		"oooooooooaaaddddaaaooooooooo",
		"ooooooooaddddddddddaoooooooo",
		"oooooooaddddddddddddaooooooo",
		"ooooooaddddddddddddddaoooooo",
		"aooooadaaaaaaddaaaaaadaooooa",
		"aoooaddaddddaddaddddaddaoooa",
		"aoooaddaaaaaaddaaaaaaddaoooa",
		"oaooaddaddddaddaddddaddaoooa",
		"oaoadddaaaaaaddaaaaaadddaoao",
		"oaoaddddddddddddddddddddaoao",
		"ooaadddddaaddddddaadddddaaoo",
		"oooadddddaaddddddaadddddaooo",
		"oooadddddddddaadddddddddaooo",		/*   ____________________ */
		"ooooaddddddaabbaaddddddaoooo",		/*  |                   | */
		"ooooaddddddabaabaddddddaoooo",		/*  | どうも、          | */
		"ooooadddddabahhabadddddaoooo",		/* <                    | */
		"oooooaddddabahhabaddddaooooo",		/*  |   ナオミさんです。| */
		"ooooooaddddabaabaddddaoooooo",		/*  |___________________| */
		"oooooooadddaabbaadddaooooooo",
		"ooooooooaddddaaddddaoooooooo",
		"oooooooooaaaddddaaaooooooooo",
		"ooooooooaaooaaaaooaaoooooooo",
		"oooooooaooooooooooooaooooooo",
		"ooooooaooooooooooooooaoooooo",
		"oooooaooooooooooooooooaooooo",
	};
	picdata(vram, 28, 0, 0, &naomi[0][0], 28, 26, -1);
	return;
}

void picdata(unsigned int *vram, int xsize, int px, int py, unsigned char *data, int sx, int sy, int bc)
{
	int i = 0, x, y, bpp = get_bpp();
	unsigned char  *cp, c;
	unsigned short *sp;
	unsigned int   *ip;
	if (bpp == 8) {
		for (y = 0; y < sy; y++) {
			cp = (unsigned char *) vram + (py + y) * xsize + px;
			for (x = 0; x < sx; x++) {
				c = data[y * sx + x];
				if ('a' <= c && c <= 'p') {
					/* 0～15 */
					cp[x] = c - 0x61;
				} else if ('q' <= c && c <= 'r') {
					/* OSAkkieカラー */
					cp[x] = c - 0x61 + 224;
				} else if (c == '.') {
					/* 透過色 */
					cp[x] = bc;
				}
			}
		}
	} else if (bpp == 16) {
		for (y = 0; y < sy; y++) {
			sp = (unsigned short *) vram + (py + y) * xsize + px;
			for (x = 0; x < sx; x++) {
				c = data[y * sx + x];
				if ('a' <= c && c <= 'p') {
					/* 0～15 */
					i = get_color(1, c - 0x61);
					sp[x] = get_color(bpp, i);
				} else if ('r' <= c && c <= 's') {
					/* OSAkkieカラー */
					i = get_color(1, c + 0x71);
					sp[x] = get_color(bpp, i);
				} else if (c == '.') {
					/* 透過色 */
					sp[x] = bc;
				}
			}
		}
	} else if (bpp == 24) {
		for (y = 0; y < sy; y++) {
			ip = vram + (py + y) * xsize + px;
			for (x = 0; x < sx; x++) {
				c = data[y * sx + x];
				if ('a' <= c && c <= 'p') {
					/* 0～15 */
					i = get_color(1, c - 0x61);
					ip[x] = get_color(bpp, i);
				} else if ('r' <= c && c <= 's') {
					/* OSAkkieカラー */
					i = get_color(1, c + 0x71);
					ip[x] = get_color(bpp, i);
				} else if (c == '.') {
					/* 透過色 */
					ip[x] = bc;
				}
			}
		}
	}
	return;
}
