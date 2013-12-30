/* コンソール関係 */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>
int atoi(const char *nptr);

void console_task(struct SHEET *sheet, int memtotal)
{
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, j = 0, k = 0;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	char *cmdline = (char *) memman_alloc_4k(memman, 30 * 21);
	struct FILEHANDLE fhandle[8];
	unsigned char *nihongo = (char *) *((int *) 0x0fe8);
	struct CONSOLE cons;
	cons.sht    = sheet;
	cons.curx   =     8;
	cons.cury   =    28;
	cons.curcol =    -1;
	task->cons = &cons;
	task->cmdline = cmdline;

	for (i = 0; i < 30 * 21; i++) {
		cmdline[i] = 0;
	}

	if (cons.sht != 0) {
		cons.timer = timer_alloc();
		timer_init(cons.timer, &task->fifo, 1);
		timer_settime(cons.timer, 50);
	}
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	for (i = 0; i < 8; i++) {
		fhandle[i].buf = 0;	/* 未使用マーク */
	}
	task->fhandle = fhandle;
	task->fat = fat;

	/* 日本語フォントファイルを読み込めたか */
	if (nihongo[4096] != 0xff) {
		task->langmode = 1;
	} else {
		task->langmode = 0;
	}
	task->langbyte1 = 0;

	/* View prompt. */
	cons_putchar(&cons, '>', 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons.sht != 0) {
				/* カーソル用 */
				if (i) {
					timer_init(cons.timer, &task->fifo, 0);
					if (cons.curcol >= 0)
						cons.curcol = 0x000000;
				} else {
					timer_init(cons.timer, &task->fifo, 1);
					if (cons.curcol >= 0)
						cons.curcol = 0xffffff;
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {
				/* カーソルON */
				cons.curcol = 0xffffff;
			}
			if (i == 3) {
				/* カーソルOFF */
				if (cons.sht != 0) {
					boxfill((unsigned int *) (cons.sht->buf), cons.sht->bxsize,
							0x000000, cons.curx, cons.cury, cons.curx + 7, cons.cury + 15);
				}
				cons.curcol = -1;
			}
			if (i == 4) {
				/* コンソールの「×」ボタンクリック */
				cmd_exit(&cons, fat);
			}
			if (256 <= i && i < 512) {
				/* キーボード */
				i -= 256;	// 先に引いとく
				if (i == 0x01) {
					/* エスケープキー */
					/* 入力中のコンソールの文字を全部消す */
					cons_putchar(&cons, ' ', 0);
					while (cons.curx > 16) {
						cons.curx -= 8;
						cons_putchar(&cons, ' ', 0);
					}
				} else if (i == 0x08) {
					/* バックスペース */
					if (cons.curx > 16) {
						cons_putchar(&cons, ' ', 0);
						cons.curx -= 8;
						cons_putchar(&cons, ' ', 0);
					}
				} else if (i == 0x0a) {
					/* Enterキー */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.curx / 8 - 2] = 0;
					k = 0;
					for (i = 0; i < 30; i++) {
						/* 直前のコマンドと比較 */
						if (cmdline[i] == cmdline[30 + i])
							k++;
					}
					if (i != k && cons.curx / 8 - 2 > 0) {
						/* 直前のコマンドとは違って、何か文字が入力されていた */
						/* コマンド履歴ずらし（やり方ふるい？） */
						for (j = 20; 0 < j; j--) {
							for (i = 0; i < 30; i++) {
								cmdline[30 * j + i] = cmdline[30 * (j - 1) + i];
								if (cmdline[30 * j + i] == 0) {
									break;
								}
							}
						}
					}
					j = 0;
					cons_newline(&cons);
					cons_runcmd(cmdline, &cons, fat, memtotal);
					if (cons.sht == 0) {
						cmd_exit(&cons, fat);
					}
					/* プロンプト表示 */
					cons_putchar(&cons, '>', 1);
				} else if (i == 0xb8) {
					/* Up key */
					if (j < 20) {
						if (cmdline[(j + 1) * 30] != 0) {
							/* 一つ前に履歴がある */
							j++;
							cons_recent(&cons, cmdline, j);
						}
					}
				} else if (i == 0xb2) {
					/* Down key */
					if (1 < j) {
						j--;
						cons_recent(&cons, cmdline, j);
					}
				} else {
					/* 一般文字 */
				 	if (cons.curx < 240) {
						cmdline[cons.curx / 8 - 2] = i;
						cons_putchar(&cons, i, 1);
					}
				}
			}
			/* カーソル再表示 */
			if (cons.sht != 0) {
				if (cons.curcol >= 0) {
					boxfill((unsigned int *) (cons.sht->buf), cons.sht->bxsize, 
							cons.curcol, cons.curx, cons.cury + 15, cons.curx + 7, cons.cury + 15);
				}
				sheet_refresh(cons.sht, cons.curx, cons.cury + 15, cons.curx + 8, cons.cury + 16);
			}
		}
	}
}

