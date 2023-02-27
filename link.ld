ENTRY(_start)

__stack_size = 2048;

PROVIDE(_stack_size = __stack_size);

MEMORY {
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
	RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS {
	.vector_table : {
		KEEP(*(SORT_NONE(.vector_table)))
		. = ALIGN(64);
	} >FLASH AT>FLASH

	.text : {
		. = ALIGN(4);
		*(.rodata .rodata*)
		*(.srodata .srodata.*)
		*(.text .text.*)
		. = ALIGN(4);
	} >FLASH AT>FLASH 

	PROVIDE(_etext = .);

	.dalign : {
		. = ALIGN(4);
		PROVIDE(_data_vma = .);
	} >RAM AT>FLASH	

	.dlalign : {
		. = ALIGN(4); 
		PROVIDE(_data_lma = .);
	} >FLASH AT>FLASH

	.data : {
	  	*(.data .data.*)
		. = ALIGN(8);
	  	PROVIDE(__global_pointer$ = . + 0x800);
	  	*(.sdata .sdata.*)
		*(.sdata2.*)
	  	. = ALIGN(8);
	  	*(.srodata.cst16)
	  	*(.srodata.cst8)
	  	*(.srodata.cst4)
	  	*(.srodata.cst2)
	  	*(.srodata .srodata.*)
	  	. = ALIGN(4);
		PROVIDE(_edata = .);
	} >RAM AT>FLASH

	.bss : {
		. = ALIGN(4);
		PROVIDE(_sbss = .);
		*(.sbss*)
		*(.bss*)
		*(COMMON*)
		. = ALIGN(4);
		PROVIDE(_ebss = .);
	} >RAM AT>FLASH

	PROVIDE(_end = _ebss);
	PROVIDE(end = .);

	.stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size : {
		PROVIDE(_heap_end = .);
		. = ALIGN(4);
		PROVIDE(_susrstack = .);
		. = . + __stack_size;
		PROVIDE(_eusrstack = .);
	} >RAM 
}

