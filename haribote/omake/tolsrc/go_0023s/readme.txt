"go_0020s"�ɂ���                              �썇�G��  kawai@imasy.org

�E����͉����H

  ����́Ago_0020w/go_0020o�̃\�[�X�ł��Bgo_0020w�Ƃ����̂́Agcc-3.2����������
������R���p�C����win32�Ńo�C�i���ł��Bgo_0020o�͂���OSASK�łł��B

  �ׂ������Ƃ������ƁA����go_0020s�́Agcc-3.2�̃\�[�X��go_0020p�̃p�b�`�����Ă�
��̃\�[�X�ł��Bgo_0020s�����őS�Ẵ\�[�X���܂܂�Ă��܂��̂ŁAgo_0020s������
�Ă����go_0020p�͕s�v�ł��B

  �i�o���i�̂܂Ƃ߁j
    go_0020w : win32�Ńo�C�i��               (GPL)
    go_0020o : OSASK�Ńo�C�i��               (GPL)
    go_0020s : go_0020w, o�̃\�[�X           (GPL)
    go_0020p : go_0020s��gcc-3.2�\�[�X�̍��� (KL-01)

�E�ǂ�����āA���s�o�C�i�������̂��H

  make�ɐ旧���Ainclude/rule.mak��ݒ肵�܂��B�܂��f�B���N�g��go_0020s�̒���
setmode go�Ƃ���΁Awin32���GO��make�ł����ԂɂȂ�܂��B�܂�setmode gcc�Ƃ�
��΁Awin32��API�𒼐ڌĂяo�����Ƃ͂��Ȃ���ANSI-C�̕W�����C�u�����������g����
���ɂȂ�܂��B���̏�ԂȂ�MinGW��GO+w32clibc��make�ł����ԂɂȂ�܂��B������
�ɂ��Ă�����rule.mak�͐��`�ł����Ȃ��̂ŁA�e���̊��ɍ��킹�ăp�X�����������
���������iw32clibc���g���Ȃ�Ainclude/rule.mak������Ɏ኱����������K�v������
�ł��傤�j�B

  go�ł̏ꍇ�Ago_0009w������ȍ~��make���܂��B��{�I�ɂ̓f�B���N�g��gcc�̒���
"make"�Ƃ���΁Acpp0.exe�Acc1.exe�Acc1plus.exe��3���ł��܂��B�ڂ������Ƃ�
Makefile����͂��Ă��������B�ȒP�ȍ\���ł�����A�ǂ߂�Ǝv���܂��B3���ł�����
��upx -9 *.exe�Ƃ���΁A���ꂼ��ӂ��킵���T�C�Y�ɂȂ�܂��B

  gcc�ł̏ꍇ�́A�܂����L�̒ʂ�ɁAgolib00�Agas2nask�Anaskcnv0�Asjisconv��stdc
�ł����܂��i�����̃c�[����cpp0�Acc1�Acc1plus�̐����ɕK�v�j�B�����ăf�B���N
�g��gcc�̒���"make"�Ƃ���΁Acpp0.exe�Acc1.exe�Acc1plus.exe��3���ł��܂��B

  �܂��A�f�B���N�g��gcc�̒���"make -r osaskgo.bin"�Ƃ���΁AOSASK�ł̃o�C�i����
�ł��܂��B

  golib00w�Agas2nask�Anask�Asjisconv�Anaskcnv0�ɂ��ẮAtoolw32��toolstdc�̗�
���ɓ����Ă��܂��Bsetmode go�̏ꍇ�́Atoolw32�̒���make�Ƃ���΁A���ꂼ�ꐶ����
��܂��Bsetmode gcc�̏ꍇ�́Atoolstdc�̒���make���܂��B���ꂼ��upx����ƁA�z�z
�Ńo�C�i���ɂȂ�܂��B

�E���C�Z���X�ɂ���

  ���̃\�[�X�Z�b�g��GPL�Ń��C�Z���X���܂��B�����AGPL�ł͂Ȃ�KL-01�Ń��C�Z���X��
�ꂽ�\�[�X���ق����l�́Ago_0020p�̂ق��𓖂����Ă��������B

  �Ȃ��ȉ��̃f�B���N�g���Ɋւ��ẮA��O�I��KL-01(�썇�����C�Z���X-01)�ł��B
    drv_osa, drv_stdc, drv_w32, funcs, go_lib, nasklib, omake, toolstdc,
    toolw32, w32clibc

  GPL�̃��C�Z���X����Copying�ɁAKL-01�̃��C�Z���X���͈ȉ���URL�ɂ���܂��B
        http://www.imasy.org/~mone/kawaido/license01-1.0.html

  GPL�����̒��쌠�ɂ��ẮA���ꂼ��̌�����҂ɋA���܂��BKL-01�����̒��쌠��
���ẮAgo_lib����string�n�֐��Q�̒��쌠��Gaku����ɁA����ȊO�͐썇�ɋA����
���B

�Elibmingw.lib�ɂ���

  libmingw�́AMinGW�̃��C�u����������s�o�C�i�������ɂǂ����Ă��K�v�Ȃ��̂�����
�W�߂��ȈՃ��C�u�����ł��B���̃��C�Z���X�͓��RGPL�ł��B

�E�ӎ�

  gcc�̊J���҂݂̂Ȃ��܂ɐS����̎ӎ����������܂��B��������go�̈ꕔ��gcc�̊J��
�҂̕��X�ɔF�߂Ă�����������A����ȂɊ��������Ƃ͂���܂���B

  Gaku�����string���C�u�������g�킹�Ă��������܂����BGaku����A���肪�Ƃ�����
���܂��B

  �܂��J���𒼐ڏ����Ă��ꂽ�A���[�݂񂳂�Ahenoheno����A������OSASK�R�~���j�e
�B�[�݂̂Ȃ��܁A���肪�Ƃ��������܂����B

  �Ȃ��A�썇��2002.10.03�`2003.02.28�̊��Ԃ̂�����J�����ʂ́A���ʔF�@�l ���
�����U�����Ƌ��� (IPA) �́u�����\�t�g�E�F�A�n�����Ɩ������[�X�v�ł̈ϑ��Ɩ��ɂ�
����̂ł��B���̏���؂�Ďx�����������������Ƃɂ������Ă���\���グ�܂��B

�E�ǂ����ς�������H

  �@�\�I�ȈႢ�ɂ��ẮA�o�C�i���ł��Q�Ƃ��Ă��������B�����ł́A�\�[�X���x��
�ł̈Ⴂ�����Ɍ��y���Ă��܂��B

< go_0000s �� go_0001s >

  include�f�B���N�g������2�̃t�@�C�����폜���܂����B�����make�ɕs�v�ȃt�@�C
���ł����B�\�[�X���̉��s�R�[�h���ACRLF��LF�݂̂̂��̂����݂��Ă��܂����BLF�̂�
�ɓ��ꂵ�܂����Bdelaln32�̃o�C�i���ƃ\�[�X���ꉞ����Ă������Ƃɂ��܂����B

< go_0001s �� go_0002s >

  �h���C�o(drv_stdc)���̃t�@�C���ȊO�̑S�ẮA�O���̃C���N���[�h�t�@�C����K�v
�Ƃ��܂���B�����������Ă��郉�C�u���������œ����Ă��܂��B�K�v�Ȃ͈̂ȉ���5�֐�
main0, GOL_open, GOL_close, GOL_stepdir, GOL_sysabort
�����ł��B�����͊��Ɉˑ����ď��������̂ł��B�����́A�������A�W�����C
�u�����݂̂ŋL�q���邱�Ƃ��ł��A�����drv_stdc�ɓ����Ă��܂��B����́A
malloc, fopen, fseek, ftell, fread, fclose, fwrite, fputs
�����ŏ�����Ă��܂��B
  �h���C�o��callmain.c�������S�Ẵt�@�C���́Ago_0002w�ŃR���p�C���ł��܂��B
���Ɉˑ�����h���C�o��callmain.c��MinGW�ŃR���p�C������K�v������܂��B
  ���킵����Makefile���Q�Ƃ��Ă��������B

< go_0002s �� go_0004s >

  stdc�h���C�o�̂ق��ɁAw32�h���C�o��p�ӂ��܂����B�����MinGW�������Ă��Ȃ���
���R���p�C���ł��܂��B�܂�stdc�ł����R���p�N�g�ł��B
  golib00w��������̂Ń\�[�X�����܂����B
  Makefile��go_0004w�����g��Ȃ��悤�ɏ��������܂����B
  libmingw���K�v�ɂȂ����̂ŉ����܂����B

< go_0004s �� go_0005s >

  ��ʌ��J�Ɍ����A�h�L�������g�����������܂����B���ꂾ���ł��B

< go_0005s �� go_0006s >

  gas2nask��nask��ǉ�����Makefile�����������Acc1��cc1plus���킸���ɉ��ǂ��܂�
��(�K�v�ȃA���C�����߂𖾎��I�ɏo�͂��܂�)�B

< go_0006s �� go_0007s >

  sjisconv��naskcnv0��ǉ����Adrv_osa�������Anask��OSASK�łƃ\�[�X���p�ł���`
���ɂ��炽�߁Acc1��cc1plus�̃Z�N�V�����܂��̃o�O���C�����Agolib00w�Œ����t�@
�C�����ł̃o�O�𒼂��܂����B

< go_0007s �� go_0008p >

  [OSASK 5584]��go_0007��NASK�Ƀo�C�i���[���[�h���̃o�O�����t�������̂ŏC������
�����B

< go_0008p �� go_0009s >

  osaskgo.bin�����ꍇ�Ɍ����āAASKA���K�v�ɂȂ�܂����B�܂�drv_osa�̒���osama
in.c�̒��̊֐�refresh_static()��osaskgo.map�̋L�q�ɂ��킹�ďC�����Ȃ���΂�����
������������̂Œ��ӂ��K�v�ł��B

< go_0009s �� go_0011s >

  go_0010p�ł�naskcnv0�̏C������荞��ŁAsjisconv��osaskgo�ɂ����ꂽ���x�ł��B

< go_0011s �� go_0012s >

  golib00��stdc��(golib00s)��ǉ����܂����B������drv_stdc���܂Ƃ��ɂ��āAcc1��
�ǂ�Linux��Ȃǂł�make�ł���悤�ɂ��܂����B

< go_0012s �� go_0013s >

  naskcnv0��stdc��(nskcn0bs)��ǉ����܂����Bgo_0012�̃o�O���C�����܂����B

< go_0013s �� go_0014s >

  gas2nask��stdc�Łigas2nsks�j��ǉ����܂����B

< go_0014s �� go_0015s >

  nask��stdc�ł�ǉ����܂����B

< go_0015s �� go_0017s >

  gas2nask��fsubrs�o�O���C�����܂����B

< go_0017s �� go_0018s >

  gas2nask�̃o�O���C�����܂����ifistps�Afists�Afilds�Astosl�Afabs�Afucomp�A
fistps�Afdivs�j�Bwin32-console�p�̊ȈՃ��C�u������ǉ����܂����B

< go_0018s �� go_0019s >

  �o���h�����Ă������C�u�����̃w�b�_�t�@�C�����ԈႦ�������Ȃ̂ŁAgo_0018s��
go_0019s�̈Ⴂ�͂��̃h�L�������g�����ł��B

< go_0019 �� go_0020 >

  �\�[�X���x���ł̉��������āAgo�ł�gcc(stdc)�ł̈Ⴂ���Ainclude/rule.mak������
���܂����B
  stdc�łł́A���p���Ă���֐����ȑO��12����8�ւƂ���Ɍ��������܂����B
    fopen, fclose, fread, fwrite, fseek, ftell, malloc, exit
�������́A�ȉ���6�֐�����邾���ł������ł��B
  GOLD_getsize(�t�@�C���T�C�Y�擾),
  GOLD_read(�t�@�C���̃o�C�i�����[�h���[�h -- �ꊇ�Ńt�@�C���S�̂�ǂނ���),
  GOLD_write_b(�t�@�C���̃o�C�i�����[�h���C�g -- �ꊇ�Ńt�@�C���S�̂���������),
  GOLD_write_t(�t�@�C���̃e�L�X�g���[�h���C�g -- �t�@�C���������Ƃ��͈ꊇ�A
                  �R���\�[���ւ̃��b�Z�[�W�o�͂̏ꍇ�͉��x���Ă΂ꂤ��),
  GOLD_exit(exit�Ƃ��Ȃ�),
  malloc(malloc�Ɠ���)
malloc�ɂ��Ă͂��ꂼ���main()�̒��ōŏ���1��Ă�ł��邾���ł��̂ŁAmalloc��
���Ƃ��č��Ȃ��Ă���։\�ł��B
  OS�ˑ��h���C�o���A���S���ʉ����܂����Bnask��gas2nask��sjisconv�Ȃǂ̃f�B���N
�g���𐮗����āA���ς��₷�����܂����B����2�_�ɂ���āA�@�\���������ɂ��������
���A�\�[�X��25KB�ȏ㏬�����Ȃ�A�t�@�C������3����A�f�B���N�g����7�����Ă�
�܂��B


�E���₢���킹�́H

  OSASK-ML���AOSASK�`���ւǂ����B

               2003.11.12  �썇�G��  kawai@imasy.org