void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;

	if (s[0] == 0x09) {
		/* Tab */
		for (;;) {
			if (cons->sht != 0) {
				putfonts_asc_sht(cons->sht, cons->curx, cons->cury, 0xffffff, 0x000000, " ", 1);
			}
			cons->curx += 8;
			if (cons->curx == 8 + 240)
				cons_newline(cons);
			if (((cons->curx - 8) & 0x1f) == 0)
				break;	/* 32で割り切れたらbreak; */
		}
	} else if (s[0] == 0x0a) {
		/* 改行 */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {
		/* 復帰 */
		// とりあえずなにもしない
	} else {
		/* ふつーの文字 */
		if (cons->sht != 0) {
			putfonts_asc_sht(cons->sht, cons->curx, cons->cury, 0xffffff, 0x000000, s, 1);
		}
		if (move) {
			cons->curx += 8;
			if (cons->curx == 8 + 240)
				cons_newline(cons);
		}
	}
	return;
}

void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++)
		cons_putchar(cons, *s, 1);
	return;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
	int i;
	for (i = 0; i < l; i++)
		cons_putchar(cons, s[i], 1);
	return;
}

void cons_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	struct TASK *task = task_now();
	int bpp = get_bpp();
	unsigned short *sp;
	unsigned int   *ip;
	if (cons->cury < 28 + 112) {
		cons->cury += 16;
	} else {
		/* スクロール */
		if (sheet != 0) {
			if (bpp == 8) {
				for (y = 28; y < 28 + 112; y++) {
					for (x = 8; x < 8 + 240; x++) {
						sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
					}
				}
				for (y = 28 + 112; y < 28 + 128; y++) {
					for (x = 8; x < 8 + 240; x++) {
						sheet->buf[x + y * sheet->bxsize] = get_color(bpp, 0x000000);
					}
				}
			} else if (bpp == 16) {
				sp = (unsigned short *) (sheet->buf);
				for (y = 28; y < 28 + 112; y++) {
					for (x = 8; x < 8 + 240; x++) {
						sp[x + y * sheet->bxsize] = sp[x + (y + 16) * sheet->bxsize];
					}
				}
				for (y = 28 + 112; y < 28 + 128; y++) {
					for (x = 8; x < 8 + 240; x++) {
						sp[x + y * sheet->bxsize] = get_color(bpp, 0x000000);
					}
				}
			} else if (bpp == 24) {
				ip = (unsigned int *) (sheet->buf);
				for (y = 28; y < 28 + 112; y++) {
					for (x = 8; x < 8 + 240; x++) {
						ip[x + y * sheet->bxsize] = ip[x + (y + 16) * sheet->bxsize];
					}
				}
				for (y = 28 + 112; y < 28 + 128; y++) {
					for (x = 8; x < 8 + 240; x++) {
						ip[x + y * sheet->bxsize] = get_color(bpp, 0x000000);
					}
				}
			}
			sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
		}
	}
	cons->curx = 8;
	if (task->langmode == 1 && task->langbyte1 != 0) {
		cons->curx += 8;
	}
	return;
}

