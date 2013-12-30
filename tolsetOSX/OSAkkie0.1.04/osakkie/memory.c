/* メモリー */

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 386か、486以降か確認 */
	eflg = io_load_eflags() | EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg = io_load_eflags() & EFLAGS_AC_BIT;
	if (eflg)
		flg486 = 1;
	eflg &= ~EFLAGS_AC_BIT;		// andしたのを取り消す
	io_store_eflags(eflg);

	if (flg486) {
		cr0 = load_cr0() | CR0_CACHE_DISABLE;	/* キャッシュ禁止 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;	/* キャッシュ許可 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		/* 空き情報の個数 */
	man->maxfrees = 0;	/* 状況観察用: freesの最大値 */
	man->lostsize = 0;	/* 開放に失敗した合計サイズ */
	man->losts = 0;		/* 開放に失敗した回数 */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
/* 空きサイズの合計を報告 */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++)
		t += man->free[i].size;
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* メモリ確保 */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 十分な広さの空きを発見 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* free[i]がなくなったので前に詰める */
				man->frees--;
				for (; i < man->frees; i++)
					man->free[i] = man->free[i + 1];
			}
			return a;
		}
	}
	return 0;	/* 空きがなかった */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* 解放 */
{
	int i, j;

	/* free[]をaddr順に並べるために、どこに入れるか決める */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr)
			break;
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* パターン1-1: 入れる場所より前がある */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* その前の空き領域にまとめられる */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* パターン1-2: 入れた場所より後ろもある */
				if (addr + size == man->free[i].addr) {
					/* 後ろの領域もまとめられる */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]の削除:: 前に詰める */
					man->frees--;
					for (; i < man->frees; i++)
						man->free[i] = man->free[i + 1];
				}
			}
			return 0;	/* ウマー */
		}
	}
	if (i < man->frees) {
		/* パターン2: 入れる場所より後ろがある */
		if (addr + size == man->free[i].addr) {
			/* 後ろの領域にまとめられる */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;	/* ウマー */
		}
	}
	if (man->frees < MEMMAN_FREES) {
		/* パターン3: 前後ともまとめる場所がないので新たにスキマをつくる */
		/* free[i]番目より後ろには1つずつ後ろにずれてもらう */
		for (j = man->frees; j > i; j--)
			man->free[j] = man->free[j - 1];
		man->frees++;
		if (man->maxfrees < man->frees)
			man->maxfrees = man->frees;	/* 最大値更新 */
			man->free[i].addr = addr;
			man->free[i].size = size;
			return 0;	/* ウマー */
	}
	/* パターン4:どうにもできなかったので「もったいないけど捨ててしまおう」 */
	man->losts++;
	man->lostsize += size;
	return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
