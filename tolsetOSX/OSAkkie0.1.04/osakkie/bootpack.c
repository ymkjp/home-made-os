/* bootpackのメイン */

#include "bootpack.h"
#include <stdio.h>

#define KEYCMD_LED	0xed

void close_constask(struct TASK *task);
void close_console(struct SHEET *sht);
struct SHEET *search_sheet(struct SHTCTL *shtctl, int mx, int my);
struct SHEET *open_omnaomi(struct SHTCTL *shtctl, unsigned int memtotal, struct SHEET *oldsht);

void HariMain(void)
{
	/* System */
		struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
		struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
		unsigned int memtotal;
		struct FIFO32 fifo, keycmd;
		int fifobuf[128], keycmd_buf[32];
		struct TASK *task_a, *task;
		struct FILEINFO *finfo;
		int *fat;
		int minlist[MAX_MINID];
		int bpp = get_bpp();
	/* Keyboard */
		struct MOUSE_DEC mdec;
		static char keytable0[0x80] = {
			/* 通常の文字 */
			  0, 0x01, '1',  '2', '3', '4', '5', '6', '7', '8', '9', '0',  '-',  '^', 0x08, 0x09,
			'Q',  'W', 'E',  'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a,    0,  'A',  'S',
			'D',  'F', 'G',  'H', 'J', 'K', 'L', ';', ':',   0,   0, ']',  'Z',  'X',  'C',  'V',
			'B',  'N', 'M',  ',', '.', '/',   0, '*',   0, ' ',   0,   0,    0,    0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0, '7', '8', '9', '-', '4',  '5',  '6',  '+',  '1',
			'2',  '3', '0',  '.',   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,
			  0,    0,   0, 0x5c,   0,   0,   0,   0,   0,   0,   0,   0,    0, 0x5c,    0,    0
		};
		static char keytable1[0x80] = {
			/* Shiftを押した状態 */
			  0,    0, '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~',  '=', '~', 0x08, 0x09,
			'Q',  'W', 'E',  'R', 'T', 'Y', 'U', 'I',  'O', 'P', '`', '{', 0x0a,   0,  'A',  'S',
			'D',  'F', 'G',  'H', 'J', 'K', 'L', '+',  '*',   0,   0, '}',  'Z', 'X',  'C',  'V',
			'B',  'N', 'M',  '<', '>', '?',   0, '*',    0, ' ',   0,   0,    0,   0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0, '7',  '8', '9', '-', '4',  '5', '6',  '+',  '1',
			'2',  '3', '0',  '.',   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,    0,    0,
			  0,    0,   0,  '_',   0,   0,   0,   0,    0,   0,   0,   0,    0, '|',    0,    0
		};
		int key_shift = 0, key_ctrl = 0, key_alt = 0, flag_e0 = 0;
		int key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
	/* Mouse */
		int mx, my, x = 0, y = 0, mmx = -1, mmy = -1, new_mx = -1, new_my = 0;
		int mmx2 = 0, new_wx = 0x7fffffff, new_wy = 0, flag_mouse = 0;
	/* IME */
		unsigned char *nihongo;
		extern char hankaku[4096];
		int oldlang;
	/* Sheets */
		struct SHTCTL *shtctl;
		unsigned char *buf_back, *buf_mouse, *buf_naomi;
		struct SHEET  *sht_back, *sht_mouse, *sht_naomi, *sht = 0, *key_win, *sht2 = 0;
	/* Balloon */
		unsigned char *buf_balloon;
		struct SHEET  *sht_balloon;
		struct BALLOON *startmenu;
		int flag_balloon = 0;
	/* Misc */
		unsigned char s[40];
		int i, j;


	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo32_init(&fifo, 128, fifobuf, 0);
	*((int *) 0x0fec) = (int) &fifo;
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* PITとPIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /*	0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	*((int *) 0x0fe2) = (int) memtotal;

	init_timerctl();
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	*((int *) 0x0fe4) = (int) shtctl;
	task_a->langmode = 0;

	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny * (bpp >> 3));
	sheet_setbuf(sht_back,  buf_back,  binfo->scrnx, binfo->scrny, -1);	/* 透明色なし */
	init_screen((unsigned int *) buf_back, binfo->scrnx, binfo->scrny);
	sht_back->flags |= 0x01;	/* 背景 */

	/* sht_naomi */
	sht_naomi = sheet_alloc(shtctl);
	buf_naomi = (unsigned char *) memman_alloc_4k(memman, 28 * 26 * (bpp >> 3));
	sheet_setbuf(sht_naomi, buf_naomi, 28, 26, get_color(bpp, 0x008484));	/* 透明色あり */
	put_naomi((unsigned int *) buf_naomi);
	sht_naomi->flags |= 0x100;	/* 特殊ウィンドウ */

	/* sht_balloon */
	sht_balloon = sheet_alloc(shtctl);
	buf_balloon = (unsigned char *) memman_alloc_4k(memman, 124 * 23 * (bpp >> 3));
	sheet_setbuf(sht_balloon, buf_balloon, 124, 23, 99);	/* 透明色あり */
	startmenu = make_balloon(memman, sht_balloon, 0, 0, 6, 19, 1);
	putminifonts_asc_sht(sht_balloon, 4, 3, 0x000000, 0xffffff, "My name is naomisan", 19);
	sht_balloon->flags |= 0x100;	/* 特殊ウィンドウ */

	/* sht_cons */
	key_win = open_console(shtctl, memtotal);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	buf_mouse = (unsigned char *) memman_alloc_4k(memman, 16 * 16 * (bpp >> 3));
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);	/* 透明色番号は99 */
	init_mouse_cursor((unsigned int *) buf_mouse, 99/* 背景色 */);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 16) / 2;

	sheet_slide(sht_back,  0,  0);
	sheet_slide(key_win,   8, 16);
	sheet_slide(sht_balloon, binfo->scrnx - 130, binfo->scrny - 70);
	sheet_slide(sht_naomi, binfo->scrnx - 50, binfo->scrny - 40);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,  0);
	sheet_updown(key_win,   1);
	sheet_updown(sht_naomi, 2);
	sheet_updown(sht_mouse, 3);
	keywin_on(key_win);

	/* 最初のキーボード状態を設定してしまう */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	/* nihongo.fntの読み込み */
	fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	finfo = file_search("nihongo.fnt", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0) {
		i = finfo->size;
		nihongo = file_loadfile2(finfo->clustno, &i, fat);
	} else {
		nihongo = (unsigned char *) memman_alloc_4k(memman, 16 * 256 + 32 * 94 * 47);
		for (i = 16 * 256; i < 16 * 256 + 32 * 94 * 47; i++) {
			nihongo[i] = 0xff;	/* 全角部分は0xffで埋め尽くす */
		}
	}
	for (i = 0; i < 16 * 256; i++) {
		nihongo[i] = hankaku[i];	/* 半角部分をコピー */
	}
	*((int *) 0x0fe8) = (int) nihongo;

	/* startup */
	finfo = file_search("osakkie.hsf", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0) {
		char *ss;
		i = finfo->size;
		ss = file_loadfile2(finfo->clustno, &i, fat);
		for (x = 0; x < i; x++) {
			if (ss[x] != 0x0d) {
				fifo32_put(&key_win->task->fifo, ss[x] + 256);
			}
		}
		memman_free_4k(memman, (int) ss, i);
	}

	memman_free_4k(memman, (int) fat, 4 * 2880);

	/* minlistの初期化 */
	for(i = 0; i < MAX_MINID; i++) {
		minlist[i] = -1;		/* 未使用 */
	}

	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* キーボードに送るデータがあれば送る */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			/* fifoがカラッポになったので、保留している描画があれば実行 */
			if (new_mx >= 0) {
				io_sti();
				sheet_slide(sht_mouse, new_mx, new_my);
				new_mx = -1;
			} else if (new_wx != 0x7fffffff) {
				io_sti();
				sheet_slide(sht, new_wx, new_wy);
				new_wx = 0x7fffffff;
			} else {
				task_sleep(task_a);
				io_sti();
			}
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win != 0 && key_win->flags == 0) {
				/* 入力ウィンドウが閉じられた */
				if (shtctl->top == 2) {
					/* もうマウスと背景しかない */
					key_win = 0;
				} else {
					key_win = shtctl->sheets[shtctl->top - 2];
					keywin_on(key_win);
				}
			}
			if (256 <= i && i < 512) {
				/* キーボード */
				i -= 256;	// 先に引いとく
				if (i == 0xe0) {
					/* E0拡張キー */
					flag_e0 = 1;
				}
				if (i < 0x80 && flag_e0 == 0) {
					/* キーコードを文字コードに変換 */
					if (key_shift == 0) {
						s[0] = keytable0[i];
					} else if (key_shift == 1) {
						s[0] = keytable1[i];
					}
				} else {
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z')	{
					/* アルファベットが入力された */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
						((key_leds & 4) != 0 && key_shift != 0)   ) {
						s[0] += 0x20;	/* 小文字に変換 */
					}
				}
				if (flag_e0 == 1) {
					if (i == 0x35 || i == 0x1c) {
						/* /とEnter */
						s[0] = keytable0[i];
					}
				}
				if (0x47 <= i && i <= 0x53) {
					if (i != 0x4a && i != 0x4e) {
						/* テンキーとIns,Del,Home,End,PgUp,PgDn,カーソルキー*/
						if (flag_e0 == 1 || (key_leds & 2) != 0) {
							s[0] = keytable0[i] + 0x80;		// 一律に0x80を足す
						}
					}
				}
				if (i == 0x0f && key_alt != 0) {
					/* Alt+Tabキーはコード送信対象外 */
					s[0] = 0;
				}
				if (s[0] != 0 && key_win != 0) {
					/* 普通の文字, Enter, BackSpace, Tab */
					if (key_ctrl == 0) {
						/* コンソールへ */
						fifo32_put(&key_win->task->fifo, s[0] + 256);
					}
				}
				if (i == 0x57 && shtctl->top > 2) {
					/* F11キー */
					sheet_updown(shtctl->sheets[1], shtctl->top - 2);
				}
				if (i == 0x2a)
					key_shift |= 1;			// 左シフトon
				if (i == 0x36)
					key_shift |= 2;			// 右シフトon
				if (i == 0xaa)
					key_shift &= ~1;		// 左シフトoff
				if (i == 0xb6)
					key_shift &= ~2;		// 右シフトoff
				if (i == 0x1d) {
					if (flag_e0 == 0) {
						key_ctrl |= 1;		// 左CTRLon
					} else {
						key_ctrl |= 2;		// 右CTRLon
					}
				}
				if (i == 0x9d) {
					if (flag_e0 == 0) {
						key_ctrl &= ~1;		// 左CTRLoff
					} else {
						key_ctrl &= ~2;		// 右CTRLoff
					}
				}
				if (i == 0x38) {
					if (flag_e0 == 0) {
						key_alt |= 1;		// 左ALTon
					} else {
						key_alt |= 2;		// 右ALTon
					}
				}
				if (i == 0xb8) {
					if (flag_e0 == 0) {
						key_alt &= ~1;		// 左ALToff
					} else {
						key_alt &= ~2;		// 右ALToff
					}
				}
				if (i == 0x3a) {
					/* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 0x45) {
					/* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 0x46) {
					/* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (((i == 0x0f && key_alt != 0) || i == 0x43) && key_win != 0) {
					/* Tab 改め Alt + Tab or F9 */
					keywin_off(key_win);
					j = key_win->height - 1;
					if (j == 0) {
						j = shtctl->top - 2;
					}
					key_win = shtctl->sheets[j];
					keywin_on(key_win);
				}
				if (i == 0x2e && key_ctrl != 0 && key_win != 0) {
					/* Shift + F1 改め Ctrl + C */
					task = key_win->task;
					if (task != 0 && task->tss.ss0 != 0) {
						task->langmode = task->langmode0;	/* APIで変更したlangmodeを元に戻す */
						oldlang = task_a->langmode;		/* task_aのlangmodeを一時的にタスクにあわせる */
						task_a->langmode = task->langmode;
						cons_putstr0(task->cons, "\nBreak(key) :\n");
						task_a->langmode = oldlang;
						io_cli();
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						task_run(task, -1, 0);
					}
				}
				if (i == 0x31 && key_ctrl != 0) {
					/* Shift + F2 改め Ctrl + N */
					/* 新しく作ったコンソールをアクティブに */
					keywin_off(key_win);
					key_win = open_console(shtctl, memtotal);
					sheet_slide(key_win, 8, 16);
					sheet_updown(key_win, shtctl->top - 1);
					keywin_on(key_win);
				}
				if (i != 0xe0 && flag_e0 == 1) {
					/* e0拡張キー終了 */
					flag_e0 = 0;
				}
				if (i == 0xfa) {
					/* キーボードがデータを受け取った */
					keycmd_wait = -1;
				}
				if (i == 0xfe) {
					/* キーボードがデータを受け取れなかった */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
			} else if (512 <= i && i < 768) {
				/* マウス */
				if (mouse_decode(&mdec, i - 512)) {
					/* マウス移動 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
						mx = 0;
					if (my < 0)
						my = 0;
					if (mx > binfo->scrnx - 1)
						mx = binfo->scrnx - 1;
					if (my > binfo->scrny - 1)
						my = binfo->scrny - 1;
					new_mx = mx;
					new_my = my;
					if ((mdec.btn != 0 || flag_mouse != 0) && mmx < 0) {
						/* makeがbreakがきている */
						/* 上の下敷きから順にマウスがさすシートを探す */
						sht = search_sheet(shtctl, mx, my);
						x = mx - sht->vx0;
						y = my - sht->vy0;
						if (mdec.btn != 0 && sht != sht_naomi) {
							sheet_updown(sht_balloon, -1);	/* バルーンを消す */
							flag_balloon = 0;
						}
					}
					if (mdec.btn & 0x01) {
						/* 左ボタン */
						flag_mouse |= 0x01;
						if (mmx < 0) {
							/* 通常モード */
							if ((sht->flags & 0x100) != 0) {
								/* ナオミさんかスタートメニュー */
								if (sht == sht_naomi) {
									if (flag_balloon == 0) {
										flag_balloon = 1;
										sheet_slide(sht_balloon, sht_naomi->vx0 - 80, sht_naomi->vy0 - 30);
										sheet_updown(sht_balloon, shtctl->top - 1);
									}
									if (flag_balloon == 2) {
										flag_balloon = 3;
										sheet_updown(sht_balloon, -1);
									}
								}
							}
							if ((sht->flags & 0xf00) == 0x200) {
								/* 最小化中のウィンドウ */
								flag_mouse |= 0x0130;
							}
							if ((sht->flags & 0xf00) == 0) {
								/* ウィンドウとか */
								sheet_updown(sht, shtctl->top - 2);
								if (sht != key_win && sht != 0) {
									keywin_off(key_win);
									key_win = sht;
									keywin_on(key_win);
								}
								/* 一般ウィンドウ */
								if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
									mmx = mx;	/* ウィンドウ移動モードへ */
									mmy = my;
									mmx2 = sht->vx0;
									new_wy = sht->vy0;
								}
								if (5 <= y && y < 19) {
									if (sht->bxsize - 20 <= x && x < sht->bxsize - 5) {
										/* [X]ボタン */
										flag_mouse |= 0x0110;
									}
									if (sht->bxsize - 31 <= x && x < sht->bxsize - 21) {
										/* [_]ボタン */
										flag_mouse |= 0x0120;
									}
								}
							}
						} else {
							if ((flag_mouse & 0x0f00) == 0) {
								x = mx - mmx;	/* 移動量を計算 */
								y = my - mmy;
								/* ボタンなどを押していない */
								/* ウィンドウ移動モード */
								new_wx = (mmx2 + x + 2) & ~3;
								new_wy = new_wy + y;
								mmy = my;		/* 移動後の座標に更新 */
							}
						}
					} else if (mdec.btn & 0x02) {
						/* 右クリック */
						flag_mouse |= 0x02;
						if (mmx < 0) {
							if (sht == sht_naomi || (sht->flags & 0x200) != 0) {
								mmx = mx;	/* 移動モードへ */
								mmy = my;
								mmx2 = sht->vx0;
								new_wy = sht->vy0;
							}
						} else {
							if ((flag_mouse & 0x0f00) == 0) {
								x = mx - mmx;	/* 移動量を計算 */
								y = my - mmy;
								/* ボタンなどを押していない */
								/* ウィンドウ移動モード */
								new_wx = (mmx2 + x + 2) & ~3;
								new_wy = new_wy + y;
								mmy = my;		/* 移動後の座標に更新 */
							}
						}
					} else if (mdec.btn == 0) {
						/* breakか、なにもない */
						if (flag_mouse & 0x01) {
							/* 左break */
							if ((flag_mouse & 0x0f00) == 0x0100 && 5 <= y && y < 19) {
								/* ウィンドウのボタン */
								if ((flag_mouse & 0xf0) == 0x10 && sht->bxsize - 20 <= x && x < sht->bxsize - 5) {
									/* ウィンドウクローズ */
									task = sht->task;
									if ((sht->flags & 0x10) != 0) {
										/* アプリが作ったウィンドウか？ */
										task->langmode = task->langmode0;	/* APIで変更したlangmodeを元に戻す */
										oldlang = task_a->langmode;		/* task_aのlangmodeを一時的にタスクにあわせる */
										task_a->langmode = task->langmode;
										cons_putstr0(task->cons, "\nBreak(mouse) :\n");
										task_a->langmode = oldlang;
										io_cli();
										task->tss.eax = (int) &(task->tss.esp0);
										task->tss.eip = (int) asm_end_app;
										io_sti();
										task_run(task, -1, 0);
									} else {
										/* Console */
										sheet_updown(sht, -1);	/* とりあえず非表示に */
										keywin_off(key_win);
										key_win = 0;
										if (shtctl->top > 2) {
											key_win = shtctl->sheets[shtctl->top - 2];
											keywin_on(key_win);
										}
									}
								}
								if ((flag_mouse & 0xf0) == 0x20 && sht->bxsize - 31 <= x && x < sht->bxsize - 21) {
									/* [_]ボタンクリック */
									sheet_updown(sht, -100);	/* 非表示に */
									keywin_off(key_win);
									key_win = 0;
									sht2 = open_omnaomi(shtctl, memtotal, sht);
									sheet_slide(sht2, sht->vx0 + sht->bxsize - 45, sht->vy0);
									if (shtctl->top > 2) {
										key_win = shtctl->sheets[shtctl->top - 2];
										keywin_on(key_win);
									}
									sheet_updown(sht2, 1);
									for(j = 0; j < MAX_MINID; j++) {
										if (minlist[j] < 0) {
											minlist[j] = 1;		/* min_id使用中 */
											sht2->min_id = j;	/* 勘合札？ */
											sht->min_id = j;
											break;
										}
									}
								}
								if ((flag_mouse & 0xf0) == 0x30) {
									/* オムレツナオミをクリック */
									keywin_off(key_win);
									for (j = 0; j < MAX_SHEETS; j++) {
										key_win = &shtctl->sheets0[j];
										if (key_win->height < 0) {
											if (key_win->min_id == sht->min_id) {
												minlist[sht->min_id] = -1;
												sht->min_id = -1;
												key_win->min_id = -1;
												sheet_free(sht);
												sheet_slide(key_win, sht->vx0 - key_win->bxsize + 42, sht->vy0);
												sheet_updown(key_win, shtctl->top - 1);
												keywin_on(key_win);
												break;
											}
										}
									}
								}
							}
							/* ナオミさんかスタートメニュー */
							if (flag_balloon == 1) {
								flag_balloon = 2;
							} else if (flag_balloon == 3) {
								flag_balloon = 0;
							}
						} else if (flag_mouse & 0x02) {
							/* 右break */
						}
						flag_mouse = 0;
						mmx = -1;	/* 通常モード */
						if (new_wx != 0x7fffffff) { 
							sheet_slide(sht, new_wx, new_wy);	/* 一度確定させる */
							new_wx = 0x7fffffff;
						}
					}
				}
			} else if (768 <= i && i < 1024) {
				close_console(shtctl->sheets0 + (i - 768));
			} else if (1024 <= i && i < 2024) {
				close_constask(taskctl->tasks0 + (i - 1024));
			} else if (2024 <= i && i < 2280) {
				/* コンソールだけ閉じる */
				sht2 = shtctl->sheets0 + (i - 2024);
				memman_free_4k(memman, (int) sht2->buf, 256 * 165);
				sheet_free(sht2);
			} else if (i == 0x4000) {
				/* key_win変更要求 from api.c */
				key_win = shtctl->sheets[shtctl->top - 2];
			}
		}
	}
}

