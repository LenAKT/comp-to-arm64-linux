.text
.global q
.type q, %function
q:
mov w10, #2
mul w9, w10, w0
add w11, w9, #6
mov w12, w11
mov w0, w12
ret
.global g
.type g, %function
g:
mov x20, x0
mov x0, #4
bl q
mov x19, x0
mov x0, x20
bl q
mov x21, x0
add w22, w19, w21
add w23, w22, w20
mov w0, w23
ret
.global h
.type h, %function
h:
mov w25, #2
mul w24, w0, w25
mov x27, x0
mov x0, x24
bl q
mov x26, x0
cmp w24, w26
b.lt skip0
sub w26, w26, w24
skip0:
mov w13, #50
cmp w26, w13
b.lt skip1
add w26, w26, #1
skip1:
mov w24, w26
mov w0, w24
ret
.global _start
_start:
mov x0, #5
bl g
mov x26, x0
mov x0, #5
bl q
mov x28, x0
add w28, w26, w28
mov w29, w28
mov w0, w29
ret
