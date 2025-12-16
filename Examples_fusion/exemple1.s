.global main
.global loop
.global other
.text
dead:
    mov r0, #0
    bl other
loop:
    subs r0, r0, #1
    bne loop
other:
    bx lr
end:
    swi 0x123456
.data
.word other
