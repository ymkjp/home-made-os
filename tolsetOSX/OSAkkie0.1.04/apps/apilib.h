void api_putchar(int c);
void api_putstr0(char *s);
void api_putstr1(char *s, int l);
void api_end(void);
int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_point(int win, int x, int y, int c);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_closewin(int win);
int api_getkey(int mode);
int api_alloctimer(void);
int api_inittimer(int timer, int data);
int api_settimer(int timer, int time);
int api_freetimer(int timer);
int api_beep(int tone);
int api_fopen(char *fname);
int api_fclose(int fhandle);
int api_fseek(int fhandle, int offset, int mode);
int api_fsize(int fhandle, int mode);
int api_fread(char *buf, int maxsize, int fhandle);
int api_cmdline(char *buf, int maxsize);
int api_getlang(void);
int api_sendkey(char *);

/* Haritomo common API */
struct SYS_INFO {
	unsigned char cyls;		/* ブートセクタはどこまでディスクを読んだのか */
	unsigned char leds;		/* ブート時のキーボードのLEDの状態 */
	unsigned char vmode;	/* ビデオモード  何ビットカラーか */
	unsigned char reserve;	/* 予約 */
	unsigned short scrnx;	/* 画面解像度 */
	unsigned short scrny;	/* 画面解像度 */
	unsigned char *vram;	/* VRAMのアドレス */
	unsigned short os_type;	/* OSの種別 */
};
struct TIME_INFO {
	int year, month, day, hour, minutes, second;
};
int tomo_gettick(void);
void tomo_setlang(int land);
void tomo_sysinfo(struct SYS_INFO *info);
void tomo_systime(struct TIME_INFO *info);

/* OSAkkie API */
void osak_putministrwin(int win, int x, int y, int col, int len, char *str);
void osak_exec(char *name);
int osak_getbuflen(void);
int osak_getcolor(int bpp, int col);
int osak_initpicture(char *name);
int osak_drawpicture(int win, int pic, int px, int py);
void osak_freepicture(int pic);
int osak_gettime(int type);
/* 0x4100 series */
void osak_putstrwin(int win, int x, int y, int col, int len, char *str);
void osak_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void osak_point(int win, int x, int y, int c);
void osak_linewin(int win, int x0, int y0, int x1, int y1, int col);
