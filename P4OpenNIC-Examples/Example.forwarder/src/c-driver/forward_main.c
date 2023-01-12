/*
-- (c) Copyright 2019 Xilinx, Inc. All rights reserved.
--
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
--
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
--
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
--
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
--------------------------------------------------------------------------------
*/

/****************************************************************************************************************************************************/
/* SECTION: Header includes */
/****************************************************************************************************************************************************/

/*
 * The example designs include file should be present in the target/inc directory
 * NOTE: This file that gives access to the generated configuration file
 */
#include "include/vitis_net_p4_0_defs.h"
#include "include/vitisnetp4_common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

/****************************************************************************************************************************************************/
/* SECTION: Constants/macros */
/****************************************************************************************************************************************************/

#define EXAMPLE_NUM_TABLE_ENTRIES (4)

#define DISPLAY_ERROR(ErrorCode)  printf("Error Code is value %s\n", XilVitisNetP4ReturnTypeToString(ErrorCode))

#define CONVERT_BITS_TO_BYTES(NumBits) ((NumBits/XIL_VITIS_NET_P4_BITS_PER_BYTE) + ((NumBits % XIL_VITIS_NET_P4_BITS_PER_BYTE)? 1 : 0))

/****************************************************************************************************************************************************/
/* SECTION: Local function declarations*/
/****************************************************************************************************************************************************/
static void DisplayVitisNetP4Versions(XilVitisNetP4TargetCtx *CtxPtr);

XilVitisNetP4ReturnType XilVitisNetP4WordLogStub(XilVitisNetP4EnvIf *EnvIfPtr, const char *MessagePtr);

XilVitisNetP4ReturnType example_log_info(XilVitisNetP4EnvIf *EnvIfPtr, const char *MessagePtr);


int device_open(char *file_name);

int device_close();

void device_write(uint32_t address, uint32_t data);

uint32_t device_read(uint32_t address, uint32_t *data);

XilVitisNetP4ReturnType env_write(XilVitisNetP4EnvIf *EnvIfPtr, XilVitisNetP4AddressType Address, uint32_t WriteValue);

XilVitisNetP4ReturnType env_read(XilVitisNetP4EnvIf *EnvIfPtr, XilVitisNetP4AddressType Address, uint32_t *ReadValuePtr);






/****************************************************************************************************************************************************/
/* SECTION: Global variables */
/****************************************************************************************************************************************************/

char sysfile_path[] = "/sys/devices/pci0000:3a/0000:3a:00.0/0000:3b:00.0/resource2";

typedef struct ExampleUserContext
{
    XilVitisNetP4AddressType VitisNetP4Address;
} ExampleUserContext;

/* Key and Responses based on the five Tuple, using Big Endian array */

uint8_t ForwardKeyArray[EXAMPLE_NUM_TABLE_ENTRIES][4] = {
    // Entry 1 : ForwardPkt 
    // key :[ ipv4.dst=9aaa2010 ] 
    {0x9a, 0xaa, 0x20, 0x10},
    //Entry 2 : ForwardPkt
    // key :[ ipv4.dst=cc930a03 ] 
    {0xcc, 0x93, 0x0a, 0x03},
    // Entry 3 : ForwardPkt
    // key :[ ipv4.dst=6353a5ca ] 
    {0x63, 0x53, 0xa5, 0xca},
    // Entry 4 : ForwardPkt
    // key :[ ipv4.dst=cc3d03d7 ]
    {0xcc, 0x3d, 0x03, 0xd7}
};

uint8_t ForwardMasksArray[EXAMPLE_NUM_TABLE_ENTRIES][4] = {
    // Entry 1 : ForwardPkt
    // key :[ ipv4.dst=9aaa2010 ] 
    {0xff, 0xff, 0xff, 0xff},
    //Entry 2 : ForwardPkt
    // key :[ ipv4.dst=cc930a03 ] 
    {0xff, 0xff, 0xff, 0xff},
    // Entry 3 : ForwardPkt
    // key :[ ipv4.dst=6353a5ca ] 
    {0xff, 0xff, 0xff, 0xff},
    // Entry 4 : ForwardPkt
    // key :[ ipv4.dst=cc3d03d7 ]
    {0xff, 0xff, 0xff, 0xff}
};

/*
 * The corresponding Action Parameters used to create the loop
 * Note: The Action Parameters are concatenated with the Action Id to construct the table response of the Match-Action unit
 */
uint8_t ForwardActionParamsArray[EXAMPLE_NUM_TABLE_ENTRIES][1] = {
    // Entry 1 : ForwardPkt
    // response :[0x0]
    {0x0},
    // Entry 2 : ForwardPkt
    // response :[0x0]
    {0x0},
    // Entry 3 : ForwardPkt
    //response :[0x0]
    {0x0},
    // Entry 4 : ForwardPkt
    // response :[0x0]
    {0x0},
};


