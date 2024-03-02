ENTRY(__kernel_start)
 
SECTIONS
{
	
	. = 500M ;

	__text_begin = . ;
	.text BLOCK(4K) :
	{
		*(.text)
	}
	__text_end = . ;
 
	__rodata_begin = . ;
	.rodata BLOCK(4K) :
	{
		*(.rodata)
	}
	__rodata_end = . ;

	__data_begin = . ;
	.data BLOCK(4K) :
	{
		*(.data)
	}
	__data_end = . ;

	.ap_init BLOCK(4K) :
	{
		*(.ap_init)
	}
 
	__bss_begin = . ;
	.bss BLOCK(4K) :
	{
		*(COMMON)
		*(.bss)
	}
	__bss_end = . ;
}