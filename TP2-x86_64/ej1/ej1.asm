; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern str_concat
extern strlen
extern strcpy
extern strcat
extern fprintf


string_proc_list_create_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     edi, 16
        call    malloc
        mov     [rbp-8], rax
        cmp     qword [rbp-8], 0
        jne     .L2
        mov     eax, 0
        jmp     .L3
.L2:
        mov     rax, [rbp-8]
        mov     qword [rax], 0
        mov     rax, [rbp-8]
        mov     qword [rax+8], 0
        mov     rax, [rbp-8]
.L3:
        leave
        ret
string_proc_node_create_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     eax, edi
        mov     [rbp-32], rsi
        mov     [rbp-20], al
        mov     edi, 32
        call    malloc
        mov     [rbp-8], rax
        cmp     qword [rbp-8], 0
        jne     .L5
        mov     eax, 0
        jmp     .L6
.L5:
        mov     rax, [rbp-8]
        mov     qword [rax], 0
        mov     rax, [rbp-8]
        mov     qword [rax+8], 0
        mov     rax, [rbp-8]
        movzx   edx, byte [rbp-20]
        mov     [rax+16], dl
        mov     rax, [rbp-8]
        mov     rdx, [rbp-32]
        mov     [rax+24], rdx
        mov     rax, [rbp-8]
.L6:
        leave
        ret
string_proc_list_add_node_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 48
        mov     [rbp-24], rdi
        mov     eax, esi
        mov     [rbp-40], rdx
        mov     [rbp-28], al
        movzx   eax, byte [rbp-28]
        mov     rdx, [rbp-40]
        mov     rsi, rdx
        mov     edi, eax
        call    string_proc_node_create_asm
        mov     [rbp-8], rax
        cmp     qword [rbp-8], 0
        je      .L11
        mov     rax, [rbp-24]
        mov     rax, [rax]
        test    rax, rax
        jne     .L10
        mov     rax, [rbp-24]
        mov     rdx, [rbp-8]
        mov     [rax], rdx
        mov     rax, [rbp-24]
        mov     rdx, [rbp-8]
        mov     [rax+8], rdx
        jmp     .L7
.L10:
        mov     rax, [rbp-24]
        mov     rax, [rax+8]
        mov     rdx, [rbp-8]
        mov     [rax], rdx
        mov     rax, [rbp-24]
        mov     rdx, [rax+8]
        mov     rax, [rbp-8]
        mov     [rax+8], rdx
        mov     rax, [rbp-24]
        mov     rdx, [rbp-8]
        mov     [rax+8], rdx
        jmp     .L7
.L11:
        nop
.L7:
        leave
        ret
string_proc_list_concat_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 64
        mov     [rbp-40], rdi
        mov     eax, esi
        mov     [rbp-56], rdx
        mov     [rbp-44], al
        mov     rax, [rbp-40]
        mov     rax, [rax]
        mov     [rbp-8], rax
        mov     qword [rbp-16], 0
        jmp     .L13
.L16:
        mov     rax, [rbp-8]
        movzx   eax, byte [rax+16]
        cmp     byte [rbp-44], al
        jne     .L14
        cmp     qword [rbp-16], 0
        jne     .L15
        mov     rax, [rbp-8]
        mov     rdx, [rax+24]
        mov     rax, [rbp-56]
        mov     rsi, rdx
        mov     rdi, rax
        call    str_concat
        mov     [rbp-16], rax
        jmp     .L14
.L15:
        mov     rax, [rbp-8]
        mov     rdx, [rax+24]
        mov     rax, [rbp-16]
        mov     rsi, rdx
        mov     rdi, rax
        call    str_concat
        mov     [rbp-24], rax
        mov     rax, [rbp-16]
        mov     rdi, rax
        call    free
        mov     rax, [rbp-24]
        mov     [rbp-16], rax
.L14:
        mov     rax, [rbp-8]
        mov     rax, [rax]
        mov     [rbp-8], rax
