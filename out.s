.text
.global dec
.type dec, %function
dec:
sub w9, w0, #1
mov w0, w9
ret
.global inc
.type inc, %function
inc:
add w9, w0, #1
mov w0, w9
ret
.global div10
.type div10, %function
div10:
mov w10, #10
sdiv w9, w0, w10
mov w0, w9
ret
.global mod10
.type mod10, %function
mod10:
mov w10, #10
srem w9, w0, w10
mov w0, w9
ret
.global mul
.type mul, %function
mul:
mul w9, w0, w1
mov w0, w9
ret
.global add
.type add, %function
add:
add w9, w0, w1
mov w0, w9
ret
.global sub
.type sub, %function
sub:
sub w9, w0, w1
mov w0, w9
ret
.global rem
.type rem, %function
rem:
srem w9, w0, w1
mov w0, w9
ret
.global quo
.type quo, %function
quo:
sdiv w9, w0, w1
mov w0, w9
ret
.global eq
.type eq, %function
eq:
cmp w0, w1
b.ne skip0
mov w9, #1
mov w0, w9
ret
skip0:
mov w9, #0
mov w0, w9
ret
.global leq
.type leq, %function
leq:
cmp w0, w1
b.gt skip1
mov w9, #1
mov w0, w9
ret
skip1:
mov w9, #0
mov w0, w9
ret
.global less
.type less, %function
less:
cmp w0, w1
b.ge skip2
mov w9, #1
mov w0, w9
ret
skip2:
mov w9, #0
mov w0, w9
ret
.global factorial
.type factorial, %function
factorial:
mov x20, x0
mov x0, x20
mov x1, #1
bl leq
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip3
mov w19, #1
mov w0, w19
ret
skip3:
mov x0, x20
bl dec
mov x19, x0
mov x0, x19
bl factorial
mov x21, x0
mov x0, x20
mov x1, x21
bl mul
mov x19, x0
mov w0, w19
ret
.global sum_to_n
.type sum_to_n, %function
sum_to_n:
mov x20, x0
mov x0, x20
mov x1, #0
bl leq
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip4
mov w19, #0
mov w0, w19
ret
skip4:
mov x0, x20
bl dec
mov x19, x0
mov x0, x19
bl sum_to_n
mov x21, x0
mov x0, x20
mov x1, x21
bl add
mov x19, x0
mov w0, w19
ret
.global power
.type power, %function
power:
mov x20, x0
mov x0, x1
mov x21, x1
mov x1, #0
bl eq
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip5
mov w19, #1
mov w0, w19
ret
skip5:
mov x0, x21
bl dec
mov x19, x0
mov x0, x20
mov x1, x19
bl power
mov x21, x0
mov x0, x20
mov x1, x21
bl mul
mov x19, x0
mov w0, w19
ret
.global gcd
.type gcd, %function
gcd:
mov x20, x0
mov x0, x1
mov x21, x1
mov x1, #0
bl eq
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip6
mov w19, w20
mov w0, w19
ret
skip6:
mov x0, x20
mov x1, x21
bl rem
mov x19, x0
mov x0, x21
mov x1, x19
bl gcd
mov x20, x0
mov w0, w20
ret
.global fibonacci
.type fibonacci, %function
fibonacci:
mov x20, x0
mov x0, x20
mov x1, #1
bl leq
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip7
mov w19, w20
mov w0, w19
ret
skip7:
mov x0, x20
bl dec
mov x19, x0
mov x0, x19
bl fibonacci
mov x21, x0
mov x0, x20
mov x1, #2
bl sub
mov x19, x0
mov x0, x19
bl fibonacci
mov x20, x0
mov x0, x21
mov x1, x20
bl add
mov x19, x0
mov w0, w19
ret
.global digit_sum
.type digit_sum, %function
digit_sum:
mov x20, x0
mov x0, x20
mov x1, #10
bl less
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip8
mov w19, w20
mov w0, w19
ret
skip8:
mov x0, x20
bl mod10
mov x19, x0
mov x0, x20
bl div10
mov x21, x0
mov x0, x21
bl digit_sum
mov x20, x0
mov x0, x19
mov x1, x20
bl add
mov x21, x0
mov w0, w21
ret
.global digit_count
.type digit_count, %function
digit_count:
mov x20, x0
mov x0, x20
mov x1, #10
bl less
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip9
mov w19, #1
mov w0, w19
ret
skip9:
mov x0, x20
bl div10
mov x19, x0
mov x0, x19
bl digit_count
mov x20, x0
mov x0, x20
bl inc
mov x19, x0
mov w0, w19
ret
.global reverse_number_helper
.type reverse_number_helper, %function
reverse_number_helper:
mov x20, x0
mov x0, x20
mov x21, x1
mov x1, #0
bl eq
mov x19, x0
mov w9, #1
cmp w19, w9
b.ne skip10
mov w19, w21
mov w0, w19
ret
skip10:
mov x0, x20
bl mod10
mov x19, x0
mov x0, x21
mov x1, #10
bl mul
mov x22, x0
mov x0, x22
mov x1, x19
bl add
mov x21, x0
mov x0, x20
bl div10
mov x19, x0
mov x0, x19
mov x1, x21
bl reverse_number_helper
mov x20, x0
mov w0, w20
ret
.global reverse_number
.type reverse_number, %function
reverse_number:
mov x20, x0
mov x0, x20
mov x1, #0
bl reverse_number_helper
mov x19, x0
mov w0, w19
ret
.global is_palindrome
.type is_palindrome, %function
is_palindrome:
mov x20, x0
mov x0, x20
bl reverse_number
mov x19, x0
mov x0, x20
mov x1, x19
bl eq
mov x21, x0
mov w9, #1
cmp w21, w9
b.ne skip11
mov w19, #1
mov w0, w19
ret
skip11:
mov w19, #0
mov w0, w19
ret
.global lcm
.type lcm, %function
lcm:
mov x20, x0
mov x0, x20
mov x21, x1
mov x1, x21
bl gcd
mov x19, x0
mov x0, x20
mov x1, x19
bl quo
mov x22, x0
mov x0, x22
mov x1, x21
bl mul
mov x19, x0
mov w0, w19
ret
.global _start
_start:
mov x0, #5
bl factorial
mov x19, x0
mov x0, #15
bl sum_to_n
mov x20, x0
mov x0, #3
mov x1, #6
bl power
mov x21, x0
mov x0, #84
mov x1, #30
bl gcd
mov x22, x0
mov x0, #10
bl fibonacci
mov x23, x0
mov x0, #9876
bl digit_sum
mov x24, x0
mov x0, #12345
bl digit_count
mov x25, x0
mov x0, #4321
bl reverse_number
mov x26, x0
mov x0, #1221
bl is_palindrome
mov x27, x0
mov x0, #12
mov x1, #18
bl lcm
mov x28, x0
mov w29, #0
mov w0, w29
ret