void cons_recent(struct CONSOLE *cons, char *cmdline, int n)
{
	int i;
	/* 入力中のコンソールの文字を全部消す */
	cons_putchar(cons, ' ', 0);
	while (cons->curx > 16) {
		cons->curx -= 8;
		cons_putchar(cons, ' ', 0);
	}
	/* 現在からn個前のコマンドを現在のコマンドラインにコピーして出力 */
	for (i = 0; i < 30; i++) {
		cmdline[i] = cmdline[n * 30 + i];
		if (cmdline[i] == 0) {
			break;
		}
		cons_putchar(cons, cmdline[i], 1);
	}
	return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, int memtotal)
{
	/* コマンド実行 */
	if (strcmp(cmdline, "mem") == 0 && cons->sht != 0) {
		/* memコマンド */
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0 && cons->sht != 0) {
		/* clsコマンド */
		cmd_cls(cons);
	} else if (strcmp(cmdline, "dir") == 0 && cons->sht != 0) {
		/* dirコマンド */
		cmd_dir(cons);
	} else if (strcmp(cmdline, "exit") == 0) {
		/* exitコマンド */
		cmd_exit(cons, fat);
	} else if (strncmp(cmdline, "start ", 6) == 0) {
		/* startコマンド */
		cmd_start(cons, cmdline, memtotal);
	} else if (strncmp(cmdline, "ncst ", 5) == 0) {
		/* ncstコマンド */
		cmd_ncst(cons, cmdline, memtotal);
	} else if (strncmp(cmdline, "langmode", 8) == 0) {
		cmd_langmode(cons, cmdline);
	} else if (strcmp(cmdline, "reboot") == 0) {
		/* rebootコマンド */
		cmd_reboot(cons);
	} else if (strcmp(cmdline, "ver") == 0 && cons->sht != 0) {
		/* verコマンド */
		cmd_ver(cons);
	} else if (strncmp(cmdline, "history", 7) == 0 && cons->sht != 0) {
		/* historyコマンド */
		cmd_history(cons, cmdline);
	} else if (strncmp(cmdline, "wallpaper ", 10) == 0 && cons->sht != 0) {
		/* wallpaperコマンド */
		cmd_wallpaper(cons, cmdline, fat);
	} else if (strcmp(cmdline, "time") == 0 && cons->sht != 0) {
		/* timeコマンド */
		cmd_time(cons);
	} else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			/* コマンドでなく、空行でもない */
			cons_putstr0(cons, "Command or file not found.\n\n");
		}
	}
	return;
}

void cmd_mem(struct CONSOLE *cons, int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "Total: %dKB\nFree : %dKB\n\n", memtotal / 1000, memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

void cmd_cls(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	int bpp = get_bpp();
	unsigned short *sp;
	unsigned int   *ip;
	if (bpp == 8) {
		for (y = 28; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = get_color(bpp, 0x000000);
			}
		}
	} else if (bpp == 16) {
		sp = (unsigned short *) (sheet->buf);
		for (y = 28; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sp[x + y * sheet->bxsize] = get_color(bpp, 0x000000);
			}
		}
	} else if (bpp == 24) {
		ip = (unsigned int *) (sheet->buf);
		for (y = 28; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				ip[x + y * sheet->bxsize] = get_color(bpp, 0x000000);
			}
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cury = 28;
	return;
}

void cmd_dir(struct CONSOLE *cons)
{
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (finfo[i].name[0] == 0x00)
			break;
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++)
					s[j] = finfo[i].name[j];
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}

void cmd_exit(struct CONSOLE *cons, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = task_now();
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct FIFO32 *fifo = (struct FIFO32 *) *((int *) 0x0fec);
	timer_cancel(cons->timer);
	memman_free_4k(memman, (int) fat, 4 * 2880);
	io_cli();
	if (cons->sht != 0) {
		fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);	/* 768~1023 */
	} else {
		fifo32_put(fifo, task - taskctl->tasks0 + 1024);	/* 1024~2023 */
	}
	io_sti();
	for (;;) {
		task_sleep(task);
	}
}

