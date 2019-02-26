; Multiplication function for field elements (integers modulo 2^255 - 19)
;
; Author: Daan Sprenkels <hello@dsprenkels.com>

%include "fe12_mul.mac"

global crypto_scalarmult_curve13318_ref12_fe12x4_mul, crypto_scalarmult_curve13318_ref12_fe12x4_mul_nosqueeze
extern crypto_scalarmult_curve13318_ref12_fe12x4_squeeze_noload

section .text
crypto_scalarmult_curve13318_ref12_fe12x4_mul:
    ; Multiply two field elements using subtractive karatsuba
    ;
    ; Input:  two vectorized field elements [rsi], [rdx]
    ; Output: the uncarried product of the two inputs [rdi]
    ;
    ; Precondition: TODO
    ; Postcondition: TODO
    ;
    ; build stack frame
    push rbp
    mov rbp, rsp
    and rsp, -32
    sub rsp, 640

    fe12x4_mul_preload rsi, rdx
    fe12x4_mul_body rdi, rsi, rdx, (rsp-128)
    fe12x4_mul_reload_C_high rdi

    ; restore stack frame
    mov rsp, rbp
    pop rbp

    ; squeeze the result
    jmp crypto_scalarmult_curve13318_ref12_fe12x4_squeeze_noload wrt ..plt

section .rodata
fe12x4_mul_consts

section .text
crypto_scalarmult_curve13318_ref12_fe12x4_mul_nosqueeze:
    ; Multiply two field elements using subtractive karatsuba
    ;
    ; Input:  two vectorized field elements [rsi], [rdx]
    ; Output: the uncarried product of the two inputs [rdi]
    ;
    ; Precondition: TODO
    ; Postcondition: TODO
    ;
    ; build stack frame
    push rbp
    mov rbp, rsp
    and rsp, -32
    sub rsp, 640

    fe12x4_mul_preload rsi, rdx
    fe12x4_mul_body rdi, rsi, rdx, (rsp-128)
    fe12x4_mul_store_C_low rdi

    ; restore stack frame
    mov rsp, rbp
    pop rbp
    ret

section .rodata
fe12x4_mul_consts
