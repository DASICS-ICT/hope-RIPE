# RIPE: The Runtime Intrusion Prevention Evaluator for RISC-V

This branch contains a modified, RISC-V compatible implementation of RIPE.
RIPE was originally developed by John Wilander and Nick Nikiforakis and presented
at the 2011 Annual Computer Security Applications Conference in Orlando, Florida.

The RIPE port to RISC-V was developed by John Merrill.
RIPE for RISC-V is designed for use with the Spike simulator.

RIPE is free software and released under the MIT licence (see file named LICENSE).

## Build & Run

#### For individual tests:

1. Build the RIPE backend: `make`

2. Run a test with specified parameters: `<spike or other run command> -t <technique> -i <attack code> -c <code pointer> -l <location> -f <function> -d <optional debug output>`

#### For the full testbench:

1. Start the RIPE frontend: `./ripe_tester.py -o <output file(optional)>`

- Additional options can be seen by running `./ripe_tester.py -h`
- For more info on attack parameters, view the [Attack Parameters](#attack-parameters) section.

## Test Results

RIPE produces and performs a series of exploits based on its five attack parameters: technique (direct or indirect), attack code, target code pointer, memory location, and vulnerable function.

The result log generated by the frontend labels each test as one of the following:
  - OK: the attack executes successfully
  - FAIL: the attack encounters an error before running to completion
  - NOT POSSIBLE: the attack is not practically possible (eg. a direct attack on a stack buffer targeting a global pointer)
  
## Attack Parameters

The RIPE testbed on RISC-V is based upon five attack parameters which combine to yield 948 buffer overflow attacks. These are:

#### Location

The attack location describes the memory section in which the target buffer is located. RIPE supports attacks on the `stack`, `heap`, `data`, and `bss` sections.

#### Attack code

RIPE presents four options for attack code:
- `returnintolibc`: A simulated return-into-libc attack which redirects the target pointer to the entry point of an otherwise inaccessible function
- `nonop`: A simple shellcode which performs a similar transfer of control flow
- `rop`: A variant of the return-into-libc attack code which instead jumps to an instruction that is *not* a function entry point. This simulates the initiation of a ROP-style attack, as it directs the PC to an illegal jump target.
- `dataonly`: Manipulates non-control data, resulting in a mock privilege escalation or data leak.

#### Target Code Pointer

The target code pointer is overwritten by the overflow such that control of the program is transferred to the attack code. RIPE includes the following target pointers:

- `ret`: return address of the perform_attack() function
- Function pointers in each memory location:
  - `funcptrstackvar`, `funcptrstackparam`, `funcptrheap`, `funcptrdata`, and `funcptrbss`
- Structs containing adjacent buffers and function pointers:
  - `structfuncptrstack`, `structfuncptrheap`, `structfuncptrdata`, and `structfuncptrbss`
- Longjmp buffers:
  - `longjmpstackvar`, `longjmpstackparam`, `longjmpheap`, `longjmpdata`, `longjmpbss`
- Data-only attacks offer two choices of attack vector:
  - `bof` edits a numerical variable that is later used as a branch condition
  - `iof` fills the buffer with 256 junk characters, causing integer overflow on an 8-bit length variable. This alters a pointer offset such that an arbitrary address is overwritten.
  - `leak` prints addresses beyond the bounds of the buffer, resulting in exfiltration of otherwise unreachable data.

#### Overflow Technique

Buffer overflows can be performed with or without indirection. 
- The `direct` technique simply overwrites a target pointer in the same memory location as the overflow buffer. Direct, data-only attacks overwrite a target pointer with an integer value.
- The `indirect` technique initially targets a generic pointer that is adjacent to the buffer. A dereference redirects this pointer to the attack code. Indirect overflows work between memory regions (i.e. from a stack buffer to a heap pointer). Indirect, data-only attacks overwrite a pointer to the target with a pointer elsewhere in memory.

#### Function

There are nine vulnerable functions available as attack vectors:

- `memcpy`
- `homebrew`, a loop-based, memcpy() equivalent
- C library string functions, including: `str(n)cpy`, `str(n)cat`, `s(n)printf` 
- `sscanf` via a format string vulnerability
