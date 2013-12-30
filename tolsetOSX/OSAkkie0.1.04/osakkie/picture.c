/* picture.c */
#include "bootpack.h"

int picture_info(struct DLL_STRPICENV *env, int *info, int size, char *fp)
{
	/* 自動判別 */
	if (info_BMP(env, info, size, fp) == 0) {
		/* BMPではなかった */
		if (info_JPEG(env, info, size, fp) == 0) {
			/* JPEGでもなかった */
			if (info_ICO(env, info, size, fp) == 0) {
				/* ICOでもなかった */
				return 0;
			}
		}
	}
	return 1;
}

int picture_decode0(int mode, struct DLL_STRPICENV *env, int size, char *fp, int b_type, char *buf, int skip)
{
	int i = 1;
	if (mode == 1) {
		i = decode0_BMP (env, size, fp, b_type, (char *) buf, skip);
	} else if (mode == 2) {
		i = decode0_JPEG(env, size, fp, b_type, (char *) buf, skip);
	} else if (mode == 4) {
		i = decode0_ICO (env, size, fp, b_type, (char *) buf, skip);
	}
	return i;
}

unsigned char rgb2pal(int r, int g, int b, int x, int y)
{
	static int table[4] = { 3, 1, 0, 2 };
	int i;
	x &= 1; /* 偶数か奇数か */
	y &= 1;
	i = table[x + y * 2];	/* 中間色を作るための定数 */
	r = (r * 21) / 256;	/* これで 0～20 になる */
	g = (g * 21) / 256;
	b = (b * 21) / 256;
	r = (r + i) / 4;	/* これで 0～5 になる */
	g = (g + i) / 4;
	b = (b + i) / 4;
	return 16 + r + g * 6 + b * 36;
}

/*
struct PICTURE {
	char name[18], *filebuf;
	struct RGB *picbuf;
	struct DLL_STRPICENV *env;
	int fsize, err, info[8];
	//	err list
	//	1: File not found.
	//	2: File is not image.
};
*/

struct PICTURE *picture_init(char *name, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct PICTURE *pic = (struct PICTURE *) memman_alloc_4k(memman, sizeof(struct PICTURE));
	int i;
	pic->name    = name;
	pic->env     = (struct DLL_STRPICENV *) memman_alloc_4k(memman, sizeof(struct DLL_STRPICENV));
	pic->err     = 0;
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0) {
		/* 画像ファイルが見つかったときの処理 */
		pic->fsize = finfo->size;
		pic->filebuf = file_loadfile2(finfo->clustno, &(pic->fsize), fat);
		i = picture_info(pic->env, pic->info, pic->fsize, pic->filebuf);
		if (i == 0) {
			pic->err = 2;
			return pic;
		}
		pic->picbuf = (struct RGB *) memman_alloc_4k(memman, pic->info[2] * pic->info[3] * sizeof(struct RGB));
	} else {
		pic->err = 1;
	}
	return pic;
}

int picture_draw(struct SHEET *sht, struct PICTURE *pic, int px, int py)
{
	struct RGB *q;
	int i, j;
	int bpp = get_bpp();
	unsigned char  *cp;
	unsigned short *sp;

	i = picture_decode0(pic->info[0], pic->env, pic->fsize, pic->filebuf,
										4, (unsigned char *) pic->picbuf, 0);
	if (i != 0) {
		return i;	// Picture decode error.
	}

	/* ここから表示処理 */
	if (bpp == 8) {
		for (i = 0; i < pic->info[3]; i++) {
			cp = sht->buf + (py + i) * sht->bxsize + px;
			q = pic->picbuf + i * pic->info[2];
			for (j = 0; j < pic->info[2]; j++) {
				cp[j] = rgb2pal(q[j].r, q[j].g, q[j].b, j, i);
			}
		}
	} else if (bpp == 16) {
		for (i = 0; i < pic->info[3]; i++) {
			sp = (unsigned short *) (sht->buf) + (py + i) * sht->bxsize + px;
			q = pic->picbuf + i * pic->info[2];
			for (j = 0; j < pic->info[2]; j++) {
				sp[j] = (unsigned short)(
							((pic->picbuf[i * pic->info[2] + j].r << 8) & 0xf800) |
							((pic->picbuf[i * pic->info[2] + j].g << 3) & 0x07e0) |
							(pic->picbuf[i * pic->info[2] + j].b >> 3)
						);
			}
		}
	} else if (bpp == 24) {
		/* 未実装 */
	}
	return 0;
}

void picture_free(struct PICTURE *pic)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	memman_free_4k(memman, (int) pic->filebuf, pic->fsize);
	memman_free_4k(memman, (int) pic->picbuf, pic->info[2] * pic->info[3] * sizeof(struct RGB));
	memman_free_4k(memman, (int) pic->env, sizeof(struct DLL_STRPICENV));
	memman_free_4k(memman, (int) pic, sizeof(struct PICTURE));
	return;
}