void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal)
{
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht = open_console(shtctl, memtotal);
	struct FIFO32 *fifo = &sht->task->fifo;
	int i;
	sheet_slide(sht, 32, 4);
	sheet_updown(sht, shtctl->top);
	/* コマンドラインに入力された文字を、1文字ずつ新しいコンソールに入力 */
	for (i = 6; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	cons_newline(cons);
	return;
}

void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal)
{
	struct TASK *task = open_constask(0, memtotal);
	struct FIFO32 *fifo = &task->fifo;
	int i;
	/* コマンドラインに入力された文字を、1文字ずつ新しいコンソールに入力 */
	for (i = 5; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	cons_newline(cons);
	return;
}

void cmd_langmode(struct CONSOLE *cons, char *cmdline)
{
	struct TASK *task = task_now();
	unsigned char mode;
	static unsigned char s[21 * 3 - 5] = {
		'A', 'S', 'C', 'I', 'I', ' ', 'M', 'o', 'd', 'e', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0x93, 0xFA, 0x96, 0x7B, 0x8C, 0xEA, 0x53, 0x68, 0x69, 0x66, 0x74, 0x4A, 0x49, 0x53, 0x83, 0x82, 0x81, 0x5B, 0x83, 0x68, 0, 
		0xC6, 0xFC, 0xCB, 0xDC, 0xB8, 0xEC, 0x45, 0x55, 0x43, 0xA5, 0xE2, 0xA1, 0xBC, 0xA5, 0xC9, 0
	};

	if (cmdline[8] == 0) {
		/* パラメータ指定なし */
		cons_putstr0(cons, &s[task->langmode * 21]);
		cons_newline(cons);
		cons_newline(cons);
		return;
	}

	mode = cmdline[9] - '0';
	if (mode <= 2) {
		task->langmode = mode;
		cons_putstr0(cons, &s[mode * 21]);
		cons_newline(cons);
	} else {
		cons_putstr0(cons, "Mode number error.\n");
	}
	cons_newline(cons);
	return;
}

void cmd_reboot(struct CONSOLE *cons)
{
	io_cli();
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, 0xfe); /* 再起動 */
	for (;;)
		io_hlt();
}

void cmd_ver(struct CONSOLE *cons)
{
	cons_putstr0(cons, OSAKKIE_VERSION1);
	cons_putstr0(cons, OSAKKIE_VERSION2);
	return;
}

void cmd_history(struct CONSOLE *cons, char *cmdline)
{
	char s[40], s2[30];
	int i, j, num = 0;

	/* 引数が指定されている */
	if (*(cmdline + 7) != 0) {
		num = atoi(cmdline + 8);
		if (0 < num && num < 21) {
			for (i = 0; i < 30; i++) {
				s2[i] = cmdline[30 * num + i];
			}
			sprintf(s, "[%d] %s\n\n", num, s2);
			cons_putstr0(cons, s);
		} else {
			/* 履歴保存数を超えているか1未満が指定されていたら、使い方を表示 */
			cons_putstr0(cons, "using> history [1-20]\n\n");
		}
		return;
	}

	/* 引数が指定されていないので、履歴をすべて表示 */
	for (j = 20; 0 < j; j--) {
		for (i = 0; i < 30; i++) {
			s2[i] = cmdline[30 * j + i];
			if (s2[i] == 0) {
				break;	/* 文字列が0まで入った */
			}
		}
		if (s2[0] == 0) {
			/* 履歴がない（カラッポ） */
			continue;
		}
		sprintf(s, "[%d] %s\n", j, s2);
		cons_putstr0(cons, s);
	}
	cons_putstr0(cons, "\n");
	return;
}

