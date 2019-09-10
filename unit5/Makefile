all:	checkpoint5.1.bin

pad:	pad.c
	gcc -o pad pad.c

pack:	pack.c
	gcc -Wall -o pack pack.c

%.prg:	%.kc myabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

packed.bin:	program1.prg program2.prg program3.prg program4.prg pack
	./pack program1.prg program2.prg program3.prg program4.prg

test5.1:	os5.1.bin packed.bin
	xmega65 -loadrom packed.bin -kickup os5.1.bin

os5.1.bin:	os5.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<
