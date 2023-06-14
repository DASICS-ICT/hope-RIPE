#ifndef _UDASICS_H_
#define _UDASICS_H_

#include <stdio.h>
#include "ucsr.h"
#include "uattr.h"

/* Add dasics exceptions */
#define EXC_DASICS_UFETCH_FAULT  24
#define EXC_DASICS_ULOAD_FAULT   26
#define EXC_DASICS_USTORE_FAULT  28
#define EXC_DASICS_UECALL_FAULT  30

/* DASICS Main csrs */
#define CSR_DUMCFG          0x5c0
#define CSR_DUMBOUNDHI      0x5c1
#define CSR_DUMBOUNDLO      0x5c2

/* DASICS Main cfg */
#define DASICS_MAINCFG_MASK 0xfUL
#define DASICS_UCFG_CLS     0x8UL
#define DASICS_UCFG_ENA     0x2UL

/* DASICS Lib csrs */
#define CSR_DLCFG0          0x881
#define CSR_DLCFG1          0x882

#define CSR_DLBOUND0        0x883
#define CSR_DLBOUND1        0x884
#define CSR_DLBOUND2        0x885
#define CSR_DLBOUND3        0x886
#define CSR_DLBOUND4        0x887
#define CSR_DLBOUND5        0x888
#define CSR_DLBOUND6        0x889
#define CSR_DLBOUND7        0x88a
#define CSR_DLBOUND8        0x88b
#define CSR_DLBOUND9        0x88c
#define CSR_DLBOUND10       0x88d
#define CSR_DLBOUND11       0x88e
#define CSR_DLBOUND12       0x88f
#define CSR_DLBOUND13       0x890
#define CSR_DLBOUND14       0x891
#define CSR_DLBOUND15       0x892
#define CSR_DLBOUND16       0x893
#define CSR_DLBOUND17       0x894
#define CSR_DLBOUND18       0x895
#define CSR_DLBOUND19       0x896
#define CSR_DLBOUND20       0x897
#define CSR_DLBOUND21       0x898
#define CSR_DLBOUND22       0x899
#define CSR_DLBOUND23       0x89a
#define CSR_DLBOUND24       0x89b
#define CSR_DLBOUND25       0x89c
#define CSR_DLBOUND26       0x89d
#define CSR_DLBOUND27       0x89e
#define CSR_DLBOUND28       0x89f
#define CSR_DLBOUND29       0x8a0
#define CSR_DLBOUND30       0x8a1
#define CSR_DLBOUND31       0x8a2

#define CSR_DMAINCALL       0x8a3
#define CSR_DRETURNPC       0x8a4
#define CSR_DFZRETURN       0x8a5

/* DASICS Lib cfg */
#define DASICS_LIBCFG_WIDTH 8
#define DASICS_LIBCFG_MASK  0xfUL
#define DASICS_LIBCFG_V     0x8UL
#define DASICS_LIBCFG_X     0x4UL
#define DASICS_LIBCFG_R     0x2UL
#define DASICS_LIBCFG_W     0x1UL

// TODO: Add UmaincallTypes
typedef enum {
    Umaincall_PRINT, 
    Umaincall_WRITE,
    Umaincall_UNKNOWN
} UmaincallTypes;

// suported syscall
#define SYS_write 64

void register_udasics(uint64_t funcptr);
void unregister_udasics(void);
uint32_t dasics_syscall_checker(uint64_t syscall,uint64_t arg0,uint64_t arg1,uint64_t arg2);
uint64_t dasics_umaincall_helper(UmaincallTypes type, uint64_t arg0, uint64_t arg1, uint64_t arg2);
void     dasics_ufault_handler(void);
int32_t  dasics_libcfg_alloc(uint64_t cfg, uint64_t hi, uint64_t lo);
int32_t  dasics_libcfg_free(int32_t idx);
uint32_t dasics_libcfg_get(int32_t idx);
void dasics_print_cfg_register(int32_t idx);

// extern uint64_t umaincall_helper;
extern void dasics_ufault_entry(void);
extern uint64_t dasics_umaincall(UmaincallTypes type, uint64_t arg0, uint64_t arg1, uint64_t arg2);

#endif
