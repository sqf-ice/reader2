/*
*         Copyright (c), NXP Semiconductors Bangalore / India
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Example Source for NfcrdlibEx6_EMVCo_Loopback.
* This application will configure Reader Library as per Emvco specification and start Emvco polling.
* This loop back application will send SELECT_PPSE command and is used to test Emvco2.3.1a(L1)
* digital compliance.
* Please refer Readme.txt file  for  Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
* $Author: jenkins_ cm (nxp92197) $
* $Revision: 4216 $ (NFCRDLIB_V4.010.02.001606 : 4184 $ (NFCRDLIB_V4.010.01.001603 : 4078 ))
* $Date: 2016-02-10 19:49:10 +0530 (Wed, 10 Feb 2016) $
*
* History:
* PN: Generated 14. May 2015
* BK: Generated 12. Jun 2014
* PC: Generated 25. Nov 2012
*
*/

/**
 * Header for hardware configuration: bus interface, reset of attached reader ID, on-board LED handling etc.
 * */
#include <phhwConfig.h>

/**
* Reader Library Headers
*/
#include <ph_Status.h>

/*OSAL Headers*/
#include <phPlatform.h>

/*BAL Headers*/
#include <phbalReg.h>

/*PAL Headers*/
#include <phpalI14443p3a.h>
#include <phpalI14443p4.h>
#include <phpalI14443p3b.h>
#include <phpalI14443p4a.h>


#include <phacDiscLoop.h>
#include <sys/time.h>

#define PRETTY_PRINTING                                         /**< Enable pretty printing */
#define MiN_VALID_DATA_SIZE                     6
#define PHAC_EMVCO_MAX_BUFFSIZE               600               /**< Maximum buffer size for Emvco. */
//#define RUN_TEST_SUIT

typedef enum{
    eEmdRes_EOT = 0x70,
    eEmdRes_SW_0 = 0x90,
    eEmdRes_SW_1 = 0x00,
}eEmvcoRespByte;

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Stub_DataParams_t         sBalReader;                 /**< BAL component holder */

/*HAL variables*/
phhalHw_Nfc_Ic_DataParams_t        sHal_Nfc_Ic;                /* HAL component holder for Nfc Ic's */
uint8_t                            bHalBufferTx[PHAC_EMVCO_MAX_BUFFSIZE];          /* HAL TX buffer. Size 256 - Based on maximum FSL */
uint8_t                            bHalBufferRx[PHAC_EMVCO_MAX_BUFFSIZE];          /* HAL RX buffer. Size 256 - Based on maximum FSL */
void                              *pHal;                       /* HAL pointer */

/*PAL variables*/
phpalI14443p3a_Sw_DataParams_t     spalI14443p3a;              /* PAL I14443-A component */
phpalI14443p4a_Sw_DataParams_t     spalI14443p4a;              /* PAL I14443-4A component */
phpalI14443p3b_Sw_DataParams_t     spalI14443p3b;              /* PAL I14443-B component */
phpalI14443p4_Sw_DataParams_t      spalI14443p4;               /* PAL I14443-4 component */

/*DiscLoop variables*/
phacDiscLoop_Sw_DataParams_t       sDiscLoop;                  /* Discovery loop component */


#ifdef RUN_TEST_SUIT
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#else
uint8_t PPSE_response_buffer[256];
uint8_t PPSE_respsize;
/*Define Macro for the PPSE command with different P2 value in PPSE command*/
//#define PPSE_FCI
//#define PPSE_FMD
//#define PPSE_FCP
//#define PPSE_NO_LE
#define PPSE_TICKET

/* CLA = 0x00
 *  INS = 0xA4
 *  P1 = 0x04
 *  P2 = 0x00
 *  LC = 0x0E,
 *  DF = 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31
 *  LE = 0x00
 */
/*P2 parameter values of
 *  000000xxb = Return FCI template, optional use of FCI tag and length ,
 *  000001xxb = Return FCP template, mandatory use of FCP tag and length,
 *  000010xxb = Return FMD template, mandatory use of FMD tag and length,
 *  000011xxb = No response data if Le field absent, or proprietary if Le field present
 * */

