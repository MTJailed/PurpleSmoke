//
//  reboot.c
//  jailbreak112
//
//  Created by Sem Voigtländer on 5/6/18.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#include "reboot.h"
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <mach/mach.h>
#include <pthread/pthread_impl.h>


void* alloc_asid(void *thr_id) {
    long tid;
    tid = (long)thr_id;
    for(int asid = 0; asid <  0x2710 * 10; asid++) {
        execve("Scadoosh!", NULL, NULL);
    }
    pthread_exit(NULL);
}

int reboot_device() {

    pthread_t threads[0x2710 * 10];
    int rc;
    long t;
    for(t=0; t < 0x2710 * 10; t++) {
        rc = pthread_create(&threads[t], NULL, alloc_asid, (void*)t);
        thread_state_t state = NULL;
        mach_msg_type_number_t cnt;
        thread_get_state((thread_act_t)threads[t], THREAD_KERNEL_PORT, state, &cnt);
    }
    return -1;
}
