ENTRY(_start)

SECTIONS {
  . = 0x0000000;
  .text.reset : {
    *(.text.reset)
  }
  .text : {
    *(.text*)
  }
}