#ifdef PPSE_TICKET
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x00, 0x05, 0xF2, 0x22, 0x22, 0x22,
        0x22 };



uint8_t partMessage[255] = { 0x00, 0xDA };
uint8_t partMessageEnd[255] = { 0x00, 0xDB };


uint8_t message_to_send[32000] = { 0x00, 0xDA, 0x04, 0x00, 0x05, 0xF2, 0x22, 0x22, 0x22,
        0x22 };
uint16_t length_message = 1000;


#endif

#ifdef PPSE_FCI
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#endif
#ifdef PPSE_FCP
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x04, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#endif
#ifdef  PPSE_FMD
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x08, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#endif
#ifdef PPSE_NO_LE
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x0C, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31 };
#endif
#endif
uint8_t command_buffer[PHAC_EMVCO_MAX_BUFFSIZE];
uint8_t *response_buffer;


static uint8_t aData[50]; /* ATS Response Holder */

#ifdef   DEBUG
static void PRINT_BUFF(uint8_t *hex, uint16_t num)
{
    uint32_t    i;

    for(i = 0; i < num; i++)
    {
        DEBUG_PRINTF(" %02X",hex[i]);
    }
}
#else
#define  PRINT_BUFF(x, y)
#endif /* DEBUG */

/*******************************************************************************
**   Function Declaration
*******************************************************************************/

/**
* \brief Initialize Reader Library
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t NfcRdLibInit(void);

/**
* \brief Provide Delay in micro second
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t DelayInMicroSec(
        uint32_t delayInMicroS)
{
    phStatus_t status = PH_ERR_SUCCESS;
    status = phhalHw_Wait(sDiscLoop.pHalDataParams,PHHAL_HW_TIME_MICROSECONDS, delayInMicroS);
    CHECK_STATUS(status);

    return status;
}

/**
* \brief Perform RF Reset as per Emvco Specification
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t EmvcoRfReset(void)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /*RF Field OFF*/
    status = phhalHw_FieldOff(sDiscLoop.pHalDataParams);
    CHECK_STATUS(status);

    status = DelayInMicroSec(5100);
    CHECK_STATUS(status);
    /*RF Field ON*/
    status = phhalHw_FieldOn(sDiscLoop.pHalDataParams);
    CHECK_STATUS(status);

    return status;
}

/**
* \brief Exchange Data APDU Packets for EMVCO (ISO14443-4 Exchange)
* This function will Exchange APDU data packets provided by Loop-Back Application
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t EmvcoDataExchange(uint8_t * com_buffer, uint16_t cmdsize, uint8_t ** resp_buffer, uint32_t * wRxLength)
{
    phStatus_t status;
    uint8_t *ppRxBuffer;
    uint16_t wRxLen = 0;

    status = phpalI14443p4_Exchange(&spalI14443p4, PH_EXCHANGE_DEFAULT, com_buffer, cmdsize, &ppRxBuffer, &wRxLen);
    if (PH_ERR_SUCCESS == status)
    {
        /* set the pointer to the start of the R-APDU */
        *resp_buffer = &ppRxBuffer[0];
    }
    else
    {
        /* Exchange not successful, reset the number of rxd bytes */
        wRxLen = 0x00;
    }

    *wRxLength = wRxLen;

    return status;
}



static phStatus_t SendTicket()
{   
    struct timeval t1,t2;
    double elapsedTime;
    uint32_t respsize;
    phStatus_t status;  
    uint16_t lengthPart = 250; // Mensajes más largos de un byte no es capaz de enviarlo
    uint8_t cmdPartSize = lengthPart - 2; // Enviamos de 255 en 255
    
    gettimeofday(&t1, NULL);
    for (int i= 0;i<length_message; i+=cmdPartSize)
    {
        DEBUG_PRINTF("Sending %d\n", i);
        uint8_t *message;
        // Si no me caben todos los bytes en este primera remesa, lo voy partiendo
        if (i + cmdPartSize < length_message)
        {
            message = partMessage;
        }
        else
        {
            message = partMessageEnd;
            lengthPart = 2 + length_message - i;
        }
        
        
        memcpy(message+2, message_to_send+i, cmdPartSize);
        int nTries = 0;
        do
        {
            status = EmvcoDataExchange(message, lengthPart, &response_buffer, &respsize);
            CHECK_STATUS(status);

            if (response_buffer[1] != message[1])
            {
                DEBUG_PRINTF("Respuesta incorrecta %02x\n", message[1]);
                status = 0xFF; // invalid status
            }
            else 
            {
                DEBUG_PRINTF("Respuesta correcta %02x\n", message[1]);
            }
            nTries++;
        } while (status != PH_ERR_SUCCESS && nTries <4);
        
        
        partMessage[1]++;

    }
        
    gettimeofday(&t2, NULL);
    
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    
    DEBUG_PRINTF("\nAfter write %f\n", elapsedTime);  
    return status;
}

