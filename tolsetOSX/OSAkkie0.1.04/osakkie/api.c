/* api.c */
#include "bootpack.h"

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct TASK *task = task_now();
	int ds_base = task->ds_base;
	struct CONSOLE *cons = task->cons;
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht;
	struct FIFO32 *sys_fifo = (struct FIFO32 *) *((int *) 0x0fec);
	int *reg = &eax + 1; /* 返し値 */
	int i;
	struct FILEINFO *finfo;
	struct FILEHANDLE *fh;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned char *p;
	int *memtotal = (int *) *((int *) 0x0fe2);
	struct TASK *task2;
	struct FIFO32 *fifo;
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int bpp = binfo->vmode;
	unsigned short *sp;
	unsigned int   *ip;
	struct PICTURE *pic;

	if (edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	} else if (edx == 2) {
		cons_putstr0(cons, (char *) ebx + ds_base);
	} else if (edx == 3) {
		cons_putstr1(cons, (char *) ebx + ds_base, ecx);
	} else if (edx == 4) {
		return &(task->tss.esp0);
	} else if (edx == 5) {
		sht = sheet_alloc(shtctl);
		sht->task = task;
		sht->flags |= 0x10;
		sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
		make_window((unsigned int *) ((char *) ebx + ds_base), esi, edi, (char *) ecx + ds_base, 0, 0);
		sht->windowname = (char *) ecx + ds_base;
		sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
		keywin_off(shtctl->sheets[shtctl->top - 2]);
		sheet_updown(sht, shtctl->top - 1);
		keywin_on(sht);
		fifo32_put(sys_fifo, 0x4000);	/* key_win変更要求 */
		reg[7] = (int) sht;
	} else if (edx == 6) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		putfonts((unsigned int *) (sht->buf), sht->bxsize, esi, edi, get_color(1, eax), (char *) ebp + ds_base);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
	} else if (edx == 7) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		boxfill((unsigned int *) (sht->buf), sht->bxsize, get_color(1, ebp), eax, ecx, esi, edi);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	} else if (edx == 8) {
		memman_init((struct MEMMAN *) (ebx + ds_base));
		ecx &= 0xfffffff0;	/* 16バイト単位にする */
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
	} else if (edx == 9) {
		ecx = (ecx + 0x0f) & 0xfffffff0;	/* 16バイト単位に切り上げ */
		reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
	} else if (edx == 10) {
		ecx = (ecx + 0x0f) & 0xfffffff0;	/* 16バイト単位に切り上げ */
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
	} else if (edx == 11) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		i = get_color(1, eax);
		if (bpp == 8) {
			sht->buf[sht->bxsize * edi + esi] = get_color(bpp, i);
		} else if (bpp == 16) {
			sp = (unsigned short *) (sht->buf);
			sp[sht->bxsize * edi + esi] = get_color(bpp, i);
		} else if (bpp == 24) {
			ip = (unsigned int *) (sht->buf);
			ip[sht->bxsize * edi + esi] = get_color(bpp, i);
		}
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
		}
	} else if (edx == 12) {
		sht = (struct SHEET *) ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
	} else if (edx == 13) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		hrb_api_linewin(sht, eax, ecx, esi, edi, get_color(1, ebp));
		if ((ebx & 1) == 0) {
			if (eax > esi) {
				i = eax;
				eax = esi;
				esi = i;
			}
			if (ecx > edi) {
				i = ecx;
				ecx = edi;
				edi = i;
			}
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	} else if (edx == 14) {
		sheet_free((struct SHEET *) ebx);
		keywin_on(shtctl->sheets[shtctl->top - 2]);
	} else if (edx == 15) {
		for (;;) {
			io_cli();
			if (fifo32_status(&task->fifo) == 0) {
				if (eax != 0) {
					/* fifoカラッポ */
					task_sleep(task);	/* 寝て待つ */
				} else {
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) {
				/* カーソル用 */
				/* アプリ実行中はカーソルが出ないので、いつも次は表示用の1を注文 */
					timer_init(cons->timer, &task->fifo, 1);
					timer_settime(cons->timer, 50);
			}
			if (i == 2) {
				/* カーソルON */
				cons->curcol = 0xffffff;
			}
			if (i == 3) {
				/* カーソルOFF */
				cons->curcol = -1;
			}
			if (i == 4) {
				timer_cancel(cons->timer);
				io_cli();
				fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);	/* 2024～2279 */
				cons->sht = 0;
				io_sti();
			}
			if (i >= 256) {
				reg[7] = i - 256;
				return 0;
			}
		}
	} else if (edx == 16) {
		reg[7] = (int) timer_alloc();
		((struct TIMER *) reg[7])->flags2 = 1;	/* 自動キャンセル有効 */
	} else if (edx == 17) {
		timer_init((struct TIMER *) ebx, &task->fifo, eax + 256);
	} else if (edx == 18) {
		timer_settime((struct TIMER *) ebx, eax);
	} else if (edx == 19) {
		timer_free((struct TIMER *) ebx);
	} else if (edx == 20) {
		if (eax == 0) {
			/* 音を消す */
			i = io_in8(0x61);
			io_out8(0x61, i & 0x0d);
		} else {
			i = 1193180000 / eax;
			io_out8(0x43, 0xb6);
			io_out8(0x42, i & 0xff);
			io_out8(0x42, i >> 8);
			i = io_in8(0x61);
			io_out8(0x61, (i | 0x03) & 0x0f);
		}
	} else if (edx == 21) {
		for (i = 0; i < 8; i++) {
			if (task->fhandle[i].buf == 0) {
				break;
			}
		}
		fh = &task->fhandle[i];
		reg[7] = 0;
		if (i < 8) {
			finfo = file_search((char *) ebx + ds_base,
						(struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
			if (finfo != 0) {
				reg[7] = (int) fh;
				fh->size = finfo->size;
				fh->pos = 0;
				fh->buf = file_loadfile2(finfo->clustno, &fh->size, task->fat);
			}
		}
	} else if (edx == 22) {
		fh = (struct FILEHANDLE *) eax;
		memman_free_4k(memman, (int) fh->buf, fh->size);
		fh->buf = 0;
	} else if (edx == 23) {
		fh = (struct FILEHANDLE *) eax;
		if (ecx == 0) {
			fh->pos = ebx;				/* シーク原点はファイル先頭 */
		} else if (ecx == 1) {
			fh->pos += ebx;				/* シーク原点は現在のアクセス位置 */
		} else if (ecx == 2) {
			fh->pos = fh->size + ebx;	/* シーク原点はファイルの終端 */
		}
		if (fh->pos < 0) {
			fh->pos = 0;
		}
		if (fh->pos > fh->size) {
			fh->pos = fh->size;
		}
	} else if (edx == 24) {
		fh = (struct FILEHANDLE *) eax;
		if (ecx == 0) {
			reg[7] = fh->size;			/* ファイルサイズ */
		} else if (ecx == 1) {
			reg[7] = fh->pos;			/* ファイル先頭から現在位置までのサイズ */
		} else if (ecx == 2) {
			reg[7] = fh->pos - fh->size;/* ファイル終端から現在位置までのサイズ */
		}
	} else if (edx == 25) {
		fh = (struct FILEHANDLE *) eax;
		for (i = 0; i < ecx; i++) {
			if (fh->pos == fh->size) {
				break;
			}
			*((char *) ebx + ds_base + i) = fh->buf[fh->pos];
			fh->pos++;
		}
		reg[7] = i;
	} else if (edx == 26) {
		i = 0;
		for (;;) {
			*((char *) ebx + ds_base + i) = task->cmdline[i];
			if (task->cmdline[i] == 0) {
				break;
			}
			if (i >= ecx) {
				break;
			}
			i++;
		}
		reg[7] = i;
	} else if (edx == 27) {
		reg[7] = task->langmode;
	} else if (edx == 0x1001) {
		/* Haritomo common API tomo_gettick() */
		reg[7] = timerctl.count;
	} else if (edx == 0x1003) {
		/* Haritomo common API tomo_setlang() */
		task->langmode = ebx;
	} else if (edx == 0x1004) {
		/* Haritomo common API tomo_sysinfo() */
		struct SYS_INFO *info = (struct SYS_INFO *) (eax + ds_base);
		info->cyls  = binfo->cyls;
		info->leds  = binfo->leds;
		info->vmode = binfo->vmode;
		info->reserve = binfo->reserve;
		info->scrnx = binfo->scrnx;
		info->scrny = binfo->scrny;
		info->vram  = binfo->vram;
		info->os_type = 4; /* OSAkkie */
	} else if (edx == 0x1005) {
		/* Haritomo common API tomo_systime() */
		struct TIME_INFO *time = (struct TIME_INFO *) (eax + ds_base);
		time->year    = rtc_get(0);
		time->month   = rtc_get(1);
		time->day     = rtc_get(2);
		time->hour    = rtc_get(3);
		time->minutes = rtc_get(4);
		time->second  = rtc_get(5);
	} else if (edx == 0x4002) {
		/* OSAkkie API osak_putministr() */
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		putminifonts((unsigned int *) (sht->buf), sht->bxsize, esi, edi, eax, (char *) ebp + ds_base);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + ecx * 6, edi + 12);
		}
	} else if (edx == 0x4003) {
		/* OSAkkie API osak_exec() */
		/* 1文字ずつ新しいコンソールに入力 */
		task2 = open_constask(0, (int) memtotal);
		fifo = &task2->fifo;
		p = (char *) (ebp + ds_base);
		for (i = 0; p[i] != 0; i++) {
			fifo32_put(fifo, p[i] + 256);
		}
		fifo32_put(fifo, 10 + 256);
	} else if (edx == 0x4004) {
		/* OSAkkie API osak_getbuflen() */
		reg[7] = bpp >> 3;
	} else if (edx == 0x4005) {
		/* OSAkkie API osak_getcolor() */
		reg[7] = get_color(ebx, ecx);
	} else if (edx == 0x4006) {
		pic = picture_init((char *) ebp + ds_base, task->fat);
		reg[7] = (int) pic;
		if (pic->err != 0) {
			reg[7] = -1;
		}
	} else if (edx == 0x4007) {
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		pic = (struct PICTURE *) eax;
		i = picture_draw(sht, pic, ecx, esi);
		if (i == 0) {
			if ((ebx & 1) == 0) {
				sheet_refresh(sht, ecx, esi, ecx + pic->info[2] + 1, esi + pic->info[3] + 1);
			}
			reg[7] = i;
		} else {
			reg[7] = -1;
		}
	} else if (edx == 0x4008) {
		pic = (struct PICTURE *) eax;
		picture_free(pic);
	} else if (edx == 0x4009) {
		reg[7] = rtc_get(ecx);
	} else if (edx == 0x4100) {
		/* OSAkkie API osak_putstringwin() */
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		putfonts((unsigned int *) (sht->buf), sht->bxsize, esi, edi, get_color(1, eax), (char *) ebp + ds_base);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
	} else if (edx == 0x4101) {
		/* OSAkkie API osak_boxfilwin() */
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		boxfill((unsigned int *) (sht->buf), sht->bxsize, ebp, eax, ecx, esi, edi);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	} else if (edx == 0x4102) {
		/* OSAkkie API osak_point() */
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		i = get_color(bpp, eax);
		if (bpp == 8) {
			sht->buf[sht->bxsize * edi + esi] = i;
		} else if (bpp == 16) {
			sp = (unsigned short *) (sht->buf);
			sp[sht->bxsize * edi + esi] = i;
		} else if (bpp == 24) {
			ip = (unsigned int *) (sht->buf);
			ip[sht->bxsize * edi + esi] = i;
		}
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
		}
	} else if (edx == 0x4103) {
		/* OSAkkie API osak_linewin() */
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
		if ((ebx & 1) == 0) {
			if (eax > esi) {
				i = eax;
				eax = esi;
				esi = i;
			}
			if (ecx > edi) {
				i = ecx;
				ecx = edi;
				edi = i;
			}
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}

	return 0;
}

