
# ファイル生成規則
img: make -r helloos.img

ipl.bin: ipl.nas Makefile
	../../../../tolsetOSX/z_tools/nask \
	ipl.nas ipl.bin ipl.lst

helloos.img: ipl.bin Makefile
	../../../../tolsetOSX/z_tools/edimg imgin:../../../../tolsetOSX/z_tools/fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0 imgout:helloos.img



