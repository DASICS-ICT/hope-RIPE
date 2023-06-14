#include <stdio.h>
#include <unistd.h>
#include "udasics.h"

uint64_t umaincall_helper;

#define BOUND_REG_READ(hi,lo,idx)   \             
                    case idx:  \
                        hi = csr_read(0x883 + idx * 2);  \
                        lo = csr_read(0x884 + idx * 2);  \
                        break;

#define BOUND_REG_WRITE(hi,lo,idx)   \             
                    case idx:  \
                        csr_write(0x883 + idx * 2, hi);  \
                        csr_write(0x884 + idx * 2, lo);  \
                        break;

#define CONCAT(OP) BOUND_REG_##OP

#define LIBBOUND_LOOKUP(HI,LO,IDX,OP) \
        switch (IDX) \
        {               \
            CONCAT(OP)(HI,LO,0);  \
            CONCAT(OP)(HI,LO,1);  \
            CONCAT(OP)(HI,LO,2);  \
            CONCAT(OP)(HI,LO,3);  \
            CONCAT(OP)(HI,LO,4);  \
            CONCAT(OP)(HI,LO,5);  \
            CONCAT(OP)(HI,LO,6);  \
            CONCAT(OP)(HI,LO,7);  \
            CONCAT(OP)(HI,LO,8);  \
            CONCAT(OP)(HI,LO,9);  \
            CONCAT(OP)(HI,LO,10); \
            CONCAT(OP)(HI,LO,11); \
            CONCAT(OP)(HI,LO,12); \
            CONCAT(OP)(HI,LO,13); \
            CONCAT(OP)(HI,LO,14); \
            CONCAT(OP)(HI,LO,15); \
            default: \
                printf("\x1b[31m%s\x1b[0m","[DASICS]Error: out of libound register range\n"); \
        }


void register_udasics(uint64_t funcptr) 
{
    umaincall_helper = (funcptr != 0) ? funcptr : (uint64_t) dasics_umaincall_helper;
    csr_write(0x8a3, (uint64_t)dasics_umaincall);
    csr_write(0x005, (uint64_t)dasics_ufault_entry);
}

void unregister_udasics(void) 
{
    csr_write(0x8a3, 0);
    csr_write(0x005, 0);    
}

uint32_t dasics_sys_write_checker(uint64_t arg0,uint64_t arg1,uint64_t arg2)
{
    int32_t max_cfgs = DASICS_LIBCFG_WIDTH << 1;
    int32_t idx, is_inbound = 0;
    uint64_t bound_lo_reg,bound_hi_reg;

    for(idx = 0; idx < max_cfgs; idx++){
        LIBBOUND_LOOKUP(bound_hi_reg,bound_lo_reg,idx,READ)

        uint64_t libcfg = dasics_libcfg_get(idx);
        is_inbound = ((libcfg & DASICS_LIBCFG_V) != 0) && ((libcfg & DASICS_LIBCFG_R) != 0) &&
                        (arg1 >= bound_lo_reg) && (arg1 + arg2 <= bound_hi_reg);

        if(is_inbound) return 1;
    }

    return 0;

}


uint32_t dasics_syscall_checker(uint64_t syscall,uint64_t arg0,uint64_t arg1,uint64_t arg2)
{
    switch(syscall)
    {
        case SYS_write:
            return dasics_sys_write_checker(arg0, arg1, arg2);
            break;
        default: //TODO: other syscall implement
            printf("\x1b[33m%s\x1b[0m","[Warning] unsurported syscall for dasics!\n");
            break;
    }
}

uint32_t dasics_syscall_proxy(uint64_t syscall, uint64_t arg0, uint64_t arg1,uint64_t arg2,uint64_t arg3,uint64_t arg4,uint64_t arg5,uint64_t arg6)
{
    switch(syscall)
    {
        //TODO: implementation of more syscalls
        case SYS_write:
            return write((int)arg0, (void *)arg1, (size_t)arg2); 
        default:
            printf("\x1b[33m%s\x1b[0m","[Warning] unsurported syscall for dasics!\n");
            break;
    }
}


