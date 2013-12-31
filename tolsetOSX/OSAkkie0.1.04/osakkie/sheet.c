/* �������� */

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
	ctl->top   = -1   ;	/* 1�����V�[�g���Ȃ� */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0;	/* ���g�p�}�[�N */
		ctl->sheets0[i].ctl = ctl;	/* �������L�^ */
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
			sht->flags = SHEET_USE;	/* �g�p���}�[�N������ */
			sht->height = -1; 		/* �܂��͔�\�� */
			sht->task = 0;			/* �����ŕ���@�\���g��Ȃ� */
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

	/* refresh�͈͂���ʊO�ɂ͂ݏo���Ă�����␳ */
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

		/* vx0�`vy1���g���āAbx0�`by1���t�Z���� */
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
					/* ���߂Ȃ��̍���ver (4 byte type) */
					bx1 = (bx1 - bx0) / 4;	/* MOV�� */
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
				/* ���߂Ȃ��̍���ver (1 byte type) */
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx++) {
						vx = sht->vx0 + bx;
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		} else {
			/* ���߂���̒ʏ�ver */
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

	/* refresh�͈͂���ʊO�ɂ͂ݏo���Ă�����␳ */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }

	for (h = h0; h <= h1; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0;
		/* vx0�`vy1���g���āAbx0�`by1���t�Z���� */
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
				i  = (bx0 + 3) / 4;	/* bx0��4�Ŋ��������́i�[���؂�グ�j */
				i1 =  bx1      / 4;	/* bx1��4�Ŋ��������́i�[���؂�̂āj */
				i1 = i1 - i;
				sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				for (by = by0; by < by1; by++) {
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1 && (bx & 3) != 0; bx++) {
						/* �O�̒[����1�o�C�g���� */
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
						/* 4�̔{���̕��� */
						if (p[i] == sid4) {
							q[i] = r[i];	/* �����̏ꍇ�̓R���ɂȂ���ۂ��̂ő��� */
						} else {
							bx2 = bx + i * 4;
							vx = sht->vx0 + bx2;
							/* 4�����͖̂ʓ|������ */
							for (j = 0; j < 4; j++) {
								if (map[vy * ctl->xsize + vx + j] == sid) {
									vram[vy * ctl->xsize + vx + j] = buf[by * sht->bxsize + bx2 + j];
								}
							}
						}
					}
					for (bx += i1 * 4; bx < bx1; bx++) {
						/* ���̒[����1�o�C�g���� */
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
	int h, old = sht->height; /* �ݒ�O�̍������L�� */

	/* �w�肳�ꂽheight���Ⴗ�����荂��������C�� */
	if (height > ctl->top + 1)	// ������
		height = ctl->top + 1;
	if (height < -1)			// �Ⴗ��
		height = -1;
	sht->height = height;

	/* sheets[]�̕��בւ� */
	if (old > height) {
		/* �ȑO�����Ⴍ�Ȃ� */
		if (height >= 0) {
			/* old��height�̊Ԃ̉��~���������グ�� */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		} else {
			/* ��\���ɂ���iheight��-1�j */
			if (ctl->top > old) {
				/* ��̉��~�������낷 */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* �\�����̉��~����1����̂ŁA��ԏ�̍������ւ� */
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
		}
	} else if (old < height) {
		/* �ȑO���������Ȃ� */
		if (old >= 0) {
			/* old��height�̊Ԃ̉��~�������������� */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {
			/* �\������(height��0�ȏ�) */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* �\�����̉��~����1������̂ŁA��ԏ�̍����������� */
		}
		/* �V�������~���̏��ɉ����ĉ�ʂ��ĕ`�� */
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) {
		/* �\�����̉��~���̏ꍇ��refresh���� */
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
		/* �\�����̉��~���̏ꍇ��refresh���� */
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
		sheet_updown(sht, -1);	/* �\�����̂Ƃ��͔�\���� */
	sht->flags = 0;	/* ���g�p��Ԃɖ߂� */

	return;
}
