//
//  offsets.c
//  jailbreak112
//
//  Created by Sem Voigtländer on 19/04/2018.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#include "offsets.h"
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <string.h>
#define OFF_ERR_SUCCESS 0x0
#define OFF_ERR_UNSUPPORTED 0x1
#define OFF_ERR_UNKNOWN 0x02

kernel_offsets default_offsets;
void init_offsets_iphone81_11_2()
{
    default_offsets.off_kernel_base = 0xfffffff007004000;
    default_offsets.off_agx_cmdq_vtable = 0xfffffff006ff9330;
    default_offsets.off_osdata_metaclass = 0xfffffff0074ac4f8;
    default_offsets.off_osserializer_serialize = 0xfffffff0074c2fa4;
    default_offsets.off_kuuid_copy = 0xfffffff0074cdf3c;
    default_offsets.off_allproc = 0xfffffff007625808;
    default_offsets.off_realhost = 0xfffffff0075c0b98;
    default_offsets.off_call5 = 0xfffffff006249e90;
    default_offsets.off_proc_task =  0xfffffff0073e250c;
}

char* get_ios_build()
{

        int mib[2] = {CTL_KERN, KERN_OSVERSION};
        size_t size = 0;
        
        // Get the size for the buffer
        sysctl(mib, 2, NULL, &size, NULL, 0);
        
        char *answer = malloc(size);
        int result = sysctl(mib, 2, answer, &size, NULL, 0);
        if(result < 0)
        {
            printf("Sysctl failed.\n");
        }
        return answer;
}



int init_offsets()
{
    //Get the kernel and build info
    struct utsname kvers = {0};
    uname(&kvers);
    char* build = get_ios_build();
    printf("kernel: %s\n", kvers.version);
    printf("build: %s\n",build);
    
    //Start initialisation of offsets
    if(strncmp(kvers.machine, "iPhone8,1", sizeof("iPhone8,1")) == 0)
    {
        if(strcmp(build, "15C114") == 0)
        {
            printf("Supported!\n");
            init_offsets_iphone81_11_2();
            return OFF_ERR_SUCCESS;
        } else {
            printf("Unsupported!\n");
            return OFF_ERR_UNKNOWN;
        }
        return OFF_ERR_UNKNOWN;
    }
    else
    {
        printf("%s is not supported (yet).\n", kvers.machine);
        return OFF_ERR_UNSUPPORTED;
    }
    return OFF_ERR_UNKNOWN;
}
