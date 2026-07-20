CROSS ?=
CC := $(CROSS)gcc
AS := $(CROSS)as
GRUB_FILE ?= grub-file
GRUB_MKRESCUE ?= grub-mkrescue
QEMU ?= qemu-system-i386

BUILD_DIR := build
KERNEL := $(BUILD_DIR)/myos.bin
ISO := $(BUILD_DIR)/myos.iso
ISO_ROOT := $(BUILD_DIR)/isodir

CFLAGS := -std=gnu11 -ffreestanding -O2 -Wall -Wextra -Werror \
	-m32 -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables \
	-fno-unwind-tables -mno-mmx -mno-sse -mno-sse2
ASFLAGS := --32
LDFLAGS := -T linker.ld -ffreestanding -nostdlib -m32 -no-pie \
	-Wl,--build-id=none -Wl,-z,noexecstack
LDLIBS := -lgcc

C_SOURCES := kernel.c gdt.c vga.c idt.c keyboard.c serial.c console.c
ASM_SOURCES := boot.s interrupts.s
OBJECTS := $(C_SOURCES:%.c=$(BUILD_DIR)/%.o) \
	$(ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)
DEPENDENCIES := $(C_SOURCES:%.c=$(BUILD_DIR)/%.d)

all: $(KERNEL)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL): linker.ld $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)
	$(GRUB_FILE) --is-x86-multiboot $@

iso: $(ISO)

$(ISO): $(KERNEL) grub.cfg
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $(ISO_ROOT)/boot/myos.bin
	cp grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(ISO_ROOT)

run: $(ISO)
	$(QEMU) -cdrom $(ISO) -boot d

run-debug: $(ISO)
	$(QEMU) -cdrom $(ISO) -boot d -serial file:$(BUILD_DIR)/debug.log

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPENDENCIES)

.PHONY: all iso run run-debug clean
