lda {z2}
sta {z1}
lda #0
sta {z1}+1
ldx #{c1}
!:
asl {z1}
rol {z1}+1
dex
bne !-

