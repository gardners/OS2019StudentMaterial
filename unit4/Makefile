all:	checkpoint4.1.bin

pad:	pad.c
	gcc -o pad pad.c

#userspace4.1.prg:	userspace4.1.kc pad
#	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<
#	./pad userspace4.1.prg
#
#userspace4.2.prg:	userspace4.2.kc pad
#	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<
#	./pad userspace4.2.prg
#
userspace4.3.prg:	userspace4.3.kc myabi.kc pad
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<
	./pad userspace4.3.prg

userspace4.4.prg:	userspace4.4.kc myabi.kc pad
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<
	./pad userspace4.4.prg

userspace4.5.prg:	userspace4.5.kc myabi.kc pad
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<
	./pad userspace4.5.prg

checkpoint4.1.bin:	checkpoint4.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test4.1:	checkpoint4.1.bin userspace4.1.prg
	./check_if_committed && xmega65 -loadrom userspace4.1.prg -kickup checkpoint4.1.bin

checkpoint4.2.bin:	checkpoint4.2.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh -a $<

test4.2:	checkpoint4.2.bin userspace4.2.prg
	./check_if_committed && xmega65 -kickup checkpoint4.2.bin -loadrom userspace4.2.prg

checkpoint4.3.bin:	checkpoint4.3.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test4.3:	checkpoint4.3.bin userspace4.3.prg
	./check_if_committed && xmega65 -kickup checkpoint4.3.bin -loadrom userspace4.3.prg

checkpoint4.4.bin:	checkpoint4.4.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test4.4:	checkpoint4.4.bin userspace4.4.prg
	./check_if_committed && xmega65 -kickup checkpoint4.4.bin -loadrom userspace4.4.prg

checkpoint4.5.bin:	checkpoint4.5.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test4.5:	checkpoint4.5.bin userspace4.5.prg
	./check_if_committed && xmega65 -kickup checkpoint4.5.bin -loadrom userspace4.5.prg