// sysfile path to open-nic-shell PCIe device
int sysfile;


/****************************************************************************************************************************************************/
/* SECTION: Entry point */
/****************************************************************************************************************************************************/


int main(void)
{
    XilVitisNetP4EnvIf EnvIf;
    XilVitisNetP4TargetCtx ForwardTargetCtx;
    XilVitisNetP4ReturnType Result;
    uint32_t Index;
    uint32_t ActionId;
    uint8_t ReadParamActionsBuffer[2];
    uint32_t ReadActionId;
    uint32_t ReadPriority;
    uint8_t Masks;

    ExampleUserContext *UserCtxPtr;
    XilVitisNetP4TableCtx *ForwardTableCtxPtr;

    XilVitisNetP4EnvIf *EnvIfPtr = &EnvIf;
    XilVitisNetP4TargetCtx *ForwardTargetCtxPtr = &ForwardTargetCtx;

    UserCtxPtr = calloc(1, sizeof(ExampleUserContext));
    if (UserCtxPtr == NULL)
    {
       printf("ERROR: Failed to allocate memory\n\r");
       return -1;
    }
    UserCtxPtr->VitisNetP4Address = 0x100000;

    /*
     * Setting up function pointers to the Read and Write function, etc.
     */
    (EnvIfPtr)->WordWrite32 = env_write;
    (EnvIfPtr)->WordRead32 = env_read;
    (EnvIfPtr)->LogError = example_log_info;
    (EnvIfPtr)->LogInfo = example_log_info;
    (EnvIfPtr)->UserCtx = (XilVitisNetP4UserCtxType)UserCtxPtr;

    printf("Opening pcimem device \n\r");
    device_open(sysfile_path);
    sleep(1);
    uint32_t readData;

    // writes to enable the CMAC port 0
    printf("Enabling CMAC port 0: \n\r");
    device_write(0x8014, 0x1);
    device_write(0x800c, 0x1);
    // read the CMAC status
    printf("Checking CMAC port 0 link status: \n\r");
    device_read(0x8204, &readData);
    device_read(0x8204, &readData);
    sleep(1);

    printf("Initialize the Target Driver\n\r");
    Result = XilVitisNetP4TargetInit(ForwardTargetCtxPtr, EnvIfPtr, &XilVitisNetP4TargetConfig_vitis_net_p4_0);
    printf("Finish Initialize!\n\r");
    if (Result == XIL_VITIS_NET_P4_TARGET_ERR_INCOMPATIBLE_SW_HW)
    {
        printf("Found IP and SW version differences:\n\r");
        DisplayVitisNetP4Versions(ForwardTargetCtxPtr);
        goto exit_example;
    }
    else if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto exit_example;
    }

    printf("Get Table Handle\n\r");
    Result = XilVitisNetP4TargetGetTableByName(ForwardTargetCtxPtr, "forwardIPv4", &ForwardTableCtxPtr);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }

    printf("Get ActionId\n\r");
    Result = XilVitisNetP4TableGetActionId(ForwardTableCtxPtr, "dropPacket", &ActionId);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }

    XilVitisNetP4TableMode mode;
    Result = XilVitisNetP4TableGetMode(ForwardTableCtxPtr, &mode);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }
    printf("Table mode: %d\n\r", mode);

    printf("\nInsert Tables.....");
    for (Index = 0; Index < EXAMPLE_NUM_TABLE_ENTRIES; Index++)
    // Insert Table 
    {
        printf("Insert table entry %d\n\r", Index);

        Result = XilVitisNetP4TableInsert(ForwardTableCtxPtr,
                                     ForwardKeyArray[Index],
                                     ForwardMasksArray[Index], 
                                     0x0, 
                                     ActionId,
                                     ForwardActionParamsArray[Index]);
        if (Result != XIL_VITIS_NET_P4_SUCCESS)
        {
            DISPLAY_ERROR(Result);
            goto target_exit;
        }
        //sleep(1);
    }