/**
* \brief EMVCo Loop-Back function
* This Loop-Back function converts each received R-APDU into the next C-APDU (by stripping the
* status words), and sends this C-APDU back to the card simulator.
* Also this function send SELECT_PPSE command after card activation.
* Loop-Back Function exist when EOT (End Of Test) Command is received from the card simulator.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t EmvcoLoopBack(phacDiscLoop_Sw_DataParams_t * pDataParams)
{

    uint32_t cmdsize, respsize;
    phStatus_t status = 0xFF;
    uint8_t bEndOfLoopBack = 0;
    uint8_t bRemovalProcedure;
    cmdsize = sizeof(PPSE_SELECT_APDU);

    while (status != PH_ERR_SUCCESS)
    {
        status = EmvcoDataExchange(PPSE_SELECT_APDU, cmdsize, &response_buffer, &respsize);

    #ifndef RUN_TEST_SUIT

        /*Check if P1 is 0x04 which means that the data field consists of DF name */
        if(PPSE_SELECT_APDU[2] == 0x04)
        {
            DEBUG_PRINTF("\n DF Name: \n");
            /* DF Size = Total Command size - size of(PDU Header + Expected Len(Le))*/
            PRINT_BUFF(&PPSE_SELECT_APDU[5], PPSE_SELECT_APDU[4]);
        }
        if (respsize > 0)
        {
            memcpy(&PPSE_response_buffer[0],response_buffer,respsize);
            DEBUG_PRINTF("\n SELECT_PPSE Res:\n");
            /* Status word removed */
            PRINT_BUFF(PPSE_response_buffer, (respsize - 2));
            DEBUG_PRINTF("\nTransaction Done Remove card\n");

            SendTicket();

        }
        else
        {
            DEBUG_PRINTF("\nFCI not recieved\n");
    #ifdef PPSE_NO_LE
            DEBUG_PRINTF("Transaction Done Remove card\n");
    #else
            DEBUG_PRINTF("Transaction Failed Replace the card\n");
    #endif
        }

    #endif
    }

#ifdef ENDOFLOOPBACK    
    while (!bEndOfLoopBack)
    {
        if (respsize > 0)
        {
            if (respsize >= MiN_VALID_DATA_SIZE)
            {
                /* EOT (End Of Test) Command. Exit the loop */
                if (eEmdRes_EOT == response_buffer[1])
                {
                    /* Second byte = 0x70, stop the loopback */
                    bEndOfLoopBack = 1;
                    bRemovalProcedure = PH_ON;
                }
                else if (eEmdRes_SW_0 == response_buffer[respsize - 2])
                {
                    /* Format the card response into a new command without the status word 0x90 0x00 */
                    cmdsize = respsize - 2;  /* To Remove two bytes of status word */
                    memcpy(command_buffer, response_buffer, cmdsize);

                    /* Send back(Command) : Received Response - Status_Word */
                    status = EmvcoDataExchange(command_buffer, cmdsize, &response_buffer, &respsize);
                }
                else
                {
                    /* error Abort Loopback */
                    bEndOfLoopBack = 1;
                }
            }
            else/*if (respsize <6)*/
            {
                /* re-send the select appli APDU */
                status = EmvcoDataExchange(PPSE_SELECT_APDU, cmdsize, &response_buffer, &respsize);
                if (respsize == 0)
                {
                    bEndOfLoopBack = 1;
                }
            }
        }/*if(respsize > 0)*/
        else
        {
            bEndOfLoopBack = 1;
        }
    }/*while (!bEndOfLoopBack)*/
