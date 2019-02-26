; Select an element from a lookup table
;
; Author: Daan Sprenkels <hello@dsprenkels.com>

%include "select.mac"

global crypto_scalarmult_curve13318_ref12_select

section .text
crypto_scalarmult_curve13318_ref12_select:
    ; select the element from the lookup table at index `idx` and copy the
    ; element to `dest`.
    ; C-type: void select(ge dest, uint8_t idx, const ge ptable[16])
    ;
    ; Arguments:
    ;   - rdi: destination buffer
    ;   - sil: idx (unsigned)
    ;   - rdx: pointer to the start of the lookup table
    ;
    select sil, rdx

    ; writeback the field element
    vmovapd [rdi], ymm0
    vmovapd [rdi + 1*32], ymm1
    vmovapd [rdi + 2*32], ymm2
    vmovapd [rdi + 3*32], ymm3
    vmovapd [rdi + 4*32], ymm4
    vmovapd [rdi + 5*32], ymm5
    vmovapd [rdi + 6*32], ymm6
    vmovapd [rdi + 7*32], ymm7
    vmovapd [rdi + 8*32], ymm8
    ret

section .rodata:
select_consts
