lda ({z2}),y
sec
sbc #<{c1}
sta {z1}
iny
lda ({z2}),y
sbc #>{c1}
sta {z1}+1
