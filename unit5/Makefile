all:	checkpoint5.1.bin

pad:	pad.c
	gcc -o pad pad.c

pack:	pack.c
	gcc -Wall -o pack pack.c

program1.prg:	program1.kc osabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

program2.prg:	program2.kc osabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

program3.prg:	program3.kc osabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

program4.prg:	program4.kc osabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

packed.bin:	program1.prg program2.prg program3.prg program4.prg pack
	./pack program1.prg program2.prg program3.prg program4.prg

test5.1:	os5.1.bin packed.bin
	./check_if_committed && xmega65 -loadrom packed.bin -kickup os5.1.bin

os5.1.bin:	os5.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test5.2:	os5.2.bin packed.bin
	./check_if_committed && xmega65 -loadrom packed.bin -kickup os5.2.bin

os5.2.bin:	os5.2.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test5.3:	os5.3.bin packed.bin
	./check_if_committed && xmega65 -loadrom packed.bin -kickup os5.3.bin

os5.3.bin:	os5.3.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test5.4:	os5.4.bin packed.bin
	./check_if_committed && xmega65 -loadrom packed.bin -kickup os5.4.bin

os5.4.bin:	os5.4.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test5.5:	os5.5.bin packed.bin
	./check_if_committed && xmega65 -loadrom packed.bin -kickup os5.5.bin

os5.5.bin:	os5.5.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

