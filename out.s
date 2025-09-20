.text
.global c
.type c, %function
c:
cmp w0, #2
b.eq goto0
cmp w0, #3
b.ne skip0
goto0:
add w0, w0, #1
b skip1
skip0:
cmp w0, #2
b.ne skip2
mov w0, #2
b skip1
skip2:
ldr w9, =100000
cmp w0, w9
b.ne skip3
mov w0, #5
b skip1
skip3:
mov w0, #3
skip1:
mov x10, x0
mov x0, x10
bl c
mov x9, x0
mov x0, x9
bl c
mov x9, x0
mov w0, w9
ret
.global _start
_start:
mov w0, #8
bl c
mov x19, x0
add w20, w19, #2
add w19, w20, #2
add w20, w19, #2
mov w0, w20
ret
