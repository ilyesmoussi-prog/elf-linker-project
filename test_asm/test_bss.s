@ test_bss.s
@ Fichier pour tester la section .bss (données non initialisées)

.bss
.align 2
.global grand_buffer
grand_buffer:
    .space 1024

.global compteur
compteur:
    .space 4

pile:
    .space 256

.text
.align 2
.global init_buffer

init_buffer:
    @ Initialiser le buffer à zéro
    ldr r0, =grand_buffer
    mov r1, #0
    mov r2, #1024
    
init_loop:
    strb r1, [r0], #1
    subs r2, r2, #1
    bne init_loop
    
    bx lr
