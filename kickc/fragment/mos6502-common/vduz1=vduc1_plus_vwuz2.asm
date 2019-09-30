lda #<{c1}
clc
adc {z2}
sta {z1}
lda #>{c1}
adc {z2}+1
sta {z1}+1
lda #<{c1}>>16
adc {z2}+2
sta {z1}+2
lda #>{c1}>>16
adc {z2}+3
sta {z1}+3
