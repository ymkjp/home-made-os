$APP_LIST = \
	apps/1line/1line.hrb \
	apps/bball/bball.hrb \
	apps/bonno/bonno.hrb \
	apps/calc/calc.hrb \
	apps/clock/clock.hrb \
	apps/color/color.hrb \
	apps/csvv/csvv.hrb \
	apps/cvtg/cvtg.hrb \
	apps/gview/gview.hrb \
	apps/invader/invader.hrb \
	apps/keyview/keyview.hrb \
	apps/kuha/kuha.hrb \
	apps/luckynum/luckynum.hrb \
	apps/menu/menu.hrb \
	apps/mmlplay/mmlplay.hrb \
	apps/mtorz/mtorz.hrb \
	apps/noodle/noodle.hrb \
	apps/osakcnt/osakcnt.hrb \
	apps/stbeep/stbeep.hrb \
	apps/tview/tview.hrb \
	apps/type/type.hrb \
	apps/walk/walk.hrb \
	apps/winhello/winhello.hrb

TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = make -r
EDIMG    = $(TOOLPATH)edimg
IMGTOL   = $(TOOLPATH)imgtol
COPY     = cp
MOVE     = mv
DEL      = rm -f

# デフォルト動作

default :
	$(MAKE) img

# ファイル生成規則

osakkie.img : osakkie/ipl.bin osakkie/osakkie.sys Makefile $(APP_LIST)
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:osakkie/ipl.bin len:512 from:0 to:0 \
		copy from:osakkie/osakkie.sys to:@: \
		copy from:osakkie/nihongo.fnt to:@: \
		copy from:apps/1line/1line.hrb to:@: \
		copy from:apps/bball/bball.hrb to:@: \
		copy from:apps/bonno/bonno.hrb to:@: \
		copy from:apps/calc/calc.hrb to:@: \
		copy from:apps/clock/clock.hrb to:@: \
		copy from:apps/color/color.hrb to:@: \
		copy from:apps/csvv/csvv.hrb to:@: \
		copy from:apps/cvtg/cvtg.hrb to:@: \
		copy from:apps/gview/gview.hrb to:@: \
		copy from:apps/invader/invader.hrb to:@: \
		copy from:apps/keyview/keyview.hrb to:@: \
		copy from:apps/kuha/kuha.hrb to:@: \
		copy from:apps/luckynum/luckynum.hrb to:@: \
		copy from:apps/menu/menu.hrb to:@: \
		copy from:apps/mmlplay/mmlplay.hrb to:@: \
		copy from:apps/mtorz/mtorz.hrb to:@: \
		copy from:apps/noodle/noodle.hrb to:@: \
		copy from:apps/osakcnt/osakcnt.hrb to:@: \
		copy from:apps/stbeep/stbeep.hrb to:@: \
		copy from:apps/type/type.hrb to:@: \
		copy from:apps/tview/tview.hrb to:@: \
		copy from:apps/walk/walk.hrb to:@: \
		copy from:apps/winhello/winhello.hrb to:@: \
		copy from:media/daigo.mml to:@: \
		copy from:media/daiku.mml to:@: \
		copy from:media/fujisan.mml to:@: \
		copy from:media/kirakira.mml to:@: \
		copy from:media/sample.txt to:@: \
		copy from:media/sample.csv to:@: \
		copy from:media/sample2.csv to:@: \
		copy from:media/maro.jpg to:@: \
		copy from:media/nyanpo.ico to:@: \
		copy from:media/shuri.jpg to:@: \
		copy from:media/stbeep.bmp to:@: \
		copy from:osakkie.hsf to:@: \
		imgout:osakkie.img

# コマンド

all :
	$(MAKE) -C osakkie
	$(MAKE) -C apilib
	$(MAKE) -C apps

os :
	-$(DEL) osakkie.img
	$(MAKE) -C osakkie
api :
	-$(DEL) osakkie.img
	$(MAKE) -C apilib
app :
	-$(DEL) osakkie.img
	$(MAKE) -C apps

img :
	$(MAKE) osakkie.img

run :
	-$(DEL) osakkie.img
	$(MAKE) img
	$(COPY) osakkie.img ../z_tools/qemu/fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: osakkie.img

clean_all :
	$(MAKE) -C osakkie clean
	$(MAKE) -C apilib  clean
	$(MAKE) -C apps    clean

clean_os :
	$(MAKE) -C osakkie clean
clean_api :
	$(MAKE) -C apilib  clean
clean_app :
	$(MAKE) -C apps    clean

cleanall_all :
	$(MAKE) -C osakkie cleanall
	$(MAKE) -C apilib  cleanall
	$(MAKE) -C apps    cleanall

cleanall_os :
	$(MAKE) -C osakkie cleanall
cleanall_api :
	$(MAKE) -C apilib  cleanall
cleanall_app :
	$(MAKE) -C apps    cleanall

clean_img :
	-$(DEL) osakkie.bhs
	-$(DEL) osakkie.img