.L13:
        cmp     qword [rbp-8], 0
        jne     .L16
        mov     rax, [rbp-16]
        leave
        ret
string_proc_list_destroy_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     [rbp-24], rdi
        mov     rax, [rbp-24]
        mov     rax, [rax]
        mov     [rbp-8], rax
        mov     qword [rbp-16], 0
        jmp     .L19
.L20:
        mov     rax, [rbp-8]
        mov     rax, [rax]
        mov     [rbp-16], rax
        mov     rax, [rbp-8]
        mov     rdi, rax
        call    string_proc_node_destroy_asm
        mov     rax, [rbp-16]
        mov     [rbp-8], rax
.L19:
        cmp     qword [rbp-8], 0
        jne     .L20
        mov     rax, [rbp-24]
        mov     qword [rax], 0
        mov     rax, [rbp-24]
        mov     qword [rax+8], 0
        mov     rax, [rbp-24]
        mov     rdi, rax
        call    free
        nop
        leave
        ret
string_proc_node_destroy_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     [rbp-8], rdi
        mov     rax, [rbp-8]
        mov     qword [rax], 0
        mov     rax, [rbp-8]
        mov     qword [rax+8], 0
        mov     rax, [rbp-8]
        mov     qword [rax+24], 0
        mov     rax, [rbp-8]
        mov     byte [rax+16], 0
        mov     rax, [rbp-8]
        mov     rdi, rax
        call    free
        nop
        leave
        ret
str_concat_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 48
        mov     [rbp-40], rdi
        mov     [rbp-48], rsi
        mov     rax, [rbp-40]
        mov     rdi, rax
        call    strlen
        mov     [rbp-4], eax
        mov     rax, [rbp-48]
        mov     rdi, rax
        call    strlen
        mov     [rbp-8], eax
        mov     edx, [rbp-4]
        mov     eax, [rbp-8]
        add     eax, edx
        mov     [rbp-12], eax
        mov     eax, [rbp-12]
        add     eax, 1
        cdqe
        mov     rdi, rax
        call    malloc
        mov     [rbp-24], rax
        mov     rdx, [rbp-40]
        mov     rax, [rbp-24]
        mov     rsi, rdx
        mov     rdi, rax
        call    strcpy
        mov     rdx, [rbp-48]
        mov     rax, [rbp-24]
        mov     rsi, rdx
        mov     rdi, rax
        call    strcat
        mov     rax, [rbp-24]
        leave
        ret


string_proc_list_print_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        mov     [rbp-24], rdi
        mov     [rbp-32], rsi
        mov     dword [rbp-4], 0
        mov     rax, qword [rbp-24]
        mov     rax, [rax]
        mov     [rbp-16], rax
        jmp     .L25
.L26:
        add     dword [rbp-4], 1
        mov     rax, qword [rbp-16]
        mov     rax, [rax]
        mov     [rbp-16], rax
.L25:
        cmp     qword [rbp-16], 0
        jne     .L26
        mov     edx, [rbp-4]
        mov     rax, [rbp-32]
        mov     rsi, .LC0
        mov     rdi, rax
        mov     eax, 0
        call    fprintf
        mov     rax, qword [rbp-24]
        mov     rax, [rax]
        mov     [rbp-16], rax
        jmp     .L27
.L28:
        mov     rax, [rbp-16]
        movzx   eax, byte [rax+16]
        movzx   ecx, al
        mov     rax, [rbp-16]
        mov     rdx, [rax+24]
        mov     rax, [rbp-32]
        mov     rsi, .LC1
        mov     rdi, rax
        mov     eax, 0
        call    fprintf
        mov     rax, qword [rbp-16]
        mov     rax, [rax]
        mov     [rbp-16], rax
.L27:
        cmp     qword [rbp-16], 0
        jne     .L28
        nop
        nop
        leave
        ret
.LC0:
    db "List length: %d", 10, 0
.LC1:
    db 9, "node hash: %s | type: %d", 10, 0