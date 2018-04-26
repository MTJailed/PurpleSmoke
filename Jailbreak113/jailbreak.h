#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <mach/mach.h>
#include <pthread.h>
#include <dlfcn.h>
#include "offsets.h"
#include "ikot.h"
extern kernel_offsets default_offsets;
int jailbreak(void);
