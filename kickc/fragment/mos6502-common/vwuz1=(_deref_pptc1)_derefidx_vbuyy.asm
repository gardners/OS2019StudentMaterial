lda {c1}
sta $fe
lda {c1}+1
sta $ff
lda ($fe),y
sta {z1}
iny
lda ($fe),y
sta {z1}+1
