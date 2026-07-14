# Emulation notes & fix history

This emulator started as a working-but-fragile interpreter and was hardened into
one that runs every bundled ÖverDOS program cleanly on modern 64-bit Linux,
macOS and Windows. This document records what the machine models, the bugs that
were fixed along the way, and how portability is achieved. It doubles as a tour
of the more interesting corners of MIPS system emulation.

## What the machine models

- **CPU** — big-endian MIPS-II (R3000-class), all 71 integer instructions the
  compiled OS and its programs use, with branch delay slots.
- **CP0** — enough of the system coprocessor for the OS: `Status` (IE/EXL and
  the interrupt masks), `Cause`, `EPC`, `Count`/`Compare` (timer), `BadVAddr`,
  and `ERET`.
- **MMU** — a linked list of memory regions with unsigned address matching.
  Regions can carry a callback, which is how memory-mapped devices are
  implemented.
- **Devices** — a 16550-style UART (the serial console), a real-time clock, a
  timer, LEDs, and the interrupt controller, plus SPIM-style memory-mapped
  console I/O at `0xFFFF0000` for the SPIM test programs.
- **Console** — a VT100/ANSI serial terminal in raw, non-blocking mode.

## Faithful-to-hardware fixes

These reproduce quirks of the real board that the OS quietly depended on. They
were pinned down while building the browser port and apply equally here.

10. **HI/LO registers are not preserved across interrupts.** The kernel's
    interrupt handler (`regs2pcb.sx` `save_regs`) clobbers `HI`/`LO` on every
    interrupt — it does `mul $k0,$k0,$k1` to compute a PCB offset — and the PCB
    has no slot to save them. So a user-mode multiply/divide result still *in
    flight* (computed but not yet read out with `mflo`/`mfhi`) when a timer tick
    lands is silently corrupted. `itoa()` — used to format every colour code and
    cursor coordinate — is full of `div`/`mul`, so the damage scaled with redraw
    rate (garbled `tetris`/`nibbles`, dropped ANSI escapes, "blocks left
    behind"). On real 67 MHz hardware the interrupt almost never landed in that
    tiny window; a fast interpreter hits it constantly. Fixed by **virtualising
    HI/LO per process**: shadow `HI`/`LO` keyed by the kernel's `pcb_executing`,
    saved on exception entry and restored on `ERET`, so each process keeps its
    own HI/LO across interrupts and context switches.

11. **Unaligned load/store dropped bytes (`LWL`/`LWR`/`SWL`/`SWR`).** The worst
    was `SWR`: a big-endian unaligned word store is emitted by the compiler as
    `swl R,0(A); swr R,3(A)`, and when `A ≡ 1 (mod 4)` the `swr` target is
    word-aligned, so the old code's `len = address − (address & ~3)` was `0` and
    **the low byte was never stored** — silent memory corruption on every
    unaligned store to an address `≡ 1 (mod 4)`. `top` (PCB structs, `qsort`
    element swaps, heavy formatting) tripped it constantly, and corrupting an OS
    global made the damage persist into the *next* program (raw escape codes /
    garbage after `top` exited). All four are now reimplemented with standard
    MIPS big-endian semantics and unit-tested at every alignment.

12. **Swedish letters å/ä/ö.** The OS speaks ISO-8859-1 (ä=`0xE4`, ö=`0xF6`,
    å=`0xE5`). Since ISO-8859-1 is exactly Unicode `U+0000..U+00FF`, the console
    layer maps cleanly on both hosts — see *Portability* below.

## Portability

Only two files contain platform-specific code, both behind `#ifdef _WIN32`:

- **`console.c`** — the terminal layer.
  - *Windows*: the console is put into ANSI/VT100 mode and code page 1252, and
    input is read as wide (Unicode) characters, so a Latin-1 byte round-trips
    directly.
  - *POSIX*: `termios` puts the terminal into raw, non-blocking, no-echo mode
    (leaving `ISIG` on so Ctrl-C quits, and output post-processing on so a bare
    `\n` still becomes CR+LF). Output bytes ≥ 0x80 are encoded as two-byte UTF-8
    and typed characters are UTF-8 decoded back to their byte, so `å ä ö` work in
    a modern UTF-8 terminal. The original terminal mode is saved and restored on
    exit and on Ctrl-C.
- **`isa_exec.c`** — the wall-clock timer uses `QueryPerformanceCounter` on
  Windows and `clock_gettime(CLOCK_MONOTONIC)` elsewhere, behind a single
  `host_ns()` helper.

Host byte order is detected at compile time via the compiler's `__BYTE_ORDER__`
macro (gcc/clang), so no `-D` flag is needed. Everything else — the interpreter,
MMU, ELF loader, device callbacks — is plain portable C99 over the standard
library only.
