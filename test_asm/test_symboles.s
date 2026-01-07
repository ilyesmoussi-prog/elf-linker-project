@ test_symboles.s
@ Fichier pour tester symboles locaux et globaux

.data
.global var_globale
var_globale:
    .word 100

var_locale:
    .word 200

.bss
.global buffer
buffer:
    .space 256

.text
.align 2
.global fonction_publique

@ Fonction globale (publique)
fonction_publique:
    push {lr}
    bl fonction_privee
    pop {pc}

@ Fonction locale (privée)
fonction_privee:
    mov r0, #1
    bx lr

@ Fonction static (locale)
.L_helper:
    mov r1, #2
    bx lr
