all:	checkpoint6.1.bin

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

program5.prg:	program5.kc osabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

program6.prg:	program6.kc osabi.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

packed1.bin:	program1.prg pack
	./pack program1.prg && mv packed.bin packed1.bin

packed2.bin:	program1.prg program2.prg pack
	./pack program1.prg program2.prg && mv packed.bin packed2.bin

packed5.bin:	program5.prg program6.prg pack
	./pack program5.prg program6.prg && mv packed.bin packed5.bin

packed4.bin:	program1.prg program2.prg program3.prg program4.prg pack
	./pack program1.prg program2.prg program3.prg program4.prg && mv packed.bin packed4.bin

test6.1:	os6.1.bin packed1.bin
	xmega65 -loadrom packed1.bin -kickup os6.1.bin

os6.1.bin:	os6.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test6.2:	os6.2.bin packed2.bin
	xmega65 -loadrom packed2.bin -kickup os6.2.bin

os6.2.bin:	os6.2.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test6.3:	os6.3.bin packed4.bin
	xmega65 -loadrom packed4.bin -kickup os6.3.bin

os6.3.bin:	os6.3.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test6.4:	os6.4.bin packed4.bin
	xmega65 -loadrom packed4.bin -kickup os6.4.bin

os6.4.bin:	os6.4.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test6.5:	os6.5.bin packed5.bin
	xmega65 -loadrom packed5.bin -kickup os6.5.bin

os6.5.bin:	os6.5.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

