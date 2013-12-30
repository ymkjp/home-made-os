OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj minifnt.obj graphic.obj \
		dsctbl.obj int.obj fifo.obj keyboard.obj mouse.obj memory.obj sheet.obj \
		timer.obj mtask.obj console.obj window.obj file.obj tek.obj api.obj \
		picture.obj bmp.obj jpeg.obj ico.obj rtc.obj

TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = make -r
NASK     = $(TOOLPATH)nask
CC1      = $(TOOLPATH)gocc1 -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask -a
OBJ2BIM  = $(TOOLPATH)obj2bim
MAKEFONT = $(TOOLPATH)makefont
MAKEFNT6 = ../font/makefnt6
BIN2OBJ  = $(TOOLPATH)bin2obj
BIM2HRB  = $(TOOLPATH)bim2hrb
BIM2BIN  = $(TOOLPATH)bim2bin
RULEFILE = haribote.rul
EDIMG    = $(TOOLPATH)edimg
IMGTOL   = $(TOOLPATH)imgtol
GOLIB    = $(TOOLPATH)golib00
COPY     = cp
MOVE     = mv
DEL      = rm -f


# デフォルト動作

default :
	$(MAKE) ipl.bin
	$(MAKE) osakkie.sys

# ファイル生成規則

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

minifnt.bin : minifnt.txt Makefile
	$(MAKEFNT6) minifnt.txt minifnt.bin

minifnt.obj : minifnt.bin Makefile
	$(BIN2OBJ) minifnt.bin minifnt.obj _minifnt

bmp.obj : bmpobj.bak Makefile
	$(COPY) bmpobj.bak bmp.obj

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

osakkie.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb >osakkie.sys
	$(BIM2BIN) -osacmp -tek5 in:bootpack.hrb out:../osakkie.bhs


# 一般規則

%.gas : %.c bootpack.h Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst


# コマンド

clean :
	-$(DEL) asmhead.bin
	-$(DEL) hankaku.bin
	-$(DEL) minifnt.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb

cleanall :
	$(MAKE) clean
	-$(DEL) ipl.bin
	-$(DEL) osakkie.sys
