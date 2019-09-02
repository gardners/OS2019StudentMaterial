all:	checkpoint3.1.bin

checkpoint3.1.bin:	checkpoint3.1.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test3.1:	checkpoint3.1.bin
	./check_if_committed && xmega65 -kickup checkpoint3.1.bin

checkpoint3.2.bin:	checkpoint3.2.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test3.2:	checkpoint3.2.bin
	./check_if_committed && xmega65 -kickup checkpoint3.2.bin

checkpoint3.3.bin:	checkpoint3.3.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test3.3:	checkpoint3.3.bin
	./check_if_committed && xmega65 -kickup checkpoint3.3.bin

checkpoint3.4.bin:	checkpoint3.4.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test3.4:	checkpoint3.4.bin
	./check_if_committed && xmega65 -kickup checkpoint3.4.bin

checkpoint3.5.bin:	checkpoint3.5.kc
	${HOME}/OS2019StudentMaterial/kickc/bin/kickc.sh  -a $<

test3.5:	checkpoint3.5.bin
	./check_if_committed && xmega65 -kickup checkpoint3.5.bin
