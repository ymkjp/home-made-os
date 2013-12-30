/* ---------------------------------------------------------------- */
/*  CSVV :: CSV-Viewer  for OSAkkie                                 */
/*          Ver.0.1(2006/11/18)   (c) 2006 Akkiesoft.               */
/*   TAB = 4      Stack: 640k  Malloc: 0k                           */
/* ---------------------------------------------------------------- */

#include "apilib.h"
#include <stdio.h>

// 設定項目
#define MAX_FILESIZ		200 * 1024	// CSVファイルは200KBまで
#define MAX_COMMA_X		201			// 対応できるx方向のセル数(コンマ単位. 676コぐらいが限度)
#define MAX_COMMA_Y		201			// 対応できるx方向のセル数(コンマ単位)
#define CELLSIZ_X		82			// x方向のセル1つのサイズ
#define CELLSIZ_Y		18			// y方向のセル1つのサイズ


// 固定
#define	AUTO_MALLOC		0
#define REWIND_CODE		1
#define WND_X			(CELLSIZ_X * maxell_x    + 12)		// Windowサイズx
#define WND_Y			(CELLSIZ_Y * maxell_y+16 + 36)		// Windowサイズy
#define GBOX_X			(CELLSIZ_X * maxell_x)
#define GBOX_Y			(CELLSIZ_Y * maxell_y)
#define TBOX_X			(GBOX_X) / 8

struct CSV_INFO {
	int comma[MAX_COMMA_Y][MAX_COMMA_X];
	int max_x;
	int max_y;
	int filesize;
	char buf[MAX_FILESIZ];
};

static int win;
static int maxell_x =  7;	/* 列数 */
static int maxell_y = 16;	/* 行数 */
	/* VGA size: ( 7, 21)
	   Normal  : ( 8, 16) */

int setdec(int i, char *s, int j);
int GetCellData(unsigned char *s, struct CSV_INFO *csvinfo, int i, int l);
void PutCell(int bx, int by, struct CSV_INFO *csvinfo);
void Putcellinfo(int x, int y, struct CSV_INFO *csvinfo);
void my_drawboxId(int c, int idx, int idy);
void putstr(int px, int py, int c, int bc, int l, char *s);

