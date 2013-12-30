/* timerとか */

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC	1	/* 確保した状態 */
#define TIMER_FLAGS_USING	2	/* タイマ作動中 */

void init_pit(void)
{
	/* PIT初期化 */
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);

	timerctl.count = 0;
	timerctl.next = 0xffffffff; /* 一人ぼっちなので、番兵の時刻 */
	return;
}

void init_timerctl(void)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    int i;
    struct TIMER *t;
    timerctl.timers0 = (struct TIMER *) memman_alloc_4k(memman, MAX_TIMER * sizeof(struct TIMER));
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = 0; /* 未使用 */
    }
	t = timer_alloc();	/* 1つくれ */
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;	/* ケツ */
	timerctl.t0 = t;	/* ケツなのに先頭。いわゆる一人ぼっち */
    return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags  = TIMER_FLAGS_ALLOC;	/* 未使用 */
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}
	}
	return 0;	/* 見つからなかった */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0;	/* 未使用 */
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
	timer->flags = TIMER_FLAGS_USING;	/* 使用中 */

	e = io_load_eflags();
	io_cli();

	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* 先頭に入れるとき */
		timerctl.t0 = timer;
		timer->next = t;	/* 次はt */
		timerctl.next = timer->timeout;
		goto fin;
	}

	/* どこに入れるか(本とはちょっと違う) */
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
	io_cli();	/* 設定中にタイマが変化するのを防止 */
	if (timer->flags == TIMER_FLAGS_USING) {
		/* タイマ取り消しは必要か（タイマ動作中か） */
		if (timer == timerctl.t0) {
			/* 先頭だった場合の取り消し処理 */
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			/* 先頭以外の取り消し処理 */
			/* timerの1つ前を探す */
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next;	/* 「timerの直前」の次が、「timerの次」を指すようにする */
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;	/* キャンセル処理成功 */
	}
	io_store_eflags(e);
	return 0;	/* キャンセル処理失敗 */
}

void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* 設定中にタイマが変化するのを防止 */
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

	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00受付完了をPICに通知 */
	timerctl.count++;
	if (timerctl.next > timerctl.count)
		return;
	timer = timerctl.t0;	/* とりあえず先頭の番地をtimerにいれる */

	for (;;) {
		if (timer->timeout > timerctl.count)
			break;	/* タイマが動作中なのでflagsをチェックしない */
		/* タイムアウト */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer)
			fifo32_put(timer->fifo, timer->data);
		else
			ts = 1;
		timer = timer->next;
	}

	timerctl.t0 = timer;	/* ずらし */
	timerctl.next = timerctl.t0->timeout;

	if (ts)
		task_switch();

	return;
}
