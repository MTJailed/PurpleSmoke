#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
typedef struct{
    uint64_t kernel_base;
    uint64_t osdata_metaclass;
    uint64_t osserializer_serialize;
    uint64_t allproc;
    uint64_t kernproc;
    uint64_t realhost;
    uint64_t proc_task;
    uint64_t lel0_sync_vec64_long;
} kernel_offsets;

int init_offsets(void);
extern kernel_offsets default_offsets;
