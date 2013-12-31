/* bootpack�̃��C�� */

#include "bootpack.h"
#include <stdio.h>

#define KEYCMD_LED	0xed

void close_constask(struct TASK *task);
void close_console(struct SHEET *sht);
struct SHEET *search_sheet(struct SHTCTL *shtctl, int mx, int my);
struct SHEET *open_omnaomi(struct SHTCTL *shtctl, unsigned int memtotal, struct SHEET *oldsht);

void HariMain(void)
{
	/* System */
		struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
		struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
		unsigned int memtotal;
		struct FIFO32 fifo, keycmd;
		int fifobuf[128], keycmd_buf[32];
		struct TASK *task_a, *task;
		struct FILEINFO *finfo;
		int *fat;
		int minlist[MAX_MINID];
		int bpp = get_bpp();
	/* Keyboard */
		struct MOUSE_DEC mdec;
		static char keytable0[0x80] = {
			/* �ʏ�̕��� */
			  0, 0x01, '1',  '2', '3', '4', '5', '6', '7', '8', '9', '0',  '-',  '^', 0x08, 0x09,
			'Q',  'W', 'E',  'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a,    0,  'A',  'S',
			'D',  'F', 'G',  'H', 'J', 'K', 'L', ';', ':',   0,   0, ']',  'Z',  'X',  'C',  'V',
			'B',  'N', 'M',  ',', '.', '/',   0, '*',   0, ' ',   0,   0,    0,    0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0, '7', '8', '9', '-', '4',  '5',  '6',  '+',  '1',
			'2',  '3', '0',  '.',   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,
			  0,    0,   0, 0x5c,   0,   0,   0,   0,   0,   0,   0,   0,    0, 0x5c,    0,    0
		};
		static char keytable1[0x80] = {
			/* Shift����������� */
			  0,    0, '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~',  '=', '~', 0x08, 0x09,
			'Q',  'W', 'E',  'R', 'T', 'Y', 'U', 'I',  'O', 'P', '`', '{', 0x0a,   0,  'A',  'S',
			'D',  'F', 'G',  'H', 'J', 'K', 'L', '+',  '*',   0,   0, '}',  'Z', 'X',  'C',  'V',
			'B',  'N', 'M',  '<', '>', '?',   0, '*',    0, ' ',   0,   0,    0,   0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0, '7',  '8', '9', '-', '4',  '5', '6',  '+',  '1',
			'2',  '3', '0',  '.',   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,    0,    0,
			  0,    0,   0,    0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,    0,    0,
			  0,    0,   0,  '_',   0,   0,   0,   0,    0,   0,   0,   0,    0, '|',    0,    0
		};
		int key_shift = 0, key_ctrl = 0, key_alt = 0, flag_e0 = 0;
		int key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
	/* Mouse */
		int mx, my, x = 0, y = 0, mmx = -1, mmy = -1, new_mx = -1, new_my = 0;
		int mmx2 = 0, new_wx = 0x7fffffff, new_wy = 0, flag_mouse = 0;
	/* IME */
		unsigned char *nihongo;
		extern char hankaku[4096];
		int oldlang;
	/* Sheets */
		struct SHTCTL *shtctl;
		unsigned char *buf_back, *buf_mouse, *buf_naomi;
		struct SHEET  *sht_back, *sht_mouse, *sht_naomi, *sht = 0, *key_win, *sht2 = 0;
	/* Balloon */
		unsigned char *buf_balloon;
		struct SHEET  *sht_balloon;
		struct BALLOON *startmenu;
		int flag_balloon = 0;
	/* Misc */
		unsigned char s[40];
		int i, j;


	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC�̏��������I������̂�CPU�̊��荞�݋֎~������ */
	fifo32_init(&fifo, 128, fifobuf, 0);
	*((int *) 0x0fec) = (int) &fifo;
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* PIT��PIC1�ƃL�[�{�[�h������(11111000) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /*	0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	*((int *) 0x0fe2) = (int) memtotal;

	init_timerctl();
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	*((int *) 0x0fe4) = (int) shtctl;
	task_a->langmode = 0;

	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny * (bpp >> 3));
	sheet_setbuf(sht_back,  buf_back,  binfo->scrnx, binfo->scrny, -1);	/* �����F�Ȃ� */
	init_screen((unsigned int *) buf_back, binfo->scrnx, binfo->scrny);
	sht_back->flags |= 0x01;	/* �w�i */

	/* sht_naomi */
	sht_naomi = sheet_alloc(shtctl);
	buf_naomi = (unsigned char *) memman_alloc_4k(memman, 28 * 26 * (bpp >> 3));
	sheet_setbuf(sht_naomi, buf_naomi, 28, 26, get_color(bpp, 0x008484));	/* �����F���� */
	put_naomi((unsigned int *) buf_naomi);
	sht_naomi->flags |= 0x100;	/* ����E�B���h�E */

	/* sht_balloon */
	sht_balloon = sheet_alloc(shtctl);
	buf_balloon = (unsigned char *) memman_alloc_4k(memman, 124 * 23 * (bpp >> 3));
	sheet_setbuf(sht_balloon, buf_balloon, 124, 23, 99);	/* �����F���� */
	startmenu = make_balloon(memman, sht_balloon, 0, 0, 6, 19, 1);
	putminifonts_asc_sht(sht_balloon, 4, 3, 0x000000, 0xffffff, "My name is naomisan", 19);
	sht_balloon->flags |= 0x100;	/* ����E�B���h�E */

	/* sht_cons */
	key_win = open_console(shtctl, memtotal);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	buf_mouse = (unsigned char *) memman_alloc_4k(memman, 16 * 16 * (bpp >> 3));
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);	/* �����F�ԍ���99 */
	init_mouse_cursor((unsigned int *) buf_mouse, 99/* �w�i�F */);
	mx = (binfo->scrnx - 16) / 2; /* ��ʒ����ɂȂ�悤�ɍ��W�v�Z */
	my = (binfo->scrny - 16) / 2;

	sheet_slide(sht_back,  0,  0);
	sheet_slide(key_win,   8, 16);
	sheet_slide(sht_balloon, binfo->scrnx - 130, binfo->scrny - 70);
	sheet_slide(sht_naomi, binfo->scrnx - 50, binfo->scrny - 40);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,  0);
	sheet_updown(key_win,   1);
	sheet_updown(sht_naomi, 2);
	sheet_updown(sht_mouse, 3);
	keywin_on(key_win);

	/* �ŏ��̃L�[�{�[�h��Ԃ�ݒ肵�Ă��܂� */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	/* nihongo.fnt�̓ǂݍ��� */
	fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	finfo = file_search("nihongo.fnt", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0) {
		i = finfo->size;
		nihongo = file_loadfile2(finfo->clustno, &i, fat);
	} else {
		nihongo = (unsigned char *) memman_alloc_4k(memman, 16 * 256 + 32 * 94 * 47);
		for (i = 16 * 256; i < 16 * 256 + 32 * 94 * 47; i++) {
			nihongo[i] = 0xff;	/* �S�p������0xff�Ŗ��ߐs���� */
		}
	}
	for (i = 0; i < 16 * 256; i++) {
		nihongo[i] = hankaku[i];	/* ���p�������R�s�[ */
	}
	*((int *) 0x0fe8) = (int) nihongo;

	/* startup */
	finfo = file_search("osakkie.hsf", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0) {
		char *ss;
		i = finfo->size;
		ss = file_loadfile2(finfo->clustno, &i, fat);
		for (x = 0; x < i; x++) {
			if (ss[x] != 0x0d) {
				fifo32_put(&key_win->task->fifo, ss[x] + 256);
			}
		}
		memman_free_4k(memman, (int) ss, i);
	}

	memman_free_4k(memman, (int) fat, 4 * 2880);

	/* minlist�̏����� */
	for(i = 0; i < MAX_MINID; i++) {
		minlist[i] = -1;		/* ���g�p */
	}

	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* �L�[�{�[�h�ɑ���f�[�^������Α��� */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			/* fifo���J���b�|�ɂȂ����̂ŁA�ۗ����Ă���`�悪����Ύ��s */
			if (new_mx >= 0) {
				io_sti();
				sheet_slide(sht_mouse, new_mx, new_my);
				new_mx = -1;
			} else if (new_wx != 0x7fffffff) {
				io_sti();
				sheet_slide(sht, new_wx, new_wy);
				new_wx = 0x7fffffff;
			} else {
				task_sleep(task_a);
				io_sti();
			}
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win != 0 && key_win->flags == 0) {
				/* ���̓E�B���h�E������ꂽ */
				if (shtctl->top == 2) {
					/* �����}�E�X�Ɣw�i�����Ȃ� */
					key_win = 0;
				} else {
					key_win = shtctl->sheets[shtctl->top - 2];
					keywin_on(key_win);
				}
			}
			if (256 <= i && i < 512) {
				/* �L�[�{�[�h */
				i -= 256;	// ��Ɉ����Ƃ�
				if (i == 0xe0) {
					/* E0�g���L�[ */
					flag_e0 = 1;
				}
				if (i < 0x80 && flag_e0 == 0) {
					/* �L�[�R�[�h�𕶎��R�[�h�ɕϊ� */
					if (key_shift == 0) {
						s[0] = keytable0[i];
					} else if (key_shift == 1) {
						s[0] = keytable1[i];
					}
				} else {
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z')	{
					/* �A���t�@�x�b�g�����͂��ꂽ */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
						((key_leds & 4) != 0 && key_shift != 0)   ) {
						s[0] += 0x20;	/* �������ɕϊ� */
					}
				}
				if (flag_e0 == 1) {
					if (i == 0x35 || i == 0x1c) {
						/* /��Enter */
						s[0] = keytable0[i];
					}
				}
				if (0x47 <= i && i <= 0x53) {
					if (i != 0x4a && i != 0x4e) {
						/* �e���L�[��Ins,Del,Home,End,PgUp,PgDn,�J�[�\���L�[*/
						if (flag_e0 == 1 || (key_leds & 2) != 0) {
							s[0] = keytable0[i] + 0x80;		// �ꗥ��0x80�𑫂�
						}
					}
				}
				if (i == 0x0f && key_alt != 0) {
					/* Alt+Tab�L�[�̓R�[�h���M�ΏۊO */
					s[0] = 0;
				}
				if (s[0] != 0 && key_win != 0) {
					/* ���ʂ̕���, Enter, BackSpace, Tab */
					if (key_ctrl == 0) {
						/* �R���\�[���� */
						fifo32_put(&key_win->task->fifo, s[0] + 256);
					}
				}
				if (i == 0x57 && shtctl->top > 2) {
					/* F11�L�[ */
					sheet_updown(shtctl->sheets[1], shtctl->top - 2);
				}
				if (i == 0x2a)
					key_shift |= 1;			// ���V�t�gon
				if (i == 0x36)
					key_shift |= 2;			// �E�V�t�gon
				if (i == 0xaa)
					key_shift &= ~1;		// ���V�t�goff
				if (i == 0xb6)
					key_shift &= ~2;		// �E�V�t�goff
				if (i == 0x1d) {
					if (flag_e0 == 0) {
						key_ctrl |= 1;		// ��CTRLon
					} else {
						key_ctrl |= 2;		// �ECTRLon
					}
				}
				if (i == 0x9d) {
					if (flag_e0 == 0) {
						key_ctrl &= ~1;		// ��CTRLoff
					} else {
						key_ctrl &= ~2;		// �ECTRLoff
					}
				}
				if (i == 0x38) {
					if (flag_e0 == 0) {
						key_alt |= 1;		// ��ALTon
					} else {
						key_alt |= 2;		// �EALTon
					}
				}
				if (i == 0xb8) {
					if (flag_e0 == 0) {
						key_alt &= ~1;		// ��ALToff
					} else {
						key_alt &= ~2;		// �EALToff
					}
				}
				if (i == 0x3a) {
					/* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 0x45) {
					/* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 0x46) {
					/* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (((i == 0x0f && key_alt != 0) || i == 0x43) && key_win != 0) {
					/* Tab ���� Alt + Tab or F9 */
					keywin_off(key_win);
					j = key_win->height - 1;
					if (j == 0) {
						j = shtctl->top - 2;
					}
					key_win = shtctl->sheets[j];
					keywin_on(key_win);
				}
				if (i == 0x2e && key_ctrl != 0 && key_win != 0) {
					/* Shift + F1 ���� Ctrl + C */
					task = key_win->task;
					if (task != 0 && task->tss.ss0 != 0) {
						task->langmode = task->langmode0;	/* API�ŕύX����langmode�����ɖ߂� */
						oldlang = task_a->langmode;		/* task_a��langmode���ꎞ�I�Ƀ^�X�N�ɂ��킹�� */
						task_a->langmode = task->langmode;
						cons_putstr0(task->cons, "\nBreak(key) :\n");
						task_a->langmode = oldlang;
						io_cli();
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						task_run(task, -1, 0);
					}
				}
				if (i == 0x31 && key_ctrl != 0) {
					/* Shift + F2 ���� Ctrl + N */
					/* �V����������R���\�[�����A�N�e�B�u�� */
					keywin_off(key_win);
					key_win = open_console(shtctl, memtotal);
					sheet_slide(key_win, 8, 16);
					sheet_updown(key_win, shtctl->top - 1);
					keywin_on(key_win);
				}
				if (i != 0xe0 && flag_e0 == 1) {
					/* e0�g���L�[�I�� */
					flag_e0 = 0;
				}
				if (i == 0xfa) {
					/* �L�[�{�[�h���f�[�^���󂯎���� */
					keycmd_wait = -1;
				}
				if (i == 0xfe) {
					/* �L�[�{�[�h���f�[�^���󂯎��Ȃ����� */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
			} else if (512 <= i && i < 768) {
				/* �}�E�X */
				if (mouse_decode(&mdec, i - 512)) {
					/* �}�E�X�ړ� */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
						mx = 0;
					if (my < 0)
						my = 0;
					if (mx > binfo->scrnx - 1)
						mx = binfo->scrnx - 1;
					if (my > binfo->scrny - 1)
						my = binfo->scrny - 1;
					new_mx = mx;
					new_my = my;
					if ((mdec.btn != 0 || flag_mouse != 0) && mmx < 0) {
						/* make��break�����Ă��� */
						/* ��̉��~�����珇�Ƀ}�E�X�������V�[�g��T�� */
						sht = search_sheet(shtctl, mx, my);
						x = mx - sht->vx0;
						y = my - sht->vy0;
						if (mdec.btn != 0 && sht != sht_naomi) {
							sheet_updown(sht_balloon, -1);	/* �o���[�������� */
							flag_balloon = 0;
						}
					}
					if (mdec.btn & 0x01) {
						/* ���{�^�� */
						flag_mouse |= 0x01;
						if (mmx < 0) {
							/* �ʏ탂�[�h */
							if ((sht->flags & 0x100) != 0) {
								/* �i�I�~���񂩃X�^�[�g���j���[ */
								if (sht == sht_naomi) {
									if (flag_balloon == 0) {
										flag_balloon = 1;
										sheet_slide(sht_balloon, sht_naomi->vx0 - 80, sht_naomi->vy0 - 30);
										sheet_updown(sht_balloon, shtctl->top - 1);
									}
									if (flag_balloon == 2) {
										flag_balloon = 3;
										sheet_updown(sht_balloon, -1);
									}
								}
							}
							if ((sht->flags & 0xf00) == 0x200) {
								/* �ŏ������̃E�B���h�E */
								flag_mouse |= 0x0130;
							}
							if ((sht->flags & 0xf00) == 0) {
								/* �E�B���h�E�Ƃ� */
								sheet_updown(sht, shtctl->top - 2);
								if (sht != key_win && sht != 0) {
									keywin_off(key_win);
									key_win = sht;
									keywin_on(key_win);
								}
								/* ��ʃE�B���h�E */
								if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
									mmx = mx;	/* �E�B���h�E�ړ����[�h�� */
									mmy = my;
									mmx2 = sht->vx0;
									new_wy = sht->vy0;
								}
								if (5 <= y && y < 19) {
									if (sht->bxsize - 20 <= x && x < sht->bxsize - 5) {
										/* [X]�{�^�� */
										flag_mouse |= 0x0110;
									}
									if (sht->bxsize - 31 <= x && x < sht->bxsize - 21) {
										/* [_]�{�^�� */
										flag_mouse |= 0x0120;
									}
								}
							}
						} else {
							if ((flag_mouse & 0x0f00) == 0) {
								x = mx - mmx;	/* �ړ��ʂ��v�Z */
								y = my - mmy;
								/* �{�^���Ȃǂ������Ă��Ȃ� */
								/* �E�B���h�E�ړ����[�h */
								new_wx = (mmx2 + x + 2) & ~3;
								new_wy = new_wy + y;
								mmy = my;		/* �ړ���̍��W�ɍX�V */
							}
						}
					} else if (mdec.btn & 0x02) {
						/* �E�N���b�N */
						flag_mouse |= 0x02;
						if (mmx < 0) {
							if (sht == sht_naomi || (sht->flags & 0x200) != 0) {
								mmx = mx;	/* �ړ����[�h�� */
								mmy = my;
								mmx2 = sht->vx0;
								new_wy = sht->vy0;
							}
						} else {
							if ((flag_mouse & 0x0f00) == 0) {
								x = mx - mmx;	/* �ړ��ʂ��v�Z */
								y = my - mmy;
								/* �{�^���Ȃǂ������Ă��Ȃ� */
								/* �E�B���h�E�ړ����[�h */
								new_wx = (mmx2 + x + 2) & ~3;
								new_wy = new_wy + y;
								mmy = my;		/* �ړ���̍��W�ɍX�V */
							}
						}
					} else if (mdec.btn == 0) {
						/* break���A�Ȃɂ��Ȃ� */
						if (flag_mouse & 0x01) {
							/* ��break */
							if ((flag_mouse & 0x0f00) == 0x0100 && 5 <= y && y < 19) {
								/* �E�B���h�E�̃{�^�� */
								if ((flag_mouse & 0xf0) == 0x10 && sht->bxsize - 20 <= x && x < sht->bxsize - 5) {
									/* �E�B���h�E�N���[�Y */
									task = sht->task;
									if ((sht->flags & 0x10) != 0) {
										/* �A�v����������E�B���h�E���H */
										task->langmode = task->langmode0;	/* API�ŕύX����langmode�����ɖ߂� */
										oldlang = task_a->langmode;		/* task_a��langmode���ꎞ�I�Ƀ^�X�N�ɂ��킹�� */
										task_a->langmode = task->langmode;
										cons_putstr0(task->cons, "\nBreak(mouse) :\n");
										task_a->langmode = oldlang;
										io_cli();
										task->tss.eax = (int) &(task->tss.esp0);
										task->tss.eip = (int) asm_end_app;
										io_sti();
										task_run(task, -1, 0);
									} else {
										/* Console */
										sheet_updown(sht, -1);	/* �Ƃ肠������\���� */
										keywin_off(key_win);
										key_win = 0;
										if (shtctl->top > 2) {
											key_win = shtctl->sheets[shtctl->top - 2];
											keywin_on(key_win);
										}
									}
								}
								if ((flag_mouse & 0xf0) == 0x20 && sht->bxsize - 31 <= x && x < sht->bxsize - 21) {
									/* [_]�{�^���N���b�N */
									sheet_updown(sht, -100);	/* ��\���� */
									keywin_off(key_win);
									key_win = 0;
									sht2 = open_omnaomi(shtctl, memtotal, sht);
									sheet_slide(sht2, sht->vx0 + sht->bxsize - 45, sht->vy0);
									if (shtctl->top > 2) {
										key_win = shtctl->sheets[shtctl->top - 2];
										keywin_on(key_win);
									}
									sheet_updown(sht2, 1);
									for(j = 0; j < MAX_MINID; j++) {
										if (minlist[j] < 0) {
											minlist[j] = 1;		/* min_id�g�p�� */
											sht2->min_id = j;	/* �����D�H */
											sht->min_id = j;
											break;
										}
									}
								}
								if ((flag_mouse & 0xf0) == 0x30) {
									/* �I�����c�i�I�~���N���b�N */
									keywin_off(key_win);
									for (j = 0; j < MAX_SHEETS; j++) {
										key_win = &shtctl->sheets0[j];
										if (key_win->height < 0) {
											if (key_win->min_id == sht->min_id) {
												minlist[sht->min_id] = -1;
												sht->min_id = -1;
												key_win->min_id = -1;
												sheet_free(sht);
												sheet_slide(key_win, sht->vx0 - key_win->bxsize + 42, sht->vy0);
												sheet_updown(key_win, shtctl->top - 1);
												keywin_on(key_win);
												break;
											}
										}
									}
								}
							}
							/* �i�I�~���񂩃X�^�[�g���j���[ */
							if (flag_balloon == 1) {
								flag_balloon = 2;
							} else if (flag_balloon == 3) {
								flag_balloon = 0;
							}
						} else if (flag_mouse & 0x02) {
							/* �Ebreak */
						}
						flag_mouse = 0;
						mmx = -1;	/* �ʏ탂�[�h */
						if (new_wx != 0x7fffffff) { 
							sheet_slide(sht, new_wx, new_wy);	/* ��x�m�肳���� */
							new_wx = 0x7fffffff;
						}
					}
				}
			} else if (768 <= i && i < 1024) {
				close_console(shtctl->sheets0 + (i - 768));
			} else if (1024 <= i && i < 2024) {
				close_constask(taskctl->tasks0 + (i - 1024));
			} else if (2024 <= i && i < 2280) {
				/* �R���\�[���������� */
				sht2 = shtctl->sheets0 + (i - 2024);
				memman_free_4k(memman, (int) sht2->buf, 256 * 165);
				sheet_free(sht2);
			} else if (i == 0x4000) {
				/* key_win�ύX�v�� from api.c */
				key_win = shtctl->sheets[shtctl->top - 2];
			}
		}
	}
}

void keywin_off(struct SHEET *key_win)
{
	change_wtitle(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 3);	/* �R���\�[���̃J�[�\��OFF */
	}
	return;
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 2);	/* �R���\�[���̃J�[�\��ON */
	}
	return;
}

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = task->cons_stack + 64 * 1024 - 12;
	task->tss.eip = (int) &console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *) (task->tss.esp + 4)) = (int) sht;
	*((int *) (task->tss.esp + 8)) = memtotal;
	task_run(task, 2, 2);
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return task;
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	int bpp = get_bpp();
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 256 * 165 * (bpp >> 3));
	sheet_setbuf(sht, buf, 256, 165, -1);	/* �����F�Ȃ� */
	make_window((unsigned int *) buf, 256, 165, "Console", 0/* �i�I�~���� */, 0);
	make_textbox(sht, 8, 28, 240, 128, 0x000000);
	sht->task = open_constask(sht, memtotal);
	sht->flags |= 0x20;	/* �J�[�\������ */
	return sht;
}

struct SHEET *open_omnaomi(struct SHTCTL *shtctl, unsigned int memtotal, struct SHEET *oldsht)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	int bpp = get_bpp();
	unsigned char *buf;

	buf = (unsigned char *) memman_alloc_4k(memman, 31 * 16 * (bpp >> 3));
	sheet_setbuf(sht, buf, 31, 16, 99);
	make_omnaomi(sht, 0, 0);
	sht->flags |= 0x0200;
	return sht;
}

void close_constask(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
	task->flags = 0;	/* task_free(task);�̑��� */
	
}

void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = sht->task;
	memman_free_4k(memman, (int) sht->buf, 256 * 165);
	sheet_free(sht);
	close_constask(task);
	return;
}

struct SHEET *search_sheet(struct SHTCTL *shtctl, int mx, int my)
{
	struct SHEET *sht = 0;
	int i, x, y;
	for (i = shtctl->top - 1; i > 0; i--) {
		sht = shtctl->sheets[i];
		x = mx - sht->vx0;
		y = my - sht->vy0;
		if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
			if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
				return sht;
			}
		}
	}
	return 0;
}
