; Ladder for shared secret point multiplication
;
; Author: Daan Sprenkels <hello@dsprenkels.com>

%include "ge_add.mac"
%include "ge_double.mac"
%include "select.mac"

global crypto_scalarmult_curve13318_ref12_ladder

section .text
crypto_scalarmult_curve13318_ref12_ladder:
    ; Double-and-add ladder for shared secret point multiplication
    ;
    ; Arguments:
    ;   ge q:               [rdi]
    ;   uint8_t *windows:   [rsi]
    ;   ge ptable[16]:      [rdx]
    ;
    %xdefine stack_size 6*384 + 192 + 768

    ; prologue
    push rbp
    mov rbp, rsp
    and rsp, -32
    sub rsp, stack_size
    mov qword [rsp - 8], r8
    mov qword [rsp - 16], r9
    mov qword [rsp - 24], r10
    mov qword [rsp - 32], r11
    mov qword [rsp - 40], rbx

    ; start loop
    xor rcx, rcx
.ladderstep:
    xor rbx, rbx
.ladderstep_double:
    ge_double rdi, rdi, rsp
    add rbx, 1
    cmp rbx, 5
    jl .ladderstep_double

    ; Our lookup table is one-based indexed. The neutral element is not stored
    ; in `ptable`, but written by `ge_neutral`. The mapping from `bits` to `idx`
    ; is defined by the following:
    ;
    ; compute_idx :: Word8 -> Word8
    ; compute_idx bits
    ;   |  0 <= bits < 16 = x - 1  // sign is (+)
    ;   | 16 <= bits < 32 = ~x     // sign is (-)
    movzx rax, byte [rsi + rcx]
    mov r8, rax
    shr r8b, 4
    and r8b, 1       ; sign
    mov r9, r8
    mov r11, r8     ; save for later
    neg r8b         ; signmask
    sub r9b, 1      ; ~signmask
    mov r10, rax
    not r10b        ; ~bits
    and r8b, r10b
    sub al, 1       ; bits - 1
    and r9b, al
    or r8b, r9b
    and r8b, 0x1F   ; force the result idx to be in [0, 0x1F]

    select r8b, rdx
    ; conditionally negate y if sign == 1
    shl r11, 63     ; 0b100.. or 0b000..
    vmovq xmm15, r11
    vmovddup xmm15, xmm15
    vinsertf128 ymm15, xmm15, 0b1
    ; conditionally flip the sign bit
    vxorpd ymm3, ymm3, ymm15
    vxorpd ymm4, ymm4, ymm15
    vxorpd ymm5, ymm5, ymm15
    ; save the point to the stack
    vmovapd [rsp + 5*384], ymm0
    vmovapd [rsp + 5*384 + 1*32], ymm1
    vmovapd [rsp + 5*384 + 2*32], ymm2
    vmovapd [rsp + 5*384 + 3*32], ymm3
    vmovapd [rsp + 5*384 + 4*32], ymm4
    vmovapd [rsp + 5*384 + 5*32], ymm5
    vmovapd [rsp + 5*384 + 6*32], ymm6
    vmovapd [rsp + 5*384 + 7*32], ymm7
    vmovapd [rsp + 5*384 + 8*32], ymm8
    ; put p at [rsp + 5*384] = t5, will be overwritten but we don't care

    ; add q and p into q
    ge_add rdi, rdi, rsp + 5*384, rsp

    ; loop repeat
    add rcx, 1
    cmp rcx, 51
    jl .ladderstep

    ; epilogue
    mov rbx, qword [rsp - 40]
    mov r11, qword [rsp - 32]
    mov r10, qword [rsp - 24]
    mov r9, qword [rsp - 16]
    mov r8, qword [rsp - 8]
    mov rsp, rbp
    pop rbp
    ret

section .rodata
select_consts
fe12x4_mul_consts
fe12x4_squeeze_consts
ge_double_consts
ge_add_consts
