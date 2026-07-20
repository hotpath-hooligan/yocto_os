.section .text
.code32

.macro INTERRUPT_NO_ERROR vector
.global interrupt_stub_\vector
.type interrupt_stub_\vector, @function
interrupt_stub_\vector:
    pushl $0
    pushl $\vector
    jmp interrupt_common
.size interrupt_stub_\vector, . - interrupt_stub_\vector
.endm

.macro INTERRUPT_WITH_ERROR vector
.global interrupt_stub_\vector
.type interrupt_stub_\vector, @function
interrupt_stub_\vector:
    pushl $\vector
    jmp interrupt_common
.size interrupt_stub_\vector, . - interrupt_stub_\vector
.endm

interrupt_common:
    pusha
    push %ds
    push %es
    push %fs
    push %gs

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    cld

    # GCC's i386 ABI expects a 16-byte-aligned stack immediately before a
    # call. Keep the original frame pointer as the sole C argument and restore
    # the interrupt stack exactly after dispatch returns.
    movl %esp, %eax
    andl $-16, %esp
    subl $12, %esp
    pushl %eax
    call interrupt_dispatch
    movl (%esp), %eax
    movl %eax, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    addl $8, %esp
    iret

INTERRUPT_NO_ERROR 0
INTERRUPT_NO_ERROR 1
INTERRUPT_NO_ERROR 2
INTERRUPT_NO_ERROR 3
INTERRUPT_NO_ERROR 4
INTERRUPT_NO_ERROR 5
INTERRUPT_NO_ERROR 6
INTERRUPT_NO_ERROR 7
INTERRUPT_WITH_ERROR 8
INTERRUPT_NO_ERROR 9
INTERRUPT_WITH_ERROR 10
INTERRUPT_WITH_ERROR 11
INTERRUPT_WITH_ERROR 12
INTERRUPT_WITH_ERROR 13
INTERRUPT_WITH_ERROR 14
INTERRUPT_NO_ERROR 15
INTERRUPT_NO_ERROR 16
INTERRUPT_WITH_ERROR 17
INTERRUPT_NO_ERROR 18
INTERRUPT_NO_ERROR 19
INTERRUPT_NO_ERROR 20
INTERRUPT_WITH_ERROR 21
INTERRUPT_NO_ERROR 22
INTERRUPT_NO_ERROR 23
INTERRUPT_NO_ERROR 24
INTERRUPT_NO_ERROR 25
INTERRUPT_NO_ERROR 26
INTERRUPT_NO_ERROR 27
INTERRUPT_NO_ERROR 28
INTERRUPT_WITH_ERROR 29
INTERRUPT_WITH_ERROR 30
INTERRUPT_NO_ERROR 31

INTERRUPT_NO_ERROR 32
INTERRUPT_NO_ERROR 33
INTERRUPT_NO_ERROR 34
INTERRUPT_NO_ERROR 35
INTERRUPT_NO_ERROR 36
INTERRUPT_NO_ERROR 37
INTERRUPT_NO_ERROR 38
INTERRUPT_NO_ERROR 39
INTERRUPT_NO_ERROR 40
INTERRUPT_NO_ERROR 41
INTERRUPT_NO_ERROR 42
INTERRUPT_NO_ERROR 43
INTERRUPT_NO_ERROR 44
INTERRUPT_NO_ERROR 45
INTERRUPT_NO_ERROR 46
INTERRUPT_NO_ERROR 47

.section .rodata
.align 4
.global interrupt_stub_table
.type interrupt_stub_table, @object
interrupt_stub_table:
.long interrupt_stub_0, interrupt_stub_1, interrupt_stub_2, interrupt_stub_3
.long interrupt_stub_4, interrupt_stub_5, interrupt_stub_6, interrupt_stub_7
.long interrupt_stub_8, interrupt_stub_9, interrupt_stub_10, interrupt_stub_11
.long interrupt_stub_12, interrupt_stub_13, interrupt_stub_14, interrupt_stub_15
.long interrupt_stub_16, interrupt_stub_17, interrupt_stub_18, interrupt_stub_19
.long interrupt_stub_20, interrupt_stub_21, interrupt_stub_22, interrupt_stub_23
.long interrupt_stub_24, interrupt_stub_25, interrupt_stub_26, interrupt_stub_27
.long interrupt_stub_28, interrupt_stub_29, interrupt_stub_30, interrupt_stub_31
.long interrupt_stub_32, interrupt_stub_33, interrupt_stub_34, interrupt_stub_35
.long interrupt_stub_36, interrupt_stub_37, interrupt_stub_38, interrupt_stub_39
.long interrupt_stub_40, interrupt_stub_41, interrupt_stub_42, interrupt_stub_43
.long interrupt_stub_44, interrupt_stub_45, interrupt_stub_46, interrupt_stub_47
.size interrupt_stub_table, . - interrupt_stub_table

.section .note.GNU-stack,"",@progbits
