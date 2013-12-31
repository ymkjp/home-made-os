/* rtc.c */
#include "bootpack.h"

int rtc_get(int type)
{
    static unsigned char adr[7] = { 0x32, 0x08, 0x07, 0x04, 0x02, 0x00, 0x09 };
    static unsigned char max[7] = { 0x99, 0x12, 0x31, 0x23, 0x59, 0x60, 0x99 };
	unsigned char s, t;
	int err, r = 0;
	/*	Type are...
		0 : Year	1 : Month	2 : Day
		3 : Hour	4 : Minute	5 : Second */
	if (6 <= type) {
		/* 6�ȏ�̓G���[ */
		return -1;
	}
	do  {
		err = 0;
		io_out8(0x70, adr[type]);
		s = io_in8(0x71);
		io_out8(0x70, adr[type]);
		t = io_in8(0x71);
		if (s != t || 9 < (t & 0x0f) || max[type] < t) {
			/* 2�x�ǂ݂ňႢ���������V���E�w�C�w�[�C�I */
			/* a�b�Ƃ�f���Ȃ�ĂȂ��񂾂��`�H */
			/* �R�C�c�����E�˔j�����H */
			err++;
		}
		if (type == 0) {
			/* Year�����g�N�x�c */
			r = ((t >> 4) * 10 + (t & 0x0f)) * 100;
			type = 6;
			err++;		// ���킢���������ǃG���[����
		}
	} while(err != 0);
	r += ((t >> 4) * 10 + (t & 0x0f));
	return r;
}
