lda #<{c2}
ldy #{c1}
sta ({z1}),y
iny
lda #>{c2}
sta ({z1}),y
