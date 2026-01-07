@ fusion_b.s
@ Second fichier pour tester la fusion (B)

.data
.align 2
.global compteur_b
compteur_b:
    .word 0

message_b:
    .ascii "Fichier B\n"
    .byte 0

.text
.align 2
.global fonction_b

fonction_b:
    push {lr}
    
    @ Incrémenter compteur_b
    ldr r0, =compteur_b
    ldr r1, [r0]
    add r1, r1, #1
    str r1, [r0]
    
    @ Appeler fonction_a (définie dans fusion_a.s)
    bl fonction_a
    
    pop {pc}

@ Déclaration externe
.extern fonction_a
.extern compteur_a
