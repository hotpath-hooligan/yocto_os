# Yocto OS

Yocto OS is a small educational 32-bit x86 kernel written in C and assembly.
It boots with GRUB, displays a VGA text terminal, accepts interrupt-driven PS/2
keyboard input, and provides a basic command prompt.

This project is unrelated to the Linux-focused Yocto Project. It targets legacy
BIOS-style hardware and is primarily intended for learning and QEMU experiments.

## Features

- Multiboot 1 boot through GRUB
- 32-bit protected-mode GDT and IDT
- Remapped 8259 PIC with keyboard interrupts
- PS/2 keyboard input with Shift and Caps Lock support
- VGA text output, hardware cursor, and screen scrolling
- COM1 serial diagnostics
- CPU idle loop using `hlt`
- Allocation-free command prompt

## Requirements

- GCC with 32-bit multilib support, or an `i686-elf` cross-compiler
- GNU Make and binutils
- GRUB tools and xorriso
- QEMU for running the OS

On Debian or Ubuntu:

```sh
sudo apt-get install build-essential gcc-multilib grub-pc-bin grub-common xorriso qemu-system-x86
```

## Build and run

```sh
make          # Build build/myos.bin
make iso      # Create build/myos.iso
make run      # Start the ISO in QEMU
make clean    # Remove generated files
```

To use an `i686-elf` cross-compiler:

```sh
make CROSS=i686-elf-
```

Use `make run-debug` to save serial output to `build/debug.log`.

## Commands

| Command | Purpose |
| --- | --- |
| `help` | List available commands |
| `about` | Show a short kernel description |
| `echo <text>` | Print text to the terminal |
| `clear` | Clear the terminal |
| `reboot` | Reset through the PS/2 controller |

Enter executes a command, Backspace edits it, Tab inserts four spaces, and
Escape clears the screen and starts a fresh prompt.

## Project structure

```text
boot.s / interrupts.s   Boot and interrupt entry assembly
kernel.c / idt.c        Initialization, IDT, PIC, and IRQ dispatch
keyboard.c / console.c  Keyboard decoding and built-in commands
vga.c / serial.c        VGA terminal and COM1 diagnostics
gdt.c / linker.ld       Protected-mode segments and memory layout
Makefile / grub.cfg     Build and boot configuration
```