#endif
    


    bRemovalProcedure = PH_ON;
    if(bRemovalProcedure == PH_ON)
    {
        /* Set Poll state to perform Tag removal procedure*/
        status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_REMOVAL);
        CHECK_STATUS(status);

        status = phacDiscLoop_Run(pDataParams, PHAC_DISCLOOP_ENTRY_POINT_POLL);
        bRemovalProcedure = PH_OFF;
    }
    return status;
}

/**
* This function will load/configure Discovery loop and 14443 PAL with default values to support Emvco2.3.1a(L1) digital compliance.
 * Application can read these values from EEPROM area and load/configure Discovery loop via SetConfig
* \param   bProfile      Reader Library Profile
* \note    Values used below are default and is for demonstration purpose.
*/
static phStatus_t LoadEmvcoSettings()
{
    phStatus_t status = PH_ERR_SUCCESS;

    sDiscLoop.pPal1443p3aDataParams  = &spalI14443p3a;
    sDiscLoop.pPal1443p3bDataParams  = &spalI14443p3b;
    sDiscLoop.pPal1443p4aDataParams  = &spalI14443p4a;
    sDiscLoop.pPal14443p4DataParams  = &spalI14443p4;
    sDiscLoop.pHalDataParams         = &sHal_Nfc_Ic.sHal;

    /* passive Bailout bitmap config. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_BAIL_OUT, 0x00);
    CHECK_STATUS(status);

    /* passive poll bitmap config. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, (PHAC_DISCLOOP_POS_BIT_MASK_A | PHAC_DISCLOOP_POS_BIT_MASK_B));
    CHECK_STATUS(status);

    /* Active Listen bitmap config. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_LIS_TECH_CFG, 0x00);
    CHECK_STATUS(status);

    /* Active Listen bitmap config. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_ACT_LIS_TECH_CFG, 0x00);
    CHECK_STATUS(status);

    /* Active Poll bitmap config. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_ACT_POLL_TECH_CFG, 0x00);
    CHECK_STATUS(status);

    /* Disable LPCD feature. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_ENABLE_LPCD, PH_OFF);
    CHECK_STATUS(status);

    /* reset collision Pending */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_COLLISION_PENDING, PH_OFF);
    CHECK_STATUS(status);

    /* whether anti-collision is supported or not. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_ANTI_COLL, PH_ON);
    CHECK_STATUS(status);

    /* Poll Mode default state*/
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
    CHECK_STATUS(status);

    /* Passive CON_DEVICE limit. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_DEVICE_LIMIT, 1);
    CHECK_STATUS(status);

    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DEVICE_LIMIT, 1);
    CHECK_STATUS(status);

    /* Passive polling Tx Guard times in micro seconds. */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_GTA_VALUE_US, 5100);
    CHECK_STATUS(status);

    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_GTB_VALUE_US, 5100);
    CHECK_STATUS(status);

    /* Configure FSDI for the 14443P4A tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_FSDI, 0x08);
    CHECK_STATUS(status);

    /* Configure CID for the 14443P4A tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_CID, 0x00);
    CHECK_STATUS(status);

    /* Configure DRI for the 14443P4A tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_DRI, 0x00);
    CHECK_STATUS(status);

    /* Configure DSI for the 14443P4A tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_DSI, 0x00);
    CHECK_STATUS(status);

    /* Configure AFI for the type B tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_AFI_REQ, 0x00);
    CHECK_STATUS(status);

    /* Configure FSDI for the type B tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_FSDI, 0x08);
    CHECK_STATUS(status);

    /* Configure CID for the type B tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_CID, 0x00);
    CHECK_STATUS(status);

    /* Configure DRI for the type B tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DRI, 0x00);
    CHECK_STATUS(status);

    /* Configure DSI for the type B tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DSI, 0x00);
    CHECK_STATUS(status);

    /* Configure Extended ATQB support for the type B tags */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_EXTATQB, 0x00);
    CHECK_STATUS(status);

    /* Configure reader library mode */
    status = phacDiscLoop_SetConfig(&sDiscLoop, PHAC_DISCLOOP_CONFIG_OPE_MODE, RD_LIB_MODE_EMVCO);
    CHECK_STATUS(status);

    return status;
}

