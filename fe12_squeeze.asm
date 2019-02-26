; Carry ripple implementation for integers modulo 2^255 - 19
;
; Author: Daan Sprenkels <hello@dsprenkels.com>

%include "fe12_squeeze.mac"

global crypto_scalarmult_curve13318_ref12_fe12x4_squeeze, crypto_scalarmult_curve13318_ref12_fe12x4_squeeze_noload


section .text
crypto_scalarmult_curve13318_ref12_fe12x4_squeeze:
    fe12x4_squeeze_load rdi
crypto_scalarmult_curve13318_ref12_fe12x4_squeeze_noload:
    fe12x4_squeeze_body
    fe12x4_squeeze_store rdi
    ret

section .rodata:
fe12x4_squeeze_consts