uint64_t dasics_umaincall_helper(UmaincallTypes type, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    uint64_t dasics_return_pc = csr_read(0x8a4);            // DasicsReturnPC
    uint64_t dasics_free_zone_return_pc = csr_read(0x8a5);  // DasicsFreeZoneReturnPC

    uint64_t retval = 0;

    switch (type)
    {
        // TODO: print supports variable arguments
        case Umaincall_PRINT:
            if (!arg0) break;
            if (arg1) goto print_arg1; 
            printf((char *)arg0);
            break;
        print_arg1:
            if (arg2) goto print_arg2;
            printf((char *)arg0, arg1);
            break;
        print_arg2:
            printf((char *)arg0, arg1, arg2);
            break;
        
        case Umaincall_WRITE:
            int32_t max_cfgs = DASICS_LIBCFG_WIDTH << 1;
            int32_t idx, is_inbound = 0;
            uint64_t bound_lo_reg,bound_hi_reg;

            for(idx = 0; idx < max_cfgs; idx++){
                LIBBOUND_LOOKUP(bound_hi_reg,bound_lo_reg,idx,READ)
                
                uint64_t libcfg = dasics_libcfg_get(idx);
                is_inbound = ((libcfg & DASICS_LIBCFG_V) != 0) && ((libcfg & DASICS_LIBCFG_R) != 0) &&
                             (arg1 >= bound_lo_reg) && (arg1 + arg2 <= bound_hi_reg);

                if(is_inbound) goto param_ok;
            }
            printf("\x1b[31m%s\x1b[0m","[ERROR] write parameter is out of bound!\n");
            break;

            param_ok:
                printf("[INFO] write parameter check OK!\n");
                write((int)arg0, (void *)arg1, (size_t)arg2);
                break;
            
            
        default:
            printf("\x1b[33m%s\x1b[0m","Warning: Invalid umaincall number %u!\n", type); //could not use printf in kernel
            break;
    }

    csr_write(0x8a4, dasics_return_pc);             // DasicsReturnPC
    csr_write(0x8a5, dasics_free_zone_return_pc);   // DasicsFreeZoneReturnPC

    return retval;
}

void dasics_ufault_handler(void)
{
    // Save some registers that should be saved by callees
    uint64_t dasics_return_pc = csr_read(0x8a4);
    uint64_t dasics_free_zone_return_pc = csr_read(0x8a5);

    uint64_t ucause = csr_read(ucause);
    uint64_t utval = csr_read(utval);
    uint64_t uepc = csr_read(uepc);

    uint64_t arg0,arg1,arg2,syscall;
    __asm__ volatile(
        "mv t0, a0\n"
        "mv t1, a1\n"
        "mv t2, a2\n"
        "mv t3, a7\n"
        "sd t0, %[arg0]\n"
        "sd t1, %[arg1]\n"
        "sd t2, %[arg2]\n"
        "sd t3, %[syscall]\n"
        : [arg0] "=m" (arg0),[arg1] "=m" (arg1),[arg2] "=m" (arg2), [syscall] "=m" (syscall)
        :
        : "memory"
    );

    switch(ucause)
    {
        case EXC_DASICS_UFETCH_FAULT:
            printf("[DASICS EXCEPTION]Info: dasics ufetch fault occurs, ucause = 0x%lx, uepc = 0x%lx, utval = 0x%lx\n", ucause, uepc, utval);
            break;
        case EXC_DASICS_ULOAD_FAULT:
            printf("[DASICS EXCEPTION]Info: dasics uload fault occurs, ucause = 0x%lx, uepc = 0x%lx, utval = 0x%lx\n", ucause, uepc, utval);
            break;
        case EXC_DASICS_USTORE_FAULT:
            printf("[DASICS EXCEPTION]Info: dasics ustore fault occurs, ucause = 0x%lx, uepc = 0x%lx, utval = 0x%lx\n", ucause, uepc, utval);
            break;
        case EXC_DASICS_UECALL_FAULT:
            //printf("[DASICS EXCEPTION]Info: dasics uecall fault occurs, ucause = 0x%lx, uepc = 0x%lx, utval = 0x%lx\n", ucause, uepc, utval);
            printf("[DASICS EXCEPTION]Info: dasics lib ecall occurs (ucause = 0x%lx, uepc = 0x%lx, utval = 0x%lx), try to check arguments...\n", ucause, uepc, utval);
            if(dasics_syscall_checker(syscall,arg0, arg1, arg2)){
                    printf("[DASICS EXCEPTION]Info: lib ecall arguments OK! sycall number:%d, syscall is permitted \n", syscall);   
                    //__asm__ volatile ("ecall\n" );    
                    //TODO: implementation of all syscalls
                    uint32_t ret = dasics_syscall_proxy(syscall, arg0, arg1, arg2, 0, 0, 0,0);
                    csr_write(uepc, uepc + 4);         
                    csr_write(0x8a4, dasics_return_pc);
                    csr_write(0x8a5, dasics_free_zone_return_pc);
                    return;
            } 
            printf("\x1b[31m%s\x1b[0m","[DASICS EXCEPTION]Error: lib ecall arguments beyond authority， dasics ecall fault occurs!\n");
            break;
        default:
            printf("\x1b[31m%s\x1b[0m","[DASICS EXCEPTION]Error: unexpected dasics fault occurs, ucause = 0x%lx, uepc = 0x%lx, utval = 0x%lx\n", ucause, uepc, utval);
            break;
    }
    printf("[DASICS EXCEPTION]Info: dasics_return_pc:0x%lx\n", dasics_return_pc);	

    // switch base on cause types.
    // currently just skip this inst.
    csr_write(uepc, uepc + 4);
    // rvc will compress jump/branch inst.
    //if (ucause == EXC_DASICS_UFETCH_FAULT)
      //  csr_write(uepc, uepc + 2);
    //else 
       // csr_write(uepc, uepc + 4); 

    // Restore those saved registers
    csr_write(0x8a4, dasics_return_pc);
    csr_write(0x8a5, dasics_free_zone_return_pc);

}