void cmd_wallpaper(struct CONSOLE *cons, char *cmdline, int *fat)
{
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht = shtctl->sheets[0];
	struct PICTURE *wall;

	if (cmdline[10] == '0' && cmdline[11] == 0) {
		/* 壁紙OFF */
		init_screen((unsigned int *) (sht->buf), sht->bxsize, sht->bysize);
	} else {
		/* 壁紙表示 */
		wall = picture_init(cmdline + 10, fat);
		if (wall->err == 1) {
			cons_putstr0(cons, "File not found.\n\n");
			return;
		}
		if (wall->err == 2) {
			cons_putstr0(cons, "File is not picture.\n\n");
			return;
		}
		if ((wall->info[2] > sht->bxsize) | (wall->info[3] > sht->bysize)) {
			/* 画面からはみ出すっぽい */
			cons_putstr0(cons, "Picture is too big.\n\n");
			return;
		}
		/* ここから表示処理 */
		if ((wall->info[2] < sht->bxsize) | (wall->info[3] < sht->bysize)) {
			/* 画面サイズよりも画像が小さかったら背景色で塗りつぶす */
			init_screen((unsigned int *) (sht->buf), sht->bxsize, sht->bysize);
		}
		if (picture_draw(sht, wall, (sht->bxsize-wall->info[2])/2, (sht->bysize-wall->info[3])/2) != 0) {
			cons_putstr0(cons, "Picture decode error.\n\n");
			return;
		}
		picture_free(wall);
	}
	sheet_refresh(sht, 0, 0, sht->bxsize, sht->bysize);
	cons_newline(cons);
	return;
}

void cmd_time(struct CONSOLE *cons)
{
	int i, t[6];
	char s[18];
	for (i = 0; i < 7; i++) {
		t[i] = rtc_get(i);
	}
	sprintf(s, "%04d/%02d/%02d %02d:%02d:%02d\n", t[0], t[1], t[2], t[3], t[4], t[5]);
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SHTCTL *shtctl;
	struct SHEET *sht;
	char name[18], *p, *q;
	struct TASK *task = task_now();
	int i;
	int segsize, datsize, esp, dathrb, appsize;

	task->langmode0 = task->langmode;

	/* コマンドラインからファイル名を生成 */
	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ')
			break;
		name[i] = cmdline[i];
	}
	name[i] = 0;

	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		/* 見つからなかったので、.HRBをつけてリトライ */
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}
	if (finfo == 0) {
		/* それでも見つからなかった */
		return 0;
	}

	/* ファイルがあった */
	appsize = finfo->size;
	p = file_loadfile2(finfo->clustno, &appsize, fat);
	if (appsize >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
		segsize = *((int *) (p + 0x0000));
		esp     = *((int *) (p + 0x000c));
		datsize = *((int *) (p + 0x0010));
		dathrb  = *((int *) (p + 0x0014));
		q = (char *) memman_alloc_4k(memman, segsize);
		task->ds_base = (int) q;
		set_segmdesc(task->ldt + 0, appsize - 1, (int) p, AR_CODE32_ER + 0x60);
		set_segmdesc(task->ldt + 1, segsize - 1, (int) q, AR_DATA32_RW + 0x60);
		for (i = 0; i < datsize; i++)
			q[esp + i] = p[dathrb + i];
		start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));
		shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
		for (i = 0; i < MAX_SHEETS; i++) {
			sht = &(shtctl->sheets0[i]);
			if ((sht->flags & 0x11) == 0x11 && sht->flags && sht->task == task) {
				/* アプリは終わってるのに下敷きが残っていた */
				sheet_free(sht);		/* 閉じる */
			}
		}
		for (i = 0; i < 8; i++) {
			/* クローズしてないファイルをクローズ */
			if (task->fhandle[i].buf != 0) {
				memman_free_4k(memman, (int) task->fhandle[i].buf, task->fhandle[i].size);
				task->fhandle[i].buf = 0;
			}
		}
		task->langmode = task->langmode0;		// APIで変更されたlangmodeを元に戻す
		timer_cancelall(&task->fifo);
		memman_free_4k(memman, (int) q, segsize);
		task->langbyte1 = 0;
	} else {
		cons_putstr0(cons, ".hrb file format error.\n");
	}
	memman_free_4k(memman, (int) p, appsize);
	cons_newline(cons);
	return 1;
}

/* hrb_api has moved to api.c */

int *inthandler0c(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
	sprintf(s, "EIP = %08X", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);
}

int *inthandler0d(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	sprintf(s, "EIP = %08X", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);
}
