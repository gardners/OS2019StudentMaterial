lda #<{c2}
sta {c1}
lda #>{c2}
sta {c1}+1
lda #<{c2}>>16
sta {c1}+2
lda #>{c2}>>16
sta {c1}+3
