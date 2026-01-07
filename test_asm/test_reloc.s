@ test_reloc.s
@ Fichier pour tester différents types de relocations ARM

.data
.align 2
ma_donnee:
    .word 42

.text
.align 2
.global test_relocations

test_relocations:
    push {r4, lr}
    
    @ R_ARM_CALL : Appel de fonction
    bl fonction_externe
    
    @ R_ARM_ABS32 : Chargement d'adresse absolue
    ldr r0, =ma_donnee
    ldr r1, [r0]
    
    @ R_ARM_JUMP24 : Saut conditionnel
    cmp r1, #0
    beq fin
    
    @ Autre appel
    bl autre_fonction
    
fin:
    pop {r4, pc}

@ Fonction locale pour créer des relocations internes
fonction_locale:
    mov r0, #123
    bx lr

@ Déclarations externes (créent des symboles UND)
.extern fonction_externe
.extern autre_fonction
