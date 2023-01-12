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

// sysfile path to open-nic-shell PCIe device
int sysfile;

/****************************************************************************************************************************************************/
/* SECTION: Global variables */
/****************************************************************************************************************************************************/

char sysfile_path[] = "/sys/devices/pci0000:3a/0000:3a:00.0/0000:3b:00.0/resource2";

typedef struct ExampleUserContext
{
    XilVitisNetP4AddressType VitisNetP4Address;
} ExampleUserContext;

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
