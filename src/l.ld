ENTRY(_start)

SECTIONS {
  . = 0;
  .text.reset : {
    *(.text.reset)
  }
  .text : {
    *(.text*)
  }
}
