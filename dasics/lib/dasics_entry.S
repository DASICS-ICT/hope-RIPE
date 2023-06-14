#--------------------------------------------------------------------
# dasics_ufault_entry: entry function before handling dasics faults.
#--------------------------------------------------------------------

.section .text
.global dasics_ufault_entry
.type dasics_ufault_entry, @function
.align 2, 0x13  # nop = addi x0, x0, 0

dasics_ufault_entry: 
    addi sp, sp, -16
    sd   ra, 8(sp)
    call dasics_ufault_handler
    ld   ra, 8(sp)
    addi sp, sp, 16
    uret 

#-----------------------------------------------------------------------
# dasics_umaincall: pass arguments to umaincall helper.
#-----------------------------------------------------------------------
.section .text
.global dasics_umaincall
.type dasics_umaincall, @function
.extern uint64_t umaincall_helper 

dasics_umaincall:
    addi sp, sp, -16
    sd   ra, 8(sp)
    ld   t0, umaincall_helper
    jalr t0
    ld   ra, 8(sp)
    addi sp, sp, 16
    .word 0x0000f00b    # pulpret in little endian
