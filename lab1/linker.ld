ENTRY (_start)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x80000, LENGTH = 128K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}
SECTIONS
{
  .text :
  {
    . = ORIGIN(FLASH); /* same as ". = 0x80000" */
    KEEP(*(.isr_vector))
    _stext = .;
    *(.text)
    _etext = .;
    _sidata = .;
  } >FLASH

  .data :
  {
    _sdata = .;
    *(.data)
    _edata = .;
  } >RAM

  .bss :
  {
    .sbss = .;
    *(.bss)
    _ebss = .;
  } >RAM

  _stack_top = ORIGIN(RAM) + LENGTH(RAM); /* stack start from bottom of RAM */
}