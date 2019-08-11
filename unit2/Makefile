all:	checkpoint2.1.bin

checkpoint2.1.bin:	checkpoint2.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test2.1:	checkpoint2.1.bin
	xmega65 -kickup checkpoint2.1.bin
