; Addition of two group elements
;
; Author: Daan Sprenkels <hello@dsprenkels.com>

%include "ge_add.mac"

global crypto_scalarmult_curve13318_ref12_ge_add

crypto_scalarmult_curve13318_ref12_ge_add:
    %xdefine stack_size  6*384 + 768

    ; build stack frame
    push rbp
    mov rbp, rsp
    and rsp, -32
    sub rsp, stack_size

    ge_add rdi, rsi, rdx, rsp

    ; restore stack frame
    mov rsp, rbp
    pop rbp
    ret

section .rodata
fe12x4_mul_consts
fe12x4_squeeze_consts
ge_add_consts
