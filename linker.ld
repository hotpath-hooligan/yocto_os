OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 1M;

	.text ALIGN(4K) :
	{
		KEEP(*(.multiboot))
		*(.text .text.*)
	}

	.rodata ALIGN(4K) :
	{
		*(.rodata .rodata.*)
	}

	.data ALIGN(4K) :
	{
		*(.data .data.*)
	}

	.bss ALIGN(4K) :
	{
		*(COMMON)
		*(.bss .bss.*)
		. = ALIGN(16);
	}

	/DISCARD/ :
	{
		*(.comment)
		*(.eh_frame .eh_frame.*)
		*(.note .note.*)
	}
}