void HariMain()
{
	char buf[WND_X * WND_Y * osak_getbuflen()];
	char cmdline[30], *p = 0;

	struct CSV_INFO csvinfo;
	int sig, cx, cy, basex, basey, i, mode;
	int *p_com1, *p_com2;
	char c;

	/* CSVデータの初期化 */
	p_com2 = &csvinfo.comma[MAX_COMMA_Y - 1][MAX_COMMA_X - 1];
	for (p_com1 = &csvinfo.comma[0][0]; p_com1 <= p_com2; p_com1++)
		*p_com1 = -1;
	csvinfo.max_x = csvinfo.max_y = csvinfo.filesize = 0;

	/* コマンドライン取得 */
	api_cmdline(cmdline, 30);
	for (p = cmdline; *p > ' '; p++);	/* スペースが来るまで読み飛ばす */
	for (; *p == ' '; p++);				/* スペースを読み飛ばす */

	i = api_fopen(p);
	if (i == 0) {
		api_putstr0("File not found.\n");
		api_end();
	}
	csvinfo.filesize = api_fsize(i, 0);
	if (csvinfo.filesize >= MAX_FILESIZ - 1) {
		/* 読めない */
		api_putstr0("Filesize is too big.\n");
		api_end();
	}
	api_fread(csvinfo.buf, csvinfo.filesize, i);
	api_fclose(i);
	p = csvinfo.buf;

	i = cx = cy = mode = 0;
	if (*p != ',')
		csvinfo.comma[0][0] = 0;

	while (i < csvinfo.filesize) {
		c = csvinfo.buf[i++];

		if (c == 0x22) {
			if  (i == 1 || mode == 0) {
				mode++;
				continue;
			}
			if (mode && (csvinfo.buf[i] == ',' || csvinfo.buf[i] == 0x0a || csvinfo.buf[i] == 0x0d))
				mode--;
		}

		if (mode == 0 && c == ',') {
			cx++;
			// 「,,」「,\n」のように、セルに何もない状態ではない
			if (csvinfo.buf[i] != ',' && csvinfo.buf[i] != 0x0a && csvinfo.buf[i] != 0x0d)
				csvinfo.comma[cy][cx] = i;		// コンマの次の文字の場所が記録される
		}

		/* 改行コード */
		if ((c == 0x0d /* CR */ && csvinfo.buf[i] == 0x0a /* LF */) ||
			(c == 0x0a /* LF */ && csvinfo.buf[i] == 0x0d /* CR */)) {
			if (i < csvinfo.filesize) {
				if (csvinfo.max_x < cx)
					csvinfo.max_x = cx;
				cy++;
				cx = 0;
				i++;
				if (i >= csvinfo.filesize)
					break;
				csvinfo.max_y++;
				if (csvinfo.buf[i] != ',' && csvinfo.buf[i] != 0x0a && csvinfo.buf[i] != 0x0d)
					csvinfo.comma[cy][cx] = i;		// コンマの次の文字の場所が記録される
				if (csvinfo.buf[i] == 0x22)
					mode++;
			}
		}
	}
	csvinfo.max_x++;
	csvinfo.max_y++;

	win = api_openwin(buf, WND_X, WND_Y, -1, "CSV-Viewer");
	api_boxfilwin(win, 6, 26+16, GBOX_X+6, GBOX_Y+26+16, 7);

	/* ワークシート作成 */
	for (i = 0; i <= GBOX_X; i += CELLSIZ_X)
		api_linewin(win, i + 6, 42, i + 6, GBOX_Y + 42, 15);
	for (i = 0; i <= GBOX_Y; i += CELLSIZ_Y)
		api_linewin(win, 6, i + 42, GBOX_X + 6, i + 42, 15);

	/* CSVデータ表示 */
	cx = cy = basex = basey = 1;
	PutCell(1, 1, &csvinfo);
	my_drawboxId(4, cx, cy);
	Putcellinfo(basex+cx-2, basey+cy-2, &csvinfo);

	/* メインループ */
	for (;;) {
		sig = api_getkey(1);
		if (sig == 0xb4) {
			 /* left */
			if (1 < cx) {
				my_drawboxId(15, cx, cy);
				cx--;
				sig = 0;
			} else if (1 < basex) {
				basex--;
				PutCell(basex, basey, &csvinfo);
				sig = 0;
			}
		}
		if (sig == 0xb6) {
			/* rigth */
			if (cx < maxell_x-1) {
				my_drawboxId(15, cx, cy);
				cx++;
				sig = 0;
			} else if (basex+cx < MAX_COMMA_X) {
				basex++;
				PutCell(basex, basey, &csvinfo);
				sig = 0;
			}
		}
		if (sig == 0xb8) {
			/* up */
			if (cy >  1) {
				my_drawboxId(15, cx, cy);
				cy--;
				sig = 0;
			} else if (1 < basey) {
				basey--;
				PutCell(basex, basey, &csvinfo);
				sig = 0;
			}
		}
		if (sig == 0xb2) {
			/* down */
			if (cy <  maxell_y-1) {
				my_drawboxId(15, cx, cy);
				cy++;
				sig = 0;
			} else if (basey+cy < MAX_COMMA_Y) {
				basey++;
				PutCell(basex, basey, &csvinfo);
				sig = 0;
			}
		}
		if (sig == 0xb9) {
			/* Page Up */
			if (basey > 1) {
				basey -= maxell_y - 1;
				if (basey < 1) {
					my_drawboxId(15, cx, cy);
					basey = cy = 1;
				}
				PutCell(basex, basey, &csvinfo);
				sig = 0;
			}
		}
		if (sig == 0xb3) {
			/* Page Down */
			if (basey <= MAX_COMMA_Y - maxell_y) {
				basey += maxell_y - 1;
				if (basey > MAX_COMMA_Y - maxell_y) {
					my_drawboxId(15, cx, cy);
					basey = MAX_COMMA_Y - maxell_y + 1;
					cy = maxell_y - 1;
				}
				PutCell(basex, basey, &csvinfo);
				sig = 0;
			}
		}

		if (sig == 0) {
			my_drawboxId(4, cx, cy);
			Putcellinfo(basex+cx-2, basey+cy-2, &csvinfo);
		}
	}
}

int setdec(int i, char *s, int j/* '\0'を除いた数 */)
/* 数値iをsの右側からj文字分代入する。最後に代入したsの位置-1を返す。 */
{
	do {
		s[j--] = (i % 10) + '0';
		if ((i /= 10) == 0)
			break;
	} while (j >= 0);
	return j;
}

