/****************************************************************************************************************************************************/
/* SECTION: Header includes */
/****************************************************************************************************************************************************/

/*
 * The example designs include file should be present in the target/inc directory
 * NOTE: This file that gives access to the generated configuration file
 */
#include "include/vitis_net_p4_0_defs.h"
#include "include/vitisnetp4_common.h"
#include "p4locals.h"
/****************************************************************************************************************************************************/
/* SECTION: Constants/macros */
/****************************************************************************************************************************************************/

#define EXAMPLE_NUM_TABLE_ENTRIES (8)

#define DISPLAY_ERROR(ErrorCode)  printf("Error Code is value %s", XilVitisNetP4ReturnTypeToString(ErrorCode))

#define CONVERT_BITS_TO_BYTES(NumBits) ((NumBits/XIL_VITIS_NET_P4_BITS_PER_BYTE) + ((NumBits % XIL_VITIS_NET_P4_BITS_PER_BYTE)? 1 : 0))

/* Key and Responses based on the five Tuple, using Big Endian array */

uint8_t CalcKeyArray[EXAMPLE_NUM_TABLE_ENTRIES][1] = {
    {0x6},
    {0x1},
    {0x5},
    {0x8},
    {0xf},
    {0x0},
	{0x4},
	{0x3}
};

//uint8_t CalcKeyArray[EXAMPLE_NUM_TABLE_ENTRIES][1] = {
//	{0x0}
//};

/*
 * The corresponding Action Parameters used to create the loop
 * Note: The Action Parameters are concatenated with the Action Id to construct the table response of the Match-Action unit
 */
uint32_t CalcActionId[EXAMPLE_NUM_TABLE_ENTRIES] = {0,1,2,3,4,5,6,7};
//uint32_t CalcActionId[EXAMPLE_NUM_TABLE_ENTRIES] = {6};

uint8_t CalcActionParaArray[1] = { 0 };





/****************************************************************************************************************************************************/
/* SECTION: Entry point */
/****************************************************************************************************************************************************/


int main(void)
{
    XilVitisNetP4EnvIf EnvIf;
    XilVitisNetP4TargetCtx CalcTargetCtx;
    XilVitisNetP4ReturnType Result;
    uint32_t Index;
    uint32_t ActionId;
    uint8_t ReadParamActionsBuffer[2];
    uint32_t ReadActionId;
    uint32_t ReadPriority;
    uint8_t Masks;

    ExampleUserContext *UserCtxPtr;
    XilVitisNetP4TableCtx *CalcTableCtxPtr;

    XilVitisNetP4EnvIf *EnvIfPtr = &EnvIf;
    XilVitisNetP4TargetCtx *CalcTargetCtxPtr = &CalcTargetCtx;

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
    Result = XilVitisNetP4TargetInit(CalcTargetCtxPtr, EnvIfPtr, &XilVitisNetP4TargetConfig_vitis_net_p4_0);
    printf("Finish Initialize!\n\r");
    if (Result == XIL_VITIS_NET_P4_TARGET_ERR_INCOMPATIBLE_SW_HW)
    {
        printf("Found IP and SW version differences:\n\r");
        DisplayVitisNetP4Versions(CalcTargetCtxPtr);
        goto exit_example;
    }
    else if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto exit_example;
    }

    printf("Get Table Handle\n\r");
    Result = XilVitisNetP4TargetGetTableByName(CalcTargetCtxPtr, "calculate", &CalcTableCtxPtr);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }
    uint32_t numActions =0;
    printf("Get Number of Actions\n\r");
    Result = XilVitisNetP4TableGetNumActions(CalcTableCtxPtr,&numActions);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }
    printf("Number of Actions %d\n\r", numActions);

    printf("Get ActionId\n\r");
    Result = XilVitisNetP4TableGetActionId(CalcTableCtxPtr, "operation_drop", &ActionId);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }
    printf("Action ID for operation_drop is %d\n\r", ActionId);
	for (int i =0; i<numActions; i++){
	    char ActionName[100] = "\0";
	    uint32_t actionNumberSize = 1000;
	    Result = XilVitisNetP4TableGetActionNameById(CalcTableCtxPtr, (uint32_t)(i), ActionName, actionNumberSize);
	    if (Result != XIL_VITIS_NET_P4_SUCCESS)
	    {
	        DISPLAY_ERROR(Result);
	        goto target_exit;
	    }
	    printf("The name for ActionID = %d is %s\n\r", i,ActionName);
	}


    XilVitisNetP4TableMode mode;
    Result = XilVitisNetP4TableGetMode(CalcTableCtxPtr, &mode);
    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
        goto target_exit;
    }
    printf("Table mode: %d\n\r", mode);
    
    printf("Resetting the table entries....\n\r");

    Result = XilVitisNetP4TableReset(CalcTableCtxPtr);

    for (Index = 0; Index < EXAMPLE_NUM_TABLE_ENTRIES; Index++)
    // Insert Table 
    {
        printf("Insert table entry %d\n\r", Index);

        Result = XilVitisNetP4TableInsert(CalcTableCtxPtr,
                                     CalcKeyArray[Index],
                                     NULL, // MaskPtr not used for a table with mode of DCAM
                                     0x0, // Priority is ignored for a table with mode of DCAM
                                     CalcActionId[Index],
                                     CalcActionParaArray);
        if (Result != XIL_VITIS_NET_P4_SUCCESS)
        {
            DISPLAY_ERROR(Result);
            goto target_exit;
        }
        printf("Finish Inseart!\n\r");
    }

target_exit:
    printf ("target_exit: \n");
    printf("Closing pcimem device\n");
    device_close();
    Result = XilVitisNetP4TargetExit(CalcTargetCtxPtr);

    if (Result != XIL_VITIS_NET_P4_SUCCESS)
    {
        DISPLAY_ERROR(Result);
    }

exit_example:
    printf ("exit_example:\n");
    free(EnvIfPtr->UserCtx);
    return Result;

}
