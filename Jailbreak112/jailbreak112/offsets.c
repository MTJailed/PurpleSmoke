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

//iPhone 5S on 11.2
void init_offsets_iphone61_11_2()
{
    default_offsets.kernel_base = 0xfffffff00d804000; //should be the same
    default_offsets.osdata_metaclass = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.realhost = 0;
    default_offsets.proc_task = 0;
    default_offsets.lel0_sync_vec64_long = 0xfffffff00d88d25c;
}

//iPhone 6S N71AP on 11.2
void init_offsets_iphone81_11_2()
{
    default_offsets.kernel_base = 0xfffffff01a204000;
    default_offsets.osdata_metaclass = 0xfffffff0074ac4f8;
    default_offsets.osserializer_serialize = 0xfffffff0074c2fa4;
    default_offsets.allproc = 0xfffffff007625808;
    default_offsets.realhost = 0xfffffff0075c0b98;
    default_offsets.proc_task =  0xfffffff0073e250c;
    default_offsets.lel0_sync_vec64_long = 0xfffffff01a29925c;
}

//iPhone 6S+ N66AP on 11.2
void init_offsets_iphone82_11_2_6()
{
    default_offsets.kernel_base = 0xfffffff00d804000; //should be the same
    default_offsets.osdata_metaclass = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.realhost = 0;
    default_offsets.proc_task = 0;
    default_offsets.lel0_sync_vec64_long = 0xfffffff00d88d25c;
}

//iPhone 7 on 11.2
void init_offsets_iphone93_11_2()
{
    default_offsets.kernel_base = 0xfffffff00d804000; //should be the same
    default_offsets.osdata_metaclass = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.realhost = 0;
    default_offsets.proc_task = 0;
    default_offsets.lel0_sync_vec64_long = 0xfffffff00d88d25c;
}

//iPhone 8+ on 11.2.6
void init_offsets_iphone105_11_2_6() {
    default_offsets.kernel_base = 0xfffffff00ee04000; //should be the same
    default_offsets.osdata_metaclass = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.realhost = 0;
    default_offsets.proc_task = 0;
    default_offsets.lel0_sync_vec64_long = 0xfffffff00eedd260;
}

//iPhone X on 11.2.1
void init_offsets_iphone106_11_2_1() {
    default_offsets.kernel_base = 0xfffffff001a4000; //should be the same
    default_offsets.osdata_metaclass = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.osserializer_serialize = 0;
    default_offsets.realhost = 0;
    default_offsets.proc_task = 0;
    default_offsets.lel0_sync_vec64_long = 0xfffffff01a0dd260;
    
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
    
    //iPhone 6S N71AP
    if(strncmp(kvers.machine, "iPhone8,1", sizeof("iPhone8,1")) == 0)
    {
        
        //iOS 11.2
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
    
    //iPhone 5S
    else if(strncmp(kvers.machine, "iPhone6,1", sizeof("iPhone6,1")) == 0)
    {
        //iOS 11.2
        if(strcmp(build, "15C114") == 0)
        {
            printf("Supported!\n");
            init_offsets_iphone61_11_2();
            return OFF_ERR_SUCCESS;
        } else {
            printf("Unsupported!\n");
            return OFF_ERR_UNKNOWN;
        }
        return OFF_ERR_UNKNOWN;
    }

    //iPhone 6S+ N66AP
    else if(strncmp(kvers.machine, "iPhone8,2", sizeof("iPhone8,2")) == 0)
    {
        
        //iOS 11.2.6
        if(strcmp(build, "15D100") == 0)
        {
            printf("Supported!\n");
            init_offsets_iphone82_11_2_6();
            return OFF_ERR_SUCCESS;
        } else {
            printf("Unsupported!\n");
            return OFF_ERR_UNKNOWN;
        }
        return OFF_ERR_UNKNOWN;
    }
    
    //iPhone 7
    else if(strncmp(kvers.machine, "iPhone9,3", sizeof("iPhone9,3")) == 0)
    {
        
        //iOS 11.2
        if(strcmp(build, "15C114") == 0)
        {
            printf("Supported!\n");
            init_offsets_iphone93_11_2();
            return OFF_ERR_SUCCESS;
        } else {
            printf("Unsupported!\n");
            return OFF_ERR_UNKNOWN;
        }
        return OFF_ERR_UNKNOWN;
    }
    
    //iPhone 8+
    else if(strncmp(kvers.machine, "iPhone10,5", sizeof("iPhone10,5")) == 0)
    {
        
        //iOS 11.2.6
        if(strcmp(build, "15D100") == 0)
        {
            printf("Supported!\n");
            init_offsets_iphone105_11_2_6();
            return OFF_ERR_SUCCESS;
        } else {
            printf("Unsupported!\n");
            return OFF_ERR_UNKNOWN;
        }
        return OFF_ERR_UNKNOWN;
    }
    
    //iPhone X
    else if(strncmp(kvers.machine, "iPhone10,6", sizeof("iPhone10,6")) == 0)
    {
        
        //iOS 11.2.1
        if(strcmp(build, "15C153") == 0)
        {
            printf("Supported!\n");
            init_offsets_iphone106_11_2_1();
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
