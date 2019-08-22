all:	checkpoint4.1.bin

pad:	pad.c
	gcc -o pad pad.c

userspace4.1.prg:	userspace4.1.kc myabi.kc pad
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -Onoloophead -a $<
	./pad userspace4.1.prg

checkpoint4.1.bin:	checkpoint4.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -Onoloophead -a $<

test4.1:	checkpoint4.1.bin userspace4.1.prg
	./check_if_committed && xmega65 -loadrom userspace4.1.prg -kickup checkpoint4.1.bin

checkpoint4.2.bin:	checkpoint4.2.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -Onoloophead -a $<

test4.2:	checkpoint4.2.bin
	./check_if_committed && xmega65 -kickup checkpoint4.2.bin

checkpoint4.3.bin:	checkpoint4.3.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -Onoloophead -a $<

test4.3:	checkpoint4.3.bin
	./check_if_committed && xmega65 -kickup checkpoint4.4.bin

checkpoint4.4.bin:	checkpoint4.4.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -Onoloophead -a $<

test4.4:	checkpoint4.4.bin
	./check_if_committed && xmega65 -kickup checkpoint4.4.bin

checkpoint4.5.bin:	checkpoint4.5.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -Onoloophead -a $<

test4.5:	checkpoint4.5.bin
	./check_if_committed && xmega65 -kickup checkpoint4.5.bin
