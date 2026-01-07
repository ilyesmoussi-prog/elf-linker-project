@ test_data.s
@ Fichier avec section .data et relocations R_ARM_ABS32

.data
.align 2
.global ma_variable
.global tableau

ma_variable:
    .word 0x12345678

tableau:
    .word 1, 2, 3, 4, 5

@ Pointeur vers une fonction (relocation ABS32)
pointeur_fonction:
    .word ma_fonction

@ Pointeur vers une variable
pointeur_variable:
    .word ma_variable

.text
.align 2
.global ma_fonction

ma_fonction:
    @ Charger l'adresse de ma_variable
    ldr r0, =ma_variable
    ldr r1, [r0]
    mov pc, lr