phStatus_t NfcRdLibInit(void)
{
    phStatus_t  status;

    /* Initialize the Reader BAL (Bus Abstraction Layer) component */
    status = phbalReg_Stub_Init(&sBalReader, sizeof(phbalReg_Stub_DataParams_t));
    CHECK_STATUS(status);

    /* Initialize the OSAL Events. */
    status = phOsal_Event_Init();
    CHECK_STATUS(status);

	//Start interrupt thread
    Set_Interrupt();
	
    /* Set HAL type in BAL */
#ifdef NXPBUILD__PHHAL_HW_PN5180
    status = phbalReg_SetConfig(
        &sBalReader,
        PHBAL_REG_CONFIG_HAL_HW_TYPE,
        PHBAL_REG_HAL_HW_PN5180);
#endif
#ifdef NXPBUILD__PHHAL_HW_RC523
    status = phbalReg_SetConfig(
        &sBalReader,
        PHBAL_REG_CONFIG_HAL_HW_TYPE,
        PHBAL_REG_HAL_HW_RC523);
#endif
#ifdef NXPBUILD__PHHAL_HW_RC663
    status = phbalReg_SetConfig(
        &sBalReader,
        PHBAL_REG_CONFIG_HAL_HW_TYPE,
        PHBAL_REG_HAL_HW_RC663);
#endif
    CHECK_STATUS(status);
	
    status = phbalReg_SetPort(
        &sBalReader,
        SPI_CONFIG);
    CHECK_STATUS(status);

    /* Open BAL */
    status = phbalReg_OpenPort(&sBalReader);
    CHECK_STATUS(status);

    /* Initialize the Reader HAL (Hardware Abstraction Layer) component */
    status = phhalHw_Nfc_IC_Init(
                                &sHal_Nfc_Ic,
                                sizeof(phhalHw_Nfc_Ic_DataParams_t),
                                &sBalReader,
                                0,
                                bHalBufferTx,
                                sizeof(bHalBufferTx),
                                bHalBufferRx,
                                sizeof(bHalBufferRx)
                                );

    /* Set the parameter to use the SPI interface */
    sHal_Nfc_Ic.sHal.bBalConnectionType = PHHAL_HW_BAL_CONNECTION_SPI;;

	Configure_Device(&sHal_Nfc_Ic);

    /* Set the generic pointer */
    pHal = &sHal_Nfc_Ic.sHal;

    /* Initialize the I14443-A PAL layer */
    status = phpalI14443p3a_Sw_Init(&spalI14443p3a, sizeof(phpalI14443p3a_Sw_DataParams_t), &sHal_Nfc_Ic.sHal);
    CHECK_STATUS(status);

    /* Initialize the I14443-A PAL component */
    status = phpalI14443p4a_Sw_Init(&spalI14443p4a, sizeof(phpalI14443p4a_Sw_DataParams_t), &sHal_Nfc_Ic.sHal);
    CHECK_STATUS(status);

    /* Initialize the I14443-4 PAL component */
    status = phpalI14443p4_Sw_Init(&spalI14443p4, sizeof(phpalI14443p4_Sw_DataParams_t), &sHal_Nfc_Ic.sHal);
    CHECK_STATUS(status);

    /* Initialize the I14443-B PAL  component */
    status = phpalI14443p3b_Sw_Init(&spalI14443p3b, sizeof(spalI14443p3b), &sHal_Nfc_Ic.sHal);
    CHECK_STATUS(status);

    /* Initialize the discover component */
    status = phacDiscLoop_Sw_Init(&sDiscLoop, sizeof(phacDiscLoop_Sw_DataParams_t), &sHal_Nfc_Ic.sHal);
    CHECK_STATUS(status);

    /* Load Emvco Default setting */
    status = LoadEmvcoSettings();
    CHECK_STATUS(status);

    /* Assign ATS buffer for Type A */
    sDiscLoop.sTypeATargetInfo.sTypeA_I3P4.pAts                      = aData;

    return status;
}

