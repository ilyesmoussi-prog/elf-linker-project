@ test_appels.s
@ Fichier avec appels de fonctions (relocations R_ARM_CALL)

.text
.align 2
.global main

main:
    push {lr}
    
    @ Appel à une fonction externe
    bl fonction_externe
    
    @ Appel à une fonction locale
    bl fonction_locale
    
    mov r0, #0
    pop {pc}

@ Fonction locale
fonction_locale:
    mov r0, #123
    bx lr

@ Déclaration de fonction externe (non définie)
.extern fonction_externe
