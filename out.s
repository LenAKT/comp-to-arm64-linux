.text
.global h
.type h, %function
h:
mov w9, #2
mov w10, #3
cmp w9, w0
b.eq goto0
cmp w10, w0
b.ne skip0
goto0:
mov w9, #5
skip0:
mov w11, w9
mov w0, w11
ret
.global _start
_start:
mov x20, x0
mov x0, #5
bl h
mov x19, x0
mov w21, w19
mov w0, w21
ret
