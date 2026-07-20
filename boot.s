# Multiboot header (32-bit compatible)
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM


# Block started by Symbol
# Uninitialized global and static variables
# Reserve a stack for the initial thread
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The kernel entry point
.section .text
.global _start
.type _start, @function
_start:
    # Multiboot enters in 32-bit protected mode, but the interrupt and
    # direction flags are not part of our C ABI contract.
    cli
    cld

    # Set up stack (32-bit mode)
    mov $stack_top, %esp
    xor %ebp, %ebp
    
    # Call the kernel main function
    call kernel_main
    
    # If kernel_main returns (shouldn't happen), halt the system
    cli
1:  hlt
    jmp 1b

.size _start, . - _start

# Tell the linker that this assembly does not require an executable stack.
.section .note.GNU-stack,"",@progbits
