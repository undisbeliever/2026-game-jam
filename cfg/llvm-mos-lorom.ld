/*
 * llvm-mos Super Nintendo 256KB FastROM LOROM memory map
 */

PROVIDE(_fixed_size = 0x400);


__rc0 = 0x00;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x001f, "Inconsistent zero page map.")

/* 0x100 bytes of stack from 0x100 - 0x1ff */
__STACK_BOTTOM = 0x01ff;

MEMORY {
    zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)
    ram (rw) : ORIGIN = 0x200, LENGTH = 0xfe00
    wram7e (rw) : ORIGIN = 0x7e2000, LENGTH = 0xe000
    wram7f (rw) : ORIGIN = 0x7f0000, LENGTH = 0x10000

    /* Fixed code used in all code banks */
    fixed (rx) : ORIGIN = 0x808000, LENGTH = _fixed_size

    /* Code + Data banks */
    bank80 (rx) : ORIGIN = 0x808000 + _fixed_size, LENGTH = 0x8000 - _fixed_size
    bank81 (rx) : ORIGIN = 0x818000 + _fixed_size, LENGTH = 0x8000 - _fixed_size
    bank82 (rx) : ORIGIN = 0x828000 + _fixed_size, LENGTH = 0x8000 - _fixed_size
    bank83 (rx) : ORIGIN = 0x838000 + _fixed_size, LENGTH = 0x8000 - _fixed_size

    /* Data only banks */
    bank84 (r)  : ORIGIN = 0x848000, LENGTH = 0x8000
    bank85 (r)  : ORIGIN = 0x858000, LENGTH = 0x8000
    bank86 (r)  : ORIGIN = 0x868000, LENGTH = 0x8000
    bank87 (r)  : ORIGIN = 0x878000, LENGTH = 0x8000
}

REGION_ALIAS("c_readonly", bank80)
REGION_ALIAS("c_writeable", ram)


SECTIONS {
  .rodata.bank80 : { *(.rodata.bank80 .rodata.bank80.*) } >.bank80
  .rodata.bank81 : { *(.rodata.bank81 .rodata.bank81.*) } >.bank81
  .rodata.bank82 : { *(.rodata.bank82 .rodata.bank82.*) } >.bank82
  .rodata.bank83 : { *(.rodata.bank83 .rodata.bank83.*) } >.bank83
  .rodata.bank84 : { *(.rodata.bank84 .rodata.bank84.*) } >.bank84
  .rodata.bank85 : { *(.rodata.bank85 .rodata.bank85.*) } >.bank85
  .rodata.bank86 : { *(.rodata.bank86 .rodata.bank86.*) } >.bank86
  .rodata.bank87 : { *(.rodata.bank87 .rodata.bank87.*) } >.bank87

  INCLUDE c.ld

  .text.bank80 : { *(.text.bank80 .text.bank80.*) } >.bank80
  .text.bank81 : { *(.text.bank81 .text.bank81.*) } >.bank81
  .text.bank82 : { *(.text.bank82 .text.bank82.*) } >.bank82
  .text.bank83 : { *(.text.bank83 .text.bank83.*) } >.bank83

  /* ::TODO Test:: */
  .bss : { *(.bss .bss.*) } > ram
  .wram7e : { *(.bss.wram7e .bss.wram7e.*) } > wram7e
  .wram7f : { *(.bss.wram7e .bss.wram7e.*) } > wram7f


  .text.interrpts : { *(.text.interrupts) } > bank80

  .sfc_header 0x80ffc0 : {
    /* ROM Name 21 spaces */
    BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20)
    BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20)
    BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20)
    BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20)
    BYTE(0x20)

    BYTE(0x30)      /* $ffd5 - map mode (LoROM FastROM) */
    BYTE(0)         /* $ffd6 - cartridge type (no SRAM) */
    BYTE(8)         /* $ffd7 - ROM size */
    BYTE(0)         /* $ffd8 - RAM size */
    BYTE(0)         /* $ffd9 - Destination code (Japan) */
    BYTE(0)         /* $ffda - developer ID (Unlicensed) */
    BYTE(0x00)      /* $ffdb - ROM version */
    SHORT(0xaaaa)   /* $ffdc - checksum compliment */
    SHORT(0x5555)   /* $ffde - checksum */
  } > bank80

  .vectors 0x80ffe0 : {
    /* native vectors */
    LONG(0)
    SHORT(CopIsr) SHORT(BreakIsr) SHORT(AbortIsr) SHORT(NmiIsr) SHORT(0) SHORT(IrqIsr)
    /* emu vectors */
    LONG(0)
    SHORT(UnusedIsr) SHORT(0) SHORT(UnusedIsr) SHORT(UnusedIsr) SHORT(Reset) SHORT(UnusedIsr)
  } > bank80
}

OUTPUT_FORMAT {
    /* Code + Data banks */
    FULL(fixed)  FULL(bank80)
    FULL(fixed)  FULL(bank81)
    FULL(fixed)  FULL(bank82)
    FULL(fixed)  FULL(bank83)

    /* Data only banks */
    FULL(bank84)
    FULL(bank85)
    FULL(bank86)
    FULL(bank87)
}

