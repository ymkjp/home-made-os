/* verコマンド(console.c)で表示される文字 */
#define OSAKKIE_VERSION1	"OSAkkie Ver.0.1.04\n"
#define OSAKKIE_VERSION2	"Copyright(C) 2006-2007 Akkiesoft.\n\n"

/* asmhead.nas */
struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; /* ブートセクタはどこまでディスクを読んだのか */
	char leds; /* ブート時のキーボードのLEDの状態 */
	char vmode; /* ビデオモード  何ビットカラーか */
	char reserve;
	short scrnx, scrny; /* 画面解像度 */
	unsigned char *vram;
};
#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void asm_inthandler0c(void);
void asm_inthandler0d(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void asm_hrb_api(void);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
void asm_end_app(void);

/* fifo.c */
struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
	struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
int get_bpp(void);
int get_color(int bpp, int col);
void boxfill(unsigned int *vram, int xsize, int c, int x0, int y0, int x1, int y1);
void init_screen(unsigned int *vram, int x, int y);
void putfont(unsigned int *vram, int xsize, int x, int y, int c, char *font);
void putfonts(unsigned int *vram, int xsize, int x, int y, int c, unsigned char *s);
void init_mouse_cursor(unsigned int *mouse, int bc);
void putminifont(unsigned int *vram, int xsize, int x, int y, int c, char *font);
void putminifonts(unsigned int *vram, int xsize, int x, int y, int c, unsigned char *s);
void put_naomi(unsigned int *vram);
void picdata(unsigned int *vram, int xsize, int px, int py, unsigned char *data, int sx, int sy, int bc);

/* dsctbl.c */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_LDT			0x0082
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

/* int.c */
void init_pic(void);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);
#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

/* memory.c */
#define MEMMAN_FREES 	4090 		/* 約32KB */
#define MEMMAN_ADDR		0x003c0000	/* MEMMANを置くアドレス */
struct FREEINFO { /* 空き情報 */
	unsigned int addr, size;
};
struct MEMMAN {	/* メモリ管理 */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* sheet.c */
#define MAX_SHEETS 256
struct SHEET {
	unsigned char *buf, *windowname;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags, min_id;
	struct SHTCTL *ctl;
	struct TASK *task;
};
struct SHTCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int sx, int sy, int bgcol);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);

/* timer.c */
#define MAX_TIMER	500
struct TIMER {
	struct TIMER *next;
	unsigned int timeout;
	char flags, flags2;
	struct FIFO32 *fifo;
	int data;
};
struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER *timers0;
};
extern struct TIMERCTL timerctl;
void init_pit(void);
void init_timerctl(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
int timer_cancel(struct TIMER *timer);
void timer_cancelall(struct FIFO32 *fifo);
void inthandler20(int *esp);

/* mtask.c */
#define MAX_TASKS		1000		/* 最大タスク数 */
#define MAX_TASKS_LV	100			/* 1Lvあたりのタスク数 */
#define MAX_TASKLEVELS	10			/* レベルの数 */
#define TASK_GDT0		3			/* TSSをGDTの何番から割り当てるか */
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
struct TASK {
	int sel, flags;	/* selはGDT番号のこと */
	int level, priority;
	struct FIFO32 fifo;
	struct TSS32 tss;
	struct SEGMENT_DESCRIPTOR ldt[2];
	struct CONSOLE *cons;
	int ds_base, cons_stack;
	struct FILEHANDLE *fhandle;
	int *fat;
	char *cmdline;
	unsigned char langmode0, langmode, langbyte1;
	char *titlebar;
};
struct TASKLEVEL {
	int running;	/* 動作中のタスクの数 */
	int now;		/* 現在動作しているタスクがどれかわかるようにするための変数 */
	struct TASK *tasks[MAX_TASKS_LV];
};
struct TASKCTL {
	int now_lv;		/* 現在動作中のレベル */
	char lv_change;	/* 次のタスクスイッチでレベルも変えるかどうか */
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};
extern struct TASKCTL *taskctl;
extern struct TIMER *task_timer;
struct TASK *task_now(void);
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int level, int priority);
void task_switch(void);
void task_sleep(struct TASK *task);

