/* したじき */

#include "bootpack.h"

#define SHEET_USE	1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;

	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if (ctl == 0)
		goto err;
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);
	if (ctl->map == 0) {
		memman_free_4k(memman, (int) ctl, sizeof (struct SHTCTL));
		goto err;
	}
	ctl->vram  = vram ;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top   = -1   ;	/* 1枚もシートがない */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0;	/* 未使用マーク */
		ctl->sheets0[i].ctl = ctl;	/* 所属を記録 */
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;	/* 使用中マークをつける */
			sht->height = -1; 		/* まずは非表示 */
			sht->task = 0;			/* 自動で閉じる機能を使わない */
			sht->min_id = -1;
			return sht;
		}
	}
	return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int sx, int sy, int bgcol)
{
	sht->buf     = buf;
	sht->bxsize  = sx;
	sht->bysize  = sy;
	sht->col_inv = bgcol;
	return;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1, sid4, *p;
	unsigned char *buf, sid, *map = ctl->map;
	unsigned short *buf16;
	unsigned int *buf24;
	struct SHEET *sht;
	int bpp = get_bpp();

	/* refresh範囲が画面外にはみ出していたら補正 */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets[0];
		buf = sht->buf;
		buf16 = (unsigned short *) (sht->buf);
		buf24 = (unsigned int *) (sht->buf);

		/* vx0～vy1を使って、bx0～by1を逆算する */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }

		if (sht->col_inv == -1) {
			if ((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0) {
					/* 透過なしの高速ver (4 byte type) */
					bx1 = (bx1 - bx0) / 4;	/* MOV回数 */
					sid4 = sid | sid << 8 | sid << 16 | sid << 24;
					for (by = by0; by < by1; by++) {
						vy = sht->vy0 + by;
						vx = sht->vx0 + bx0;
						p = (int *) &map[vy * ctl->xsize + vx];
						for (bx = 0; bx < bx1; bx++) {
							p[bx] = sid4;
						}
					}
			} else {
				/* 透過なしの高速ver (1 byte type) */
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		} else {
			/* 透過ありの通常ver */
			if (bpp == 8) {
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						if (buf[by * sht->bxsize + bx] != sht->col_inv)
							map[vy * ctl->xsize + vx] = sid;
					}
				}
			} else if (bpp == 16) {
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						if (buf16[by * sht->bxsize + bx] != sht->col_inv)
							map[vy * ctl->xsize + vx] = sid;
					}
				}
			} else if (bpp == 24) {
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						if (buf24[by * sht->bxsize + bx] != sht->col_inv)
							map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		}
	}
	return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	int bx2, sid4, i, i1, *p, *q, *r, j = 0;
	unsigned char sid, *map = ctl->map;
	unsigned char *buf, *vram = ctl->vram;
	unsigned short *buf16, *vram16 = (unsigned short *) (ctl->vram);
	unsigned int *buf24, *vram24 = (unsigned int *) (ctl->vram);
	struct SHEET *sht;
	int bpp = get_bpp();

	/* refresh範囲が画面外にはみ出していたら補正 */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

	for (h = h0; h <= h1; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0;
		/* vx0～vy1を使って、bx0～by1を逆算する */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		if (bpp == 8) {
			buf = sht->buf;
			if ((sht->vx0 & 3) == 0) {
				/* 4 byte type */
				i  = (bx0 + 3) / 4;	/* bx0を4で割ったもの（端数切り上げ） */
				i1 =  bx1      / 4;	/* bx1を4で割ったもの（端数切り捨て） */
				i1 = i1 - i;
				sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1 && (bx & 3) != 0; bx++) {
						/* 前の端数を1バイトずつ */
						vx = sht->vx0 + bx;
						if (map[vy * ctl->xsize + vx] == sid) {
							vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
						}
					}
					vx = sht->vx0 + bx;
					p = (int *) &map[vy * ctl->xsize + vx];
					q = (int *) &vram[vy * ctl->xsize + vx];
					r = (int *) &buf[by * sht->bxsize + bx];
					for (i = 0; i < i1; i++) {
						/* 4の倍数の部分 */
						if (p[i] == sid4) {
							q[i] = r[i];	/* 多くの場合はコレになるっぽいので速い */
						} else {
							bx2 = bx + i * 4;
							vx = sht->vx0 + bx2;
							/* 4個書くのは面倒だった */
							for (j = 0; j < 4; j++) {
								if (map[vy * ctl->xsize + vx + j] == sid) {
									vram[vy * ctl->xsize + vx + j] = buf[by * sht->bxsize + bx2 + j];
								}
							}
						}
					}
					for (bx += i1 * 4; bx < bx1; bx++) {
						/* 後ろの端数を1バイトずつ */
						vx = sht->vx0 + bx;
						if (map[vy * ctl->xsize + vx] == sid)
							vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			} else {
				/* 1 byte type */
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						if (map[vy * ctl->xsize + vx] == sid)
							vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		} else if (bpp == 16) {
			buf16 = (unsigned short *) (sht->buf);
			/* 1 byte type */
			for (by = by0; by < by1; by++) {
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1; bx++) {
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid)
						vram16[vy * ctl->xsize + vx] = buf16[by * sht->bxsize + bx];
				}
			}
		} else if (bpp == 24) {
			buf24 = (unsigned int *) (sht->buf);
			/* 1 byte type */
			for (by = by0; by < by1; by++) {
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1; bx++) {
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid)
						vram24[vy * ctl->xsize + vx] = buf24[by * sht->bxsize + bx];
				}
			}
		}
	}
	return;
}

void sheet_updown(struct SHEET *sht, int height)
{
	struct SHTCTL *ctl = sht->ctl;
	int h, old = sht->height; /* 設定前の高さを記憶 */

	/* 指定されたheightが低すぎたり高すぎたら修正 */
	if (height > ctl->top + 1)	// 高すぎ
		height = ctl->top + 1;
	if (height < -1)			// 低すぎ
		height = -1;
	sht->height = height;

	/* sheets[]の並べ替え */
	if (old > height) {
		/* 以前よりも低くなる */
		if (height >= 0) {
			/* oldとheightの間の下敷きを引き上げる */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		} else {
			/* 非表示にする（heightが-1） */
			if (ctl->top > old) {
				/* 上の下敷きを下ろす */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* 表示中の下敷きが1つ減るので、一番上の高さがへる */
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
		}
	} else if (old < height) {
		/* 以前よりも高くなる */
		if (old >= 0) {
			/* oldとheightの間の下敷きを押し下げる */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {
			/* 表示する(heightが0以上) */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* 表示中の下敷きが1つ増えるので、一番上の高さが増える */
		}
		/* 新しい下敷きの情報に沿って画面を再描画 */
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) {
		/* 表示中の下敷きの場合はrefreshする */
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0,
						 sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	}
	return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) {
		/* 表示中の下敷きの場合はrefreshする */
		sheet_refreshmap(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}

void sheet_free(struct SHEET *sht)
{
	if (sht->height >= 0)
		sheet_updown(sht, -1);	/* 表示中のときは非表示に */
	sht->flags = 0;	/* 未使用状態に戻す */

	return;
}
