.text
.global q
.type q, %function
q:
mov w9, #2
add w10, w0, w9
mov w11, w10
mov x12, x0
mov w0, w11
ret
.global g
.type g, %function
g:
mov x13, x0
mov x0, #2
bl q
mov x14, x0
add w15, w14, w13
mov w0, w15
ret
.global _start
_start:
mov x0, #3
bl g
mov x16, x0
mov w17, #0
mov w0, w17
ret
