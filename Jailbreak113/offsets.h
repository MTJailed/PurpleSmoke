#include <stdio.h>
#include <stdlib.h>
#pragma once
#ifndef kernel_offsets
typedef struct {
    uint64_t off_kernel_base;
    uint64_t off_agx_cmdq_vtable;
    uint64_t off_osdata_metaclass;
    uint64_t off_osserializer_serialize;
    uint64_t off_kuuid_copy;
    uint64_t off_allproc;
    uint64_t off_realhost;
    uint64_t off_call5;
    uint64_t off_proc_task;
} kernel_offsets;

int init_offsets(void);
#endif