//
    printf("\nTable Querying... \n\r");
    for (Index = 0; Index < EXAMPLE_NUM_TABLE_ENTRIES; Index++)
    {   
        Result = XilVitisNetP4TableGetByKey(ForwardTableCtxPtr,
                                       ForwardKeyArray[Index],
                                       ForwardMasksArray[Index], 
                                       &ReadPriority, 
                                       &ReadActionId,
                                       ReadParamActionsBuffer);

        if (Result == XIL_VITIS_NET_P4_SUCCESS)
        {
            printf("For table entry %d the Action Parameters are 0x%02X and Action Id is %d\n\r",
                   Index,
                   ReadParamActionsBuffer[0],
                   ReadActionId);
        }
        else
        {
            DISPLAY_ERROR(Result);
            goto target_exit;
        }
    }
        //sleep(1);
    
    printf("\n Updating Tables...\n\r");
    for (Index = 0; Index < EXAMPLE_NUM_TABLE_ENTRIES; Index++){
        printf("Updating the Response for table entry %d\n\r", Index);
        Result = XilVitisNetP4TableUpdate(ForwardTableCtxPtr,
                                     ForwardKeyArray[Index],
                                     ForwardMasksArray[Index], 
                                     ActionId,
                                     ReadParamActionsBuffer);

        if (Result != XIL_VITIS_NET_P4_SUCCESS)
        {
            DISPLAY_ERROR(Result);
            goto target_exit;
        }
    }

    printf("\n Deleting Tables....\n\r");
    for (Index = 0; Index < EXAMPLE_NUM_TABLE_ENTRIES-2; Index++)
    {
        printf("Delete table entry %d\n\r", Index);
        printf("The masks is %2x", ForwardMasksArray[Index][1]);
        Result = XilVitisNetP4TableDelete(ForwardTableCtxPtr, ForwardKeyArray[Index], ForwardMasksArray[Index]);

        if (Result == XIL_VITIS_NET_P4_SUCCESS)
        {
            // Not neccessary but checking if the key can be found to demo the usage //
            Result = XilVitisNetP4TableGetByKey(ForwardTableCtxPtr,
                                           ForwardKeyArray[Index],
                                           ForwardMasksArray[Index],
                                           &ReadPriority, 
                                           &ReadActionId,
                                           ReadParamActionsBuffer);
            if (Result != XIL_VITIS_NET_P4_CAM_ERR_KEY_NOT_FOUND)
            {
                printf("Error table entry %d is present\n\r", Index);
            }
            else
            {
                printf("\nTable entry %d successfully deleted\n\r", Index);
            }
        }
        else
        {
            DISPLAY_ERROR(Result);
            goto target_exit;
        }
    }
//  */

target_exit:
    printf ("target_exit: \n");
    printf("Closing pcimem device\n");
    device_close();
    Result = XilVitisNetP4TargetExit(ForwardTargetCtxPtr);

    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
    }

exit_example:
    printf ("exit_example:\n");
    free(EnvIfPtr->UserCtx);
    return Result;

}


/****************************************************************************************************************************************************/
/* SECTION: Local function definitions */
/****************************************************************************************************************************************************/
static void DisplayVitisNetP4Versions(XilVitisNetP4TargetCtx *CtxPtr)
{
    XilVitisNetP4ReturnType Result;
    XilVitisNetP4Version SwVersion;
    XilVitisNetP4Version IpVersion;
    XilVitisNetP4TargetBuildInfoCtx *BuildInfoCtxPtr;

    Result =  XilVitisNetP4TargetGetSwVersion(CtxPtr, &SwVersion);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        return;
    }

    /* The BuildInfo Driver provides access to the IP Version if present */
    Result = XilVitisNetP4TargetGetBuildInfoDrv(CtxPtr, &BuildInfoCtxPtr);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        return;
    }

    Result = XilVitisNetP4TargetBuildInfoGetIpVersion(BuildInfoCtxPtr, &IpVersion);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        return;
    }

    printf("----VitisNetP4Runtime Software Version\n");
    printf("\t\t Major = %d\n", SwVersion.Major);
    printf("\t\t Minor = %d\n", SwVersion.Minor);
    printf("\n");

    printf("----VitisNetP4IP Version\n");
    printf("\t\t Major = %d\n", IpVersion.Major);
    printf("\t\t Minor = %d\n", IpVersion.Minor);
}


XilVitisNetP4ReturnType XilVitisNetP4WordLogStub(XilVitisNetP4EnvIf *EnvIfPtr, const char *MessagePtr)
{
    if (EnvIfPtr == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_NULL_PARAM;
    }

    if (MessagePtr == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_NULL_PARAM;
    }

    return XIL_VITIS_NET_P4_SUCCESS;
}

XilVitisNetP4ReturnType example_log_info(XilVitisNetP4EnvIf *EnvIfPtr, const char *MessagePtr)
{
    if (EnvIfPtr == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_NULL_PARAM;
    }

    if (MessagePtr == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_NULL_PARAM;
    }

    printf(MessagePtr);

    return XIL_VITIS_NET_P4_SUCCESS;
}


inline int device_open(char *sys_file)
{
    if ((sysfile = open(sys_file, O_RDWR | O_SYNC)) < 0)
    {
        fprintf(stderr, "Error openning sysfile: %s\n", strerror(errno));
        return -1;
    } else
        return 0;
}

inline int device_close()
{
    return close(sysfile);
}


