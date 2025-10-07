.text
.global p
.type p, %function
p:
mov w9, w0
mov w0, w9
ret
.global l
.type l, %function
l:
add w9, w0, w1
mov w0, w9
ret
.global _start
_start:
sub sp, sp, #32
mov w0, #10
bl p
mov x19, x0
str x19, [sp, #4]
mov w0, #1
bl p
mov x19, x0
mov w0, #2
bl p
mov x20, x0
mov w0, #3
bl p
mov x21, x0
str x21, [sp, #12]
mov w0, #4
bl p
mov x21, x0
str x21, [sp, #16]
mov w0, #5
bl p
mov x21, x0
str x21, [sp, #20]
mov w0, #6
bl p
mov x21, x0
str x21, [sp, #8]
mov w0, #7
bl p
mov x21, x0
str x21, [sp, #24]
mov w0, #8
bl p
mov x21, x0
mov w0, #9
bl p
mov x22, x0
mov w0, #10
bl p
mov x23, x0
mov x0, w19
mov x1, w20
bl l
mov x24, x0
mov x0, w19
mov x1, w20
bl l
mov x25, x0
mov x0, w24
mov x1, w25
bl l
mov x26, x0
str x26, [sp, #0]
add w24, w19, w20
ldr w26, [sp, #12]
add w25, w24, w26
ldr w26, [sp, #16]
add w24, w25, w26
ldr w26, [sp, #20]
add w25, w24, w26
ldr w26, [sp, #8]
add w24, w25, w26
ldr w26, [sp, #24]
add w25, w24, w26
add w24, w25, w21
add w25, w24, w22
add w24, w25, w23
mov x0, w24
bl p
mov x24, x0
ldr w24, [sp, #4]
cmp w24, #10
b.ne skip0
add w24, w19, w20
ldr w26, [sp, #12]
add w25, w24, w26
ldr w26, [sp, #16]
add w24, w25, w26
ldr w26, [sp, #20]
add w25, w24, w26
ldr w26, [sp, #8]
add w24, w25, w26
ldr w26, [sp, #24]
add w25, w24, w26
add w24, w25, w21
add w21, w24, w22
add w22, w21, w23
add w21, w22, w24
ldr w22, [sp, #0]
add w22, w21, w22
mov w0, w22
add sp, sp, #32
ret
b skip2
skip0:
ldr w21, [sp, #4]
cmp w21, #20
b.ne skip1
mov w22, #5
mov w0, w22
add sp, sp, #32
ret
b skip2
skip1:
ldr w21, [sp, #4]
mov w20, w21
ldr w21, [sp, #4]
mov w19, w21
ldr w21, [sp, #4]
mov w21, #2
cmp w19, #20
b.ne skip3
mov w20, #24
skip3:
mov w22, #10
mov w0, w22
add sp, sp, #32
ret
skip2:
add w21, w20, w19
add w22, w21, w19
mov w0, w22
add sp, sp, #32
ret
