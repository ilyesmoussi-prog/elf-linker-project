@ test_minimal.s
@ Fichier ARM minimal (pas de relocation, pas de données)

.text
.align 2
.global _start

_start:
    mov r0, #0
    mov r7, #1    @ syscall exit
    svc 0
