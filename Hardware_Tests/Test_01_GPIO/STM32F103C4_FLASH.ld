ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size = 0;
_Min_Stack_Size = 0x400;


MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 6K
}


SECTIONS
{
    /*
     * Interrupt Vector Table
     */

    .isr_vector :
    {
        . = ALIGN(4);

        KEEP(*(.isr_vector))

        . = ALIGN(4);
    } > FLASH


    /*
     * Program Code
     */

    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text*)

        *(.rodata)
        *(.rodata*)

        . = ALIGN(4);

        _etext = .;
    } > FLASH


    /*
     * Initialized Data
     */

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);

        _sdata = .;

        *(.data)
        *(.data*)

        . = ALIGN(4);

        _edata = .;
    } > RAM AT > FLASH


    /*
     * Uninitialized Data
     */

    .bss :
    {
        . = ALIGN(4);

        _sbss = .;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
    } > RAM


    /*
     * Stack / Heap check
     */

    ._user_heap_stack :
    {
        . = ALIGN(8);

        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;

        . = ALIGN(8);
    } > RAM


    /*
     * Discard unwanted sections
     */

    /DISCARD/ :
    {
        *(.note.GNU-stack)
        *(.comment)
    }
}
