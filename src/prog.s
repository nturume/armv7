.section .text
.globl _start

my_data: .long 0xfff
 .long 0xfff
 .long 0xfff
 .long 0xfff
 .long 0xfff

foo:
        push {lr}
        wfe
        wfi
        pop {pc}


_start:
ldr r0, =my_data
ldr r0, [r0, #4]
mov sp, #64
bl foo
b _start