void keywin_off(struct SHEET *key_win)
{
	change_wtitle(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 3);	/* コンソールのカーソルOFF */
	}
	return;
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 2);	/* コンソールのカーソルON */
	}
	return;
}

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = task->cons_stack + 64 * 1024 - 12;
	task->tss.eip = (int) &console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *) (task->tss.esp + 4)) = (int) sht;
	*((int *) (task->tss.esp + 8)) = memtotal;
	task_run(task, 2, 2);
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return task;
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	int bpp = get_bpp();
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 256 * 165 * (bpp >> 3));
	sheet_setbuf(sht, buf, 256, 165, -1);	/* 透明色なし */
	make_window((unsigned int *) buf, 256, 165, "Console", 0/* ナオミさん */, 0);
	make_textbox(sht, 8, 28, 240, 128, 0x000000);
	sht->task = open_constask(sht, memtotal);
	sht->flags |= 0x20;	/* カーソルあり */
	return sht;
}

struct SHEET *open_omnaomi(struct SHTCTL *shtctl, unsigned int memtotal, struct SHEET *oldsht)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	int bpp = get_bpp();
	unsigned char *buf;

	buf = (unsigned char *) memman_alloc_4k(memman, 31 * 16 * (bpp >> 3));
	sheet_setbuf(sht, buf, 31, 16, 99);
	make_omnaomi(sht, 0, 0);
	sht->flags |= 0x0200;
	return sht;
}

void close_constask(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
	task->flags = 0;	/* task_free(task);の代わり */
	
}

void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = sht->task;
	memman_free_4k(memman, (int) sht->buf, 256 * 165);
	sheet_free(sht);
	close_constask(task);
	return;
}

struct SHEET *search_sheet(struct SHTCTL *shtctl, int mx, int my)
{
	struct SHEET *sht = 0;
	int i, x, y;
	for (i = shtctl->top - 1; i > 0; i--) {
		sht = shtctl->sheets[i];
		x = mx - sht->vx0;
		y = my - sht->vy0;
		if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
			if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
				return sht;
			}
		}
	}
	return 0;
}