void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
	int i, x, y, len, dx, dy;
	int bpp = get_bpp();
	int c = get_color(bpp, col);	/* 色変換 */
	unsigned short *sp;
	unsigned int   *ip;

	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0){
		dx = - dx;
	}
	if (dy < 0){
		dy = - dy;
	}
	if (dx >= dy) {
		len = dx + 1;
		if (x0 > x1) {
			dx = - 1024;
		} else {
			dx = 1024;
		}
		if (y0 <= y1) {
			dy = ((y1 - y0 + 1) << 10) / len;
		} else {
			dy = ((y1 - y0 - 1) << 10) / len;
		}
	} else {
		len = dy + 1;
		if (y0 > y1) {
			dy = - 1024;
		} else {
			dy = 1024;
		}
		if (x0 <= x1) {
			dx = ((x1 - x0 + 1) << 10) / len;
		} else {
			dx = ((x1 - x0 - 1) << 10) / len;
		}
	}

	if (bpp == 8) {
		for (i = 0; i < len; i++) {
			sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = c;
			x += dx;
			y += dy;
		}
	} else if (bpp == 16) {
		sp = (unsigned short *) (sht->buf);
		for (i = 0; i < len; i++) {
			sp[(y >> 10) * sht->bxsize + (x >> 10)] = c;
			x += dx;
			y += dy;
		}
	} else if (bpp == 24) {
		ip = (unsigned int *) (sht->buf);
		for (i = 0; i < len; i++) {
			ip[(y >> 10) * sht->bxsize + (x >> 10)] = c;
			x += dx;
			y += dy;
		}
	}

	return;
}
