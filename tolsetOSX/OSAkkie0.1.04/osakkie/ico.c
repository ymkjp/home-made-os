/*** ico.c by �������� */

// Special Thanks to:						//
//  �R��[�� �� (http://www.setsuki.com/)	//
//  �썇�G�� ��	(http://osask.jp/)			//

typedef unsigned char UCHAR;

struct DLL_STRPICENV { int work[16384]; };

int info_ICO(struct DLL_STRPICENV *env, int *info, int size, UCHAR *fp);
int decode0_ICO(struct DLL_STRPICENV *env, int size, UCHAR *fp, int b_type, UCHAR *buf, int skip);
int decode0_ICOpart(struct DLL_STRPICENV *env, int xsz, int ysz, int x0, int y0, int size, UCHAR *fp, int b_type, UCHAR *buf, int skip);

typedef struct {
	int pnt;
	int headsize;
	int px;
	int py;
	int pbit;
}ICOINFO;

void init_ico(UCHAR *fp, ICOINFO *icoinfo)
{
	int c;

	// �w�b�_�̏ꏊ���擾
	icoinfo->pnt = fp[18] | fp[19] << 8 | fp[20] << 16 | fp[21] << 24;

	// �A�C�R���f�B���N�g��(16)
	icoinfo->headsize = fp[icoinfo->pnt] | fp[icoinfo->pnt+1] << 8 | fp[icoinfo->pnt+2] << 16 | fp[icoinfo->pnt+3] << 24;	// �w�b�_�T�C�Y
	icoinfo->px = fp[icoinfo->pnt+4] | fp[icoinfo->pnt+5] << 8 | fp[icoinfo->pnt+6] << 16 | fp[icoinfo->pnt+7] << 24;		// �A�C�R���̕�
	icoinfo->py = fp[icoinfo->pnt+8] | fp[icoinfo->pnt+9] << 8;		// �A�C�R���̍���
	c = fp[icoinfo->pnt+10] | fp[icoinfo->pnt+11] << 8;
	icoinfo->py = (c << 16)+icoinfo->py / 2;

	// �F���̎擾
	icoinfo->pbit = fp[icoinfo->pnt+14] | fp[icoinfo->pnt+15] << 8;	// �F��(�r�b�g�P��)

	return;
}


int info_ICO(struct DLL_STRPICENV *env, int *info, int size, UCHAR *fp)
{
	ICOINFO *icoinfo = (ICOINFO *) (((int *) env) + 128);
	int i, j, k;

	// �A�C�R���w�b�_�擾
	i = fp[0] | fp[1] << 8;	// Reserved
	j = fp[2] | fp[3] << 8;	// Resource type
	k = fp[4] | fp[5] << 8;	// Icon's number.

	if ((i) || ((j != 1) && (j != 2)) || (k < 1))
		return 0;	// Not icon file.

	init_ico(fp, icoinfo);

	info[0] = 0x0004;
	info[1] = 0x0000;
	info[2] = icoinfo->px;
	info[3] = icoinfo->py;

	return 1;
}

int decode0_ICO(struct DLL_STRPICENV *env, int size, UCHAR *fp, int b_type, UCHAR *buf, int skip)
{
	ICOINFO *icoinfo = (ICOINFO *) (((int *) env) + 128);
	int i, e, p, pp, x, y, px2, px3, off, flag = 0, mask[2], dmask = 0, col, cnt, *lines;
	int pal[256];

	if (b_type != 4)
		return 2;	/* �p�����[�^�G���[�ɏC�� */

	init_ico(fp, icoinfo);

	// ���k�`���̎擾
	i = fp[icoinfo->pnt+16] | fp[icoinfo->pnt+17] << 8 | fp[icoinfo->pnt+18] << 16 | fp[icoinfo->pnt+19] << 24;	// ���k�`��
	if (i)
		return 1; // Not ready compressed file.

	// �p���b�g�̎擾 & �}�X�N�擾����
	p = icoinfo->pnt + icoinfo->headsize;
	if (icoinfo->pbit <= 8) {
		i = 1<<icoinfo->pbit;
		if (i > 256)
			return 1; // too many colors.
		for (cnt = 0; cnt < i; cnt++) {
			pal[cnt] = fp[p] | fp[p+1] << 8 | fp[p+2] << 16;
			p += 4;
		}
	}

	// �}�X�N�擾����
	px2 = (icoinfo->px * icoinfo->pbit + 31) / 32 * 4;
	px3 = (icoinfo->px + 31) / 32 * 4;
	off = p;
	pp = px2 * icoinfo->py + p;

	// �A�C�R���̕`��
	x = icoinfo->px;
	y = icoinfo->py;
	mask[0] = 0;
	mask[1] = 0;
	dmask = 0;

	for (cnt = 0; cnt < icoinfo->px*icoinfo->py; cnt++) {
		if (p >= size)
			break;
		if ((cnt % icoinfo->px) == 0) {
			x -= icoinfo->px;
			y--;
			p = cnt / icoinfo->px * px2 + off;
			mask[0] = 0;
			mask[1] = cnt / icoinfo->px * px3 + pp;
			flag = -1;
		}
		if (mask[0] == 0) {
			mask[0] = 0x80;
			dmask = fp[mask[1]];
			mask[1]++;
		}
		if (icoinfo->pbit > 8) {
			e = 0;
			pal[0] = fp[p] | fp[p+1] << 8 | fp[p+2] << 16;
			p += 3;
	// �p�ɂɎg���Ȃ��悤�Ȃ̂ō���̓p�X
	//		if (pbit > 24) {
	//			h = fp[p];
	//			p++;
	//			p[y * px + x] = col;
	//			pget x,y
	//			r = r * h + (255 -h * rval) /255;
	//			g = g * h + (255 -h * gval) /255;
	//			b = b * h + (255 -h * gval) /255;
	//		}
		} else {
			if (flag < 0) {
				i = fp[p];
				p++;
				flag = 8-icoinfo->pbit;
			}
			e = (i >> flag) % (1 << icoinfo->pbit);
			if (1<<icoinfo->pbit <= e)
				break;
			flag -= icoinfo->pbit;
		}
		col = 0xC6C6C6;
		if ((dmask & mask[0]) == 0)
			col = pal[e];
		lines = (int*)&buf[y * (icoinfo->px * (b_type % 7) + skip)];
		lines[x] = col;
		mask[0] >>= 1;
		x++;
	}
	return 0;
}

int decode0_ICOpart(struct DLL_STRPICENV *env, int xsz, int ysz, int x0, int y0, int size, UCHAR *fp, int b_type, UCHAR *buf, int skip)
{
	return 1;		/* �������G���[�ɏC�� */
}
