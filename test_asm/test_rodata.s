@ test_rodata.s
@ Fichier avec section .rodata (données en lecture seule)

.section .rodata
.align 2

message:
    .ascii "Hello, World!\n"
    .byte 0

constantes:
    .word 3, 1, 4, 1, 5, 9, 2, 6

magic_number:
    .word 0xDEADBEEF

.text
.align 2
.global get_message
.global get_magic

get_message:
    ldr r0, =message
    bx lr

get_magic:
    ldr r0, =magic_number
    ldr r0, [r0]
    bx lr