int32_t dasics_libcfg_alloc(uint64_t cfg, uint64_t hi, uint64_t lo)
{
    uint64_t libcfg0 = csr_read(0x881);  // DasicsLibCfg0
    uint64_t libcfg1 = csr_read(0x882);  // DasicsLibCfg1

    int32_t max_cfgs = DASICS_LIBCFG_WIDTH << 1;
#ifndef RV32
    int32_t step = 8;  // rv64
#else
    int32_t step = 4;  // rv32
#endif

    int32_t idx;

    for (idx = 0; idx < max_cfgs; ++idx)
    {
        int choose_libcfg0 = (idx < DASICS_LIBCFG_WIDTH);
        int32_t _idx = choose_libcfg0 ? idx : idx - DASICS_LIBCFG_WIDTH;
        uint64_t curr_cfg = ((choose_libcfg0 ? libcfg0 : libcfg1) >> (_idx * step)) & DASICS_LIBCFG_MASK;

        if ((curr_cfg & DASICS_LIBCFG_V) == 0)  // Find avaliable cfg
        {
            if (choose_libcfg0)
            {
                libcfg0 &= ~(DASICS_LIBCFG_MASK << (_idx * step));
                libcfg0 |= (cfg & DASICS_LIBCFG_MASK) << (_idx * step);
                csr_write(0x881, libcfg0);  // DasicsLibCfg0
            }
            else
            {
                libcfg1 &= ~(DASICS_LIBCFG_MASK << (_idx * step));
                libcfg1 |= (cfg & DASICS_LIBCFG_MASK) << (_idx * step);
                csr_write(0x882, libcfg1);  // DasicsLibCfg1
            }

            // Write DASICS boundary csrs     
            LIBBOUND_LOOKUP(hi,lo,idx,WRITE)       
            
            return idx;
        }
    }

    return -1;
}

int32_t dasics_libcfg_free(int32_t idx)
{
    if (idx < 0 || idx >= (DASICS_LIBCFG_WIDTH << 1))
    {
        return -1;
    }

#ifndef RV32
    int32_t step = 8;  // rv64
#else
    int32_t step = 4;  // rv32
#endif

    int choose_libcfg0 = (idx < DASICS_LIBCFG_WIDTH);
    int32_t _idx = choose_libcfg0 ? idx : idx - DASICS_LIBCFG_WIDTH;

    uint64_t libcfg = choose_libcfg0 ? csr_read(0x881):  // DasicsLibCfg0
                                       csr_read(0x882);  // DasicsLibCfg1
    libcfg &= ~(DASICS_LIBCFG_V << (_idx * step));

    if (choose_libcfg0)
    {
        csr_write(0x881, libcfg);  // DasicsLibCfg0
    }
    else
    {
        csr_write(0x882, libcfg);  // DasicsLibCfg1
    }

    return 0;
}

uint32_t dasics_libcfg_get(int32_t idx)
{
    if (idx < 0 || idx >= (DASICS_LIBCFG_WIDTH << 1))
    {
        return -1;
    }

#ifndef RV32
    int32_t step = 8;  // rv64
#else
    int32_t step = 4;  // rv32
#endif

    int choose_libcfg0 = (idx < DASICS_LIBCFG_WIDTH);
    int32_t _idx = choose_libcfg0 ? idx : idx - DASICS_LIBCFG_WIDTH;

    uint64_t libcfg = choose_libcfg0 ? csr_read(0x881):  // DasicsLibCfg0
                                       csr_read(0x882);  // DasicsLibCfg1

    return (libcfg >> (_idx * step)) & DASICS_LIBCFG_MASK;
}


void dasics_print_cfg_register(int32_t idx)
{
	printf("DASICS uLib CFG Registers: idx:%x  config: %x \n",idx,dasics_libcfg_get(idx));
}

