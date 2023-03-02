.global _start
.section ".text"

_start:
    ldr x0, = _stack_top
    mov sp, x0
    wfe
    b main