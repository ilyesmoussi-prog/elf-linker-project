@ fusion_a.s
@ Premier fichier pour tester la fusion (A)

.data
.align 2
.global compteur_a
compteur_a:
    .word 0

message_a:
    .ascii "Fichier A\n"
    .byte 0

.text
.align 2
.global main
.global fonction_a

main:
    push {lr}
    
    @ Appeler fonction_a
    bl fonction_a
    
    @ Appeler fonction_b (définie dans fusion_b.s)
    bl fonction_b
    
    mov r0, #42
    pop {pc}

fonction_a:
    @ Incrémenter compteur_a
    ldr r0, =compteur_a
    ldr r1, [r0]
    add r1, r1, #1
    str r1, [r0]
    bx lr

@ Déclaration externe
.extern fonction_b
.extern compteur_b
