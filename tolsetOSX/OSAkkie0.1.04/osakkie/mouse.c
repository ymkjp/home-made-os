/* �܂��� */

#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)
/* PS/2�}�E�X����̊��荞�� */
{
	int data;
	io_out8(PIC1_OCW2, 0x64);	/* IRQ-12��t������PIC�ɒʒm */
	io_out8(PIC0_OCW2, 0x62);	/* IRQ-02��t������PIC�ɒʒm */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	/* �������ݐ��FIFO�o�b�t�@���L�� */
	mousefifo = fifo;
	mousedata0 = data0;
	/* �}�E�X�L�� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return; /* ���܂�������ACK(0xfa)�����M����Ă��� */
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* ACK(0xfa)�҂� */
		if (dat == 0xfa)
			mdec->phase++;
	} else {
		/* �}�E�X��1,2,3�o�C�g�ڂ����ꂼ�ꏇ�Ԃɑ҂��Ă��� */
		mdec->buf[mdec->phase - 1] = dat;
		if (mdec->phase == 1) {
			/* �}�E�X��1�o�C�g�ڂ̎��̂� */
			if ((mdec->buf[0] & 0xc8) != 0x08)
				mdec->phase--;	// hrib05b�́i��s�j�������@�o�[�W����
		}
		if (mdec->phase == 3) {
			/* �}�E�X��3�o�C�g�ڂ̎��̂� */
			mdec->phase = 1;
			mdec->btn   = mdec->buf[0] & 0x07;
			mdec->x     = mdec->buf[1];
			mdec->y     = mdec->buf[2];
			if ((mdec->buf[0] & 0x10) != 0)
				mdec->x |= 0xffffff00;
			if ((mdec->buf[0] & 0x20) != 0)
				mdec->y |= 0xffffff00;
			mdec->y = - mdec->y; /* �}�E�X��y�����̕�������ʂƋt */
			return 1;
		}
		mdec->phase++;
	}
	return 0;
}
