@ test_simple.s
@ Fichier ARM simple avec une fonction

.text
.align 2
.global main

main:
    push {lr}
    mov r0, #42
    pop {pc}

@ Symbole local
.L_local_label:
    nop
