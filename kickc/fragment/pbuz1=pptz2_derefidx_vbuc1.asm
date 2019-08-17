ldy #{c1}
lda ({z2}),y
pha
iny
lda ({z2}),y
sta {z1}+1
pla
sta {z1}
