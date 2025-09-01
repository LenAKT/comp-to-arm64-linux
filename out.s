.text
.global q
.type q, %function
q:
mov w10, #2
mul w9, w10, w0
add w11, w9, #6
mul w12, w0, w11
mov w13, w12
mov x14, x0
mov w0, w13
ret
.global g
.type g, %function
g:
mov w15, #4
mov x16, x0
mov x0, x15
bl q
mov x17, x0
add w19, w18, w16
mov w0, w19
ret
.global _start
_start:
mov x0, #3
bl g
mov x20, x0
mov w22, #0
mov w0, w22
ret
