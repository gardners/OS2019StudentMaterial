ldy #{c1}
lda ({z2}),y
sta {z1}
iny
lda #0
sta {z1}+1
iny
sta {z1}+2
iny
sta {z1}+3