XilVitisNetP4ReturnType env_write(XilVitisNetP4EnvIf *EnvIfPtr, XilVitisNetP4AddressType Address, uint32_t WriteValue) {
    ExampleUserContext *UserCtxPtr;
    //printf("Writing: ");
    if (EnvIfPtr == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_NULL_PARAM;
    }
    else if (EnvIfPtr->UserCtx == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_INTERNAL_ASSERTION;
    }
    UserCtxPtr = (ExampleUserContext *)EnvIfPtr->UserCtx;

    void *region;
    void *virtual;
    off_t addr = (off_t)UserCtxPtr->VitisNetP4Address + Address;
    off_t offset = addr & (-4096);
    off_t rem = addr & 0xFFF;
    size_t length = 4096;

    //printf("Writing:");
    region = mmap(0, length,  PROT_WRITE, MAP_SHARED, sysfile, offset);
    if (region == MAP_FAILED)
    {
        fprintf(stderr, "Error calling mmap: mapping failed\n");
        exit(-1);
    }
    virtual = region + rem;
    *((uint32_t *)virtual) = WriteValue;
    
    if(munmap(region, length) < 0)
    {
        fprintf(stderr, "Error calling munmap: %s\n", strerror(errno));
        exit(-1);
    }

    #ifdef _DEBUG
    printf("Wrote 0x%0*X to 0x%lX;\n", 8, WriteValue, addr);
    #endif

    return XIL_VITIS_NET_P4_SUCCESS;
}

XilVitisNetP4ReturnType env_read(XilVitisNetP4EnvIf *EnvIfPtr, XilVitisNetP4AddressType Address, uint32_t *ReadValuePtr) {
    ExampleUserContext *UserCtxPtr;
    if (EnvIfPtr == NULL || ReadValuePtr == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_NULL_PARAM;
    }
    else if (EnvIfPtr->UserCtx == NULL)
    {
        return XIL_VITIS_NET_P4_GENERAL_ERR_INTERNAL_ASSERTION;
    }

    UserCtxPtr = (ExampleUserContext *) EnvIfPtr->UserCtx;

    void *region;
    void *virtual;
    off_t addr = (off_t)UserCtxPtr->VitisNetP4Address + Address;
    off_t offset = addr & (-4096);
    off_t rem = addr & 0xFFF;
    size_t length = 4096;

    //printf("Reading:");
    region = mmap(0, length, PROT_READ , MAP_SHARED, sysfile, offset);
    if (region == MAP_FAILED)
    {
        fprintf(stderr, "Error calling mmap: mapping failed\n");
        exit(-1);
    }
    virtual = region + rem;
    *ReadValuePtr = *((uint32_t *)virtual);

    if(munmap(region, length) < 0)
    {
        fprintf(stderr, "Error calling munmap: %s\n", strerror(errno));
        exit(-1);
    }
    //sleep(1);

    #ifdef _DEBUG
    printf("Read value at offset (%zu): 0x%0*X\n", addr, 8, *ReadValuePtr);
    #endif

    return XIL_VITIS_NET_P4_SUCCESS;
}


void device_write(uint32_t address, uint32_t data) {
    void *region;
    void *virtual;
    off_t addr = (off_t)address;
    off_t offset = addr & (-4096);
    off_t rem = addr & 0xFFF;
    size_t length = 4096;

    //printf("Writing:");
    region = mmap(0, length, PROT_WRITE, MAP_SHARED, sysfile, offset);
    if (region == MAP_FAILED)
    {
        fprintf(stderr, "Error calling mmap: mapping failed\n");
        exit(-1);
    }
    virtual = region + rem;
    *((uint32_t *)virtual) = data;

    if(munmap(region, length) < 0)
    {
        fprintf(stderr, "Error calling munmap: %s\n", strerror(errno));
        exit(-1);
    }
    #ifdef _DEBUG
    printf("Wrote 0x%0*X to 0x%X;\n", 8, data, address);
    #endif
}

uint32_t device_read(uint32_t address, uint32_t *data) {
    void *region;
    void *virtual;
    off_t addr = (off_t)address;
    off_t offset = addr & (-4096);
    off_t rem = addr & 0xFFF;
    size_t length = 4096;

    //printf("Reading:");
    region = mmap(0, length, PROT_READ , MAP_SHARED, sysfile, offset);
    if (region == MAP_FAILED)
    {
        fprintf(stderr, "Error calling mmap: mapping failed\n");
        exit(-1);
    }
    virtual = region + rem;
    *data = *((uint32_t *)virtual);

    if(munmap(region, length) < 0)
    {
        fprintf(stderr, "Error calling munmap: %s\n", strerror(errno));
        exit(-1);
    }
    #ifdef _DEBUG
    printf("Read value at offset (%u): 0x%0*X\n", address, 8, *data);
    #endif
    //sleep(1);
    return 0;
}