void Emvco_Demo(void * pDiscLoopParams)
{

    phStatus_t  status;
    phStatus_t tmpStatus;
    uint16_t    wTagsDetected = 0;
    uint8_t     bCidEnabled;
    uint8_t     bCid;
    uint8_t     bNadSupported;
    uint8_t     bFwi;
    uint8_t     bFsdi;
    uint8_t     bFsci;

    phacDiscLoop_Sw_DataParams_t * pDataParams;

    pDataParams = &sDiscLoop;
    tmpStatus = PHAC_DISCLOOP_NO_TECH_DETECTED;

    Set_Interrupt();

    /* Initialize library */
    status = NfcRdLibInit();
    CHECK_STATUS(status);

    status = EmvcoRfReset();
    CHECK_STATUS(status);
    status = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_SET_EMD, PH_OFF);
    CHECK_STATUS(status);
    /* Set discovery loop poll state */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
    CHECK_STATUS(status);



    while(1)
    {
        if((tmpStatus & PH_ERR_MASK) != PHAC_DISCLOOP_NO_TECH_DETECTED)
        {
            /* Perform RF Reset */
            status = EmvcoRfReset();
            CHECK_STATUS(status);
        }
        /* Start Polling, Function will return once card is activated or any other error has occurred */
        tmpStatus = phacDiscLoop_Run(pDataParams, PHAC_DISCLOOP_ENTRY_POINT_POLL);

        if((tmpStatus & PH_ERR_MASK) == PHAC_DISCLOOP_DEVICE_ACTIVATED)
        {
            status = phacDiscLoop_GetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTagsDetected);
            CHECK_STATUS(status);

            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_A))
            {
                /* Retrieve 14443-4A protocol parameter */
                status = phpalI14443p4a_GetProtocolParams(
                                pDataParams->pPal1443p4aDataParams,
                                &bCidEnabled,
                                &bCid,
                                &bNadSupported,
                                &bFwi,
                                &bFsdi,
                                &bFsci);
                CHECK_STATUS(status);

                /* Set 14443-4 protocol parameter */
                status = phpalI14443p4_SetProtocol(
                                pDataParams->pPal14443p4DataParams,
                                PH_OFF,
                                bCid,
                                PH_OFF,
                                PH_OFF,
                                bFwi,
                                bFsdi,
                                bFsci);

                CHECK_STATUS(status);
            }

            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_B))
            {
                /* Retrieve 14443-3b protocol parameter */
                status = phpalI14443p3b_GetProtocolParams(
                                pDataParams->pPal1443p3bDataParams,
                                &bCidEnabled,
                                &bCid,
                                &bNadSupported,
                                &bFwi,
                                &bFsdi,
                                &bFsci);
                CHECK_STATUS(status);

                /* Set 14443-4 protocol parameter */
                status = phpalI14443p4_SetProtocol(
                                pDataParams->pPal14443p4DataParams,
                                PH_OFF,
                                bCid,
                                PH_OFF,
                                PH_OFF,
                                bFwi,
                                bFsdi,
                                bFsci);

                CHECK_STATUS(status);
            }
            /* Send SELECT_PPSE command and start exchange I-PDU */
            status = EmvcoLoopBack(pDataParams);

        }
        else if (((tmpStatus & PH_ERR_MASK) == PHAC_DISCLOOP_NO_TECH_DETECTED) ||
            ((tmpStatus & PH_ERR_MASK) == PHAC_DISCLOOP_NO_DEVICE_RESOLVED))
        {
           // wEntryPoint = PHAC_DISCLOOP_ENTRY_POINT_POLL;
            status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
        }        
        else
        {
            printf("algÚn error\n");
            status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
            CHECK_STATUS(status);
        }
    }
}
/*******************************************************************************
**   Main Function
*******************************************************************************/

int main (void)
{
	int ret = 0;

    /* Set the interface link for the internal chip communication */
    ret = Set_Interface_Link();
    if(ret)
    {
    	return 1;
    }

    /* Perform a hardware reset */
    Reset_reader_device();

    DEBUG_PRINTF("\n Emvco Example: ");

    Emvco_Demo(NULL);

	return 0;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
