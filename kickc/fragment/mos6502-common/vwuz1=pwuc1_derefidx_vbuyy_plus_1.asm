lda {c1},y
clc
adc #1
sta {z1}
lda {c1}+1,y
adc #0
sta {z1}+1

