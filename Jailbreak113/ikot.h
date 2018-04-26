
#define is_ipc_kobject(ikot)    ((ikot) != IKOT_NONE)
#define IKOT_NONE                 0
#define IKOT_THREAD               1
#define IKOT_TASK                 2
#define IKOT_HOST                 3
#define IKOT_HOST_PRIV            4
#define IKOT_PROCESSOR            5
#define IKOT_PSET                 6
#define IKOT_PSET_NAME            7
#define IKOT_TIMER                8
#define IKOT_PAGING_REQUEST       9
#define IKOT_MIG                 10
#define IKOT_MEMORY_OBJECT       11
#define IKOT_XMM_PAGER           12
#define IKOT_XMM_KERNEL          13
#define IKOT_XMM_REPLY           14
#define IKOT_UND_REPLY           15
/*      NOT DEFINED              16    */
#define IKOT_HOST_SECURITY       17
#define    IKOT_LEDGER           18
#define IKOT_MASTER_DEVICE       19
#define IKOT_ACT                 20
#define IKOT_SUBSYSTEM           21
#define IKOT_IO_DONE_QUEUE       22
#define IKOT_SEMAPHORE           23
#define IKOT_LOCK_SET            24
#define IKOT_CLOCK               25
#define IKOT_CLOCK_CTRL          26
#define IKOT_IOKIT_SPARE         27
#define IKOT_NAMED_ENTRY         28
#define IKOT_IOKIT_CONNECT       29
#define IKOT_IOKIT_OBJECT        30
#define IKOT_UPL                 31
#define IKOT_UNKNOWN             32    /* magic catchall    */
#define IKOT_MAX_TYPE            33    /* # of IKOT_ types    */
