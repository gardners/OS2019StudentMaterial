.file [name="%O.prg", type="prg", segments="Program"]
.segmentdef Program [segments="Basic, Code, Data, CodeHigh, DataHigh"]
.segmentdef Basic [start=$0801]
.segmentdef Code [start=$0810]
.segmentdef Data [startAfter="Code"]
.segmentdef CodeHigh [start=$4000]
.segmentdef DataHigh [startAfter="CodeHigh"]
.segment Basic
:BasicUpstart(main)

