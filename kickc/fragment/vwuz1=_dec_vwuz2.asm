lda {z2}
clc
adc #<$ffffffff
sta {z1}
lda {z2}+1
adc #>$ffffffff
sta {z1}+1
