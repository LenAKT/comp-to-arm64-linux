.text
.global f
.type f, %function
f:
mov w10, #2
mul w9, w10, w0
mov w0, w9
ret
.global _start
_start:
mov x0, #2
bl f
mov x19, x0
mov w20, #2
mov w10, #4
cmp w19, w10
b.ne skip0
mov w20, #3
skip0:
add w19, w20, #5
add w21, w19, #2
add w21, w21, w19
add w22, w19, #10
add w23, w22, w21
add w22, w23, #4
add w23, w22, w21
add w22, w23, #10
add w23, w22, w19
add w22, w23, #2
add w23, w22, #2
add w22, w23, #4
mov w0, w22
ret
