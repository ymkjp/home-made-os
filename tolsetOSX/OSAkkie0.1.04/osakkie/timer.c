/* timer�Ƃ� */

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC	1	/* �m�ۂ������ */
#define TIMER_FLAGS_USING	2	/* �^�C�}�쓮�� */

void init_pit(void)
{
	/* PIT������ */
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);

	timerctl.count = 0;
	timerctl.next = 0xffffffff; /* ��l�ڂ����Ȃ̂ŁA�ԕ��̎��� */
	return;
}

void init_timerctl(void)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    int i;
    struct TIMER *t;
    timerctl.timers0 = (struct TIMER *) memman_alloc_4k(memman, MAX_TIMER * sizeof(struct TIMER));
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = 0; /* ���g�p */
    }
	t = timer_alloc();	/* 1���� */
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;	/* �P�c */
	timerctl.t0 = t;	/* �P�c�Ȃ̂ɐ擪�B�������l�ڂ��� */
    return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags  = TIMER_FLAGS_ALLOC;	/* ���g�p */
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}
	}
	return 0;	/* ������Ȃ����� */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0;	/* ���g�p */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;

	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;	/* �g�p�� */

	e = io_load_eflags();
	io_cli();

	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* �擪�ɓ����Ƃ� */
		timerctl.t0 = timer;
		timer->next = t;	/* ����t */
		timerctl.next = timer->timeout;
		goto fin;
	}

	/* �ǂ��ɓ���邩(�{�Ƃ͂�����ƈႤ) */
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			s->next = timer;
			timer->next = t;
			goto fin;
		}
	}

fin:
	io_store_eflags(e);
	return;
}

int timer_cancel(struct TIMER *timer)
{
	int e;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* �ݒ蒆�Ƀ^�C�}���ω�����̂�h�~ */
	if (timer->flags == TIMER_FLAGS_USING) {
		/* �^�C�}�������͕K�v���i�^�C�}���쒆���j */
		if (timer == timerctl.t0) {
			/* �擪�������ꍇ�̎��������� */
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			/* �擪�ȊO�̎��������� */
			/* timer��1�O��T�� */
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next;	/* �utimer�̒��O�v�̎����A�utimer�̎��v���w���悤�ɂ��� */
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;	/* �L�����Z���������� */
	}
	io_store_eflags(e);
	return 0;	/* �L�����Z���������s */
}

void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* �ݒ蒆�Ƀ^�C�}���ω�����̂�h�~ */
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}

void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;

	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00��t������PIC�ɒʒm */
	timerctl.count++;
	if (timerctl.next > timerctl.count)
		return;
	timer = timerctl.t0;	/* �Ƃ肠�����擪�̔Ԓn��timer�ɂ���� */

	for (;;) {
		if (timer->timeout > timerctl.count)
			break;	/* �^�C�}�����쒆�Ȃ̂�flags���`�F�b�N���Ȃ� */
		/* �^�C���A�E�g */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer)
			fifo32_put(timer->fifo, timer->data);
		else
			ts = 1;
		timer = timer->next;
	}

	timerctl.t0 = timer;	/* ���炵 */
	timerctl.next = timerctl.t0->timeout;

	if (ts)
		task_switch();

	return;
}
