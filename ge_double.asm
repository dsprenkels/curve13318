; Doubling of group elements
;
; Author: Daan Sprenkels <hello@dsprenkels.com>

%include "ge_double.mac"

global crypto_scalarmult_curve13318_ref12_ge_double

section .text
crypto_scalarmult_curve13318_ref12_ge_double:
    %xdefine stack_size 6*384 + 192 + 768

    ; build stack frame
    push rbp
    mov rbp, rsp
    and rsp, -32
    sub rsp, stack_size
    ge_double rdi, rsi, rsp
    mov rsp, rbp
    pop rbp
    ret

section .rodata
fe12x4_mul_consts
fe12x4_squeeze_consts
ge_double_consts