int GetCellData(unsigned char *s, struct CSV_INFO *csvinfo, int i, int l)
/* セルの文字列を取り出す。 */
{
	int j = 0, mode = 0;
	char c;

	if (i < 0)
		return -1;

	if (csvinfo->buf[i] == 0x22) {
		i++;
		mode++;
	}

	for (;;) {
		c = csvinfo->buf[i++];

		/* ファイルの終端に到達 */
		if (i > csvinfo->filesize)
			break;

		/* 改行コードとか、""モードOFFの時のコンマだったら出力終わり */
		if (c == 0x0d /* CR */ || c == 0x0a /* LF */ || (mode == 0 && c == ','))
			break;

		if (mode && c == 0x22 /* " */) {
			if (csvinfo->buf[i] == ',' || csvinfo->buf[i] == 0x0a || csvinfo->buf[i] == 0x0d)
				break;
			if (csvinfo->buf[i] == 0x22 && csvinfo->buf[i+1] != ',' && csvinfo->buf[i+1] != 0x0a && csvinfo->buf[i+1] != 0x0d)
				i++;
		}

		/* 表示文字へ変換 */
		if (j < l) {
			if (c == '\t' /* Tab */) {
				j += 3;	// tab == 4 (if,elseを抜けたあとのj++で4になる)
/*
			} else if (0xa1 <= c && c <= 0xdf) {
				s[j] = c;
			} else if (0x81 <= c && c <= 0xef && i < csvinfo->filesize && 0x40 <= csvinfo->buf[i]) {
				if (j+1 < l) {
					s[j++] = c;
					s[j] = csvinfo->buf[i++];
				}
*/
			} else {
				s[j] = c;
			}
			j++;
		}
	}

	return j;
}

void PutCell(int bx, int by, struct CSV_INFO *csvinfo)
{
	int i, tx = bx, ty = by, y;
	char s[11];
	s[10] = '\0';

	for (i = 0; i < 10; i++) {s[i] = ' ';}
	for (i=1; i<maxell_y;i++) {
		setdec(ty, s, 5);
		ty++;
		putstr(2, CELLSIZ_Y*i+2, 7, 12, 10, s);
	}

	for (i = 0; i < 10; i++) {s[i] = ' ';}
	i = 1;
	for (;;) {
		tx = bx + i - 2;
		if (tx > 25)
			s[4] = 0x40 + (tx / 26);
		s[5] = 0x41 + (tx % 26);
		putstr(CELLSIZ_X*i+2, 2, 7, 12, 10, s);
		i++;
		if (i >= maxell_x)
			break;
	}

	/* Read file */
	for (y = 1; y < maxell_y; y++) {
		int x;
		for (x = 1; x < maxell_x; x++) {
			tx = bx+x-2;	// tx, tyをリサイクル
			ty = by+y-2;

			for (i = 0; i < 10; i++) {s[i] = ' ';}
			putstr(CELLSIZ_X*x+2, CELLSIZ_Y*y+2, 0, 7, 10, s);
			i = GetCellData(s, csvinfo, csvinfo->comma[ty][tx], 10);
			if (i >= 0) {
				putstr(CELLSIZ_X*x+2, CELLSIZ_Y*y+2, 0, 7, 10, s);
			}
		}
	}

	return;
}

void Putcellinfo(int x, int y, struct CSV_INFO *csvinfo)
/* セル番地の表示 */
{
	int i;
	char s[TBOX_X + 1];

	for (i = 0; i < TBOX_X; i++) {s[i] = ' ';}
	s[0] = '(';
	s[9] = ')';
	s[TBOX_X] = '\0';

	i = setdec(y + 1, s, 7);	// これは行番号
	s[i--] = 0x41 + (x % 26);	// ここから列番号
	if (x > 25)
		s[i] = 0x40 + (x / 26);
	api_boxfilwin(win, 6, 26, 6 + TBOX_X * 8, 41, 8);
	api_putstrwin(win, 6, 26, 0, TBOX_X, s);	// 表示1

	i = GetCellData(s, csvinfo, csvinfo->comma[y][x], TBOX_X-12);
	if (i >= 0) {
		s[i] = 0;
		api_putstrwin(win, 86, 26, 0, i, s);	// 表示2
	}
	return;
}


void my_drawboxId(int c, int idx, int idy)
{
	idx *= CELLSIZ_X;
	idy *= CELLSIZ_Y;
	api_linewin(win, idx           + 6, idy           + 42, idx+CELLSIZ_X + 6, idy           + 42, c);
	api_linewin(win, idx           + 6, idy+CELLSIZ_Y + 42, idx+CELLSIZ_X + 6, idy+CELLSIZ_Y + 42, c);
	api_linewin(win, idx           + 6, idy           + 42, idx           + 6, idy+CELLSIZ_Y + 42, c);
	api_linewin(win, idx+CELLSIZ_X + 6, idy           + 42, idx+CELLSIZ_X + 6, idy+CELLSIZ_Y + 42, c);
	return;
}

void putstr(int px, int py, int c, int bc, int l, char *s)
{
	if (bc != 7) {
		api_boxfilwin(win, px + 6, py + 42, px + 6 + l * 8, py + 58, bc);
	} else {
		api_boxfilwin(win, px + 6, py + 42, px + 5 + l * 8, py + 57, bc);
	}
	api_putstrwin(win, px + 6, py + 42, c, l, s);
	return;
}