/* window.c */
struct BALLOON {
	struct SHEET *sht;
	int px, py, smode, sx, sy;
};
void make_window(unsigned int *buf, int xsize, int ysize, char *title, int icon, char act);
void make_wtitle(unsigned int *buf, int xsize, char *title, int icon, char act);
void make_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void putfonts_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void change_wtitle(struct SHEET *sht, int act);
struct BALLOON *make_balloon(struct MEMMAN *memman, struct SHEET *sht,
								int px, int py, int smode, int sx, int sy);
void putminifonts_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_omnaomi(struct SHEET *sht, int px, int py);

/* console.c */
struct CONSOLE {
	struct SHEET *sht;
	int curx, cury, curcol;
	struct TIMER *timer;
};
struct FILEHANDLE {
	char *buf;
	int size;
	int pos;
};
void console_task(struct SHEET *sheet, int memtotal);
void cons_putchar(struct CONSOLE *cons, int chr, char move);
void cons_putstr0(struct CONSOLE *cons, char *s);
void cons_putstr1(struct CONSOLE *cons, char *s, int l);
void cons_newline(struct CONSOLE *cons);
void cons_recent(struct CONSOLE *cons, char *cmdline, int n);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, int memtotal);
void cmd_mem(struct CONSOLE *cons, int memtotal);
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
void cmd_exit(struct CONSOLE *cons, int *fat);
void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_langmode(struct CONSOLE *cons, char *cmdline);
void cmd_reboot(struct CONSOLE *cons);
void cmd_ver(struct CONSOLE *cons);
void cmd_history(struct CONSOLE *cons, char *cmdline);
void cmd_wallpaper(struct CONSOLE *cons, char *cmdline, int *fat);
void cmd_time(struct CONSOLE *cons);
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);
int *inthandler0c(int *esp);
int *inthandler0d(int *esp);

/* file.c */
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
char *file_loadfile2(int clustno, int *psize, int *fat);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);

/* tek.c */
int tek_getsize(unsigned char *p);
int tek_decomp(unsigned char *p, char *q, int size);

/* bootpack.c */
#define MAX_MINID 256
struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal);
struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal);
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);

/* picture.c */
struct DLL_STRPICENV {	/* 64KB */
	int work[64 * 1024 / 4];
};
struct RGB {
	unsigned char b, g, r, t;
};
struct PICTURE {
	char *name, *filebuf;
	struct RGB *picbuf;
	struct DLL_STRPICENV *env;
	int fsize, err, info[8];
	/* 
		err list
		1: File not found.
		2: File is not image.
	 */
};
int picture_info(struct DLL_STRPICENV *env, int *info, int size, char *fp);
int picture_decode0(int mode, struct DLL_STRPICENV *env, int size,
					 char *fp, int b_type, char *buf, int skip);
unsigned char rgb2pal(int r, int g, int b, int x, int y);
struct PICTURE *picture_init(char *name, int *fat);
int picture_draw(struct SHEET *sht, struct PICTURE *pic, int px, int py);
void picture_free(struct PICTURE *pic);

/* bmp.nasm */
int info_BMP(struct DLL_STRPICENV *env, int *info, int size, char *fp);
int decode0_BMP(struct DLL_STRPICENV *env, int size, char *fp, int b_type, char *buf, int skip);

/* jpeg.c */
int info_JPEG(struct DLL_STRPICENV *env, int *info, int size, char *fp);
int decode0_JPEG(struct DLL_STRPICENV *env, int size, char *fp, int b_type, char *buf, int skip);

/* ico.c */
int info_ICO(struct DLL_STRPICENV *env, int *info, int size, char *fp);
int decode0_ICO(struct DLL_STRPICENV *env, int size, char *fp, int b_type, char *buf, int skip);

/* api.c */
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col);
struct SYS_INFO {
	unsigned char cyls;     /* ブートセクタはどこまでディスクを読んだのか */
	unsigned char leds;     /* ブート時のキーボードのLEDの状態 */
	unsigned char vmode;     /* ビデオモード  何ビットカラーか */
	unsigned char reserve;  /* 予約 */
	unsigned short scrnx; /* 画面解像度 */
	unsigned short scrny; /* 画面解像度 */
	unsigned char *vram;    /* VRAMのアドレス */
	unsigned short os_type;  /* OSの種別 */
};
struct TIME_INFO {
	int year, month, day, hour, minutes, second;
};

/* rtc.c */
int rtc_get(int type);
