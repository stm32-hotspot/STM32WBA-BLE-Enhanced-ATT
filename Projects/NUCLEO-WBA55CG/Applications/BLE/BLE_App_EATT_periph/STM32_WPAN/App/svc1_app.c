/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service1_app.c
  * @author  MCD Application Team
  * @brief   service1_app application definition.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "log_module.h"
#include "app_ble.h"
#include "ll_sys_if.h"
#include "dbg_trace.h"
#include "ble.h"
#include "svc1_app.h"
#include "svc1.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32_timer.h"
#include "stm32wbaxx_nucleo.h"
#include "host_stack_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  EATTS_APP_FLOW_OFF,
  EATTS_APP_FLOW_ON
} EATTS_App_Flow_Status_t;

typedef enum
{
  EATTS_APP_TRANSFER_REQ_OFF,
  EATTS_APP_TRANSFER_REQ_ON
} EATTS_App_Transfer_Req_Status_t;

typedef struct
{
  SVC1_Data_t TxData;
  EATTS_App_Transfer_Req_Status_t NotificationTransferReq;
  EATTS_App_Transfer_Req_Status_t ButtonTransferReq;
  EATTS_App_Flow_Status_t DtFlowStatus;
  uint8_t connectionstatus;
} EATTS_App_Context_t;
/* USER CODE END PTD */

typedef enum
{
  Char1_NOTIFICATION_OFF,
  Char1_NOTIFICATION_ON,
  Char2_NOTIFICATION_OFF,
  Char2_NOTIFICATION_ON,
  /* USER CODE BEGIN Service1_APP_SendInformation_t */

  /* USER CODE END Service1_APP_SendInformation_t */
  SVC1_APP_SENDINFORMATION_LAST
} SVC1_APP_SendInformation_t;

typedef struct
{
  SVC1_APP_SendInformation_t     Char1_Notification_Status;
  SVC1_APP_SendInformation_t     Char2_Notification_Status;
  /* USER CODE BEGIN Service1_APP_Context_t */
  /**
   * ID of the Write timeout
   */
  UTIL_TIMER_Object_t TimerDataThroughputWrite_Bearer1_Id;
  UTIL_TIMER_Object_t TimerDataThroughputWrite_Bearer2_Id;
  /* USER CODE END Service1_APP_Context_t */
  uint16_t              ConnectionHandle;
} SVC1_APP_Context_t;

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DELAY_1S (1000)
#define TIMEUNIT  1
/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */
extern uint16_t EATT_Bearer_connHdl;
extern uint16_t EATT_Bearer1_connHdl;
extern uint16_t EATT_Bearer2_connHdl;
extern uint16_t MTUSizeValue;
/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static SVC1_APP_Context_t SVC1_APP_Context;

uint8_t a_SVC1_UpdateCharData[247];

/* USER CODE BEGIN PV */
EATTS_App_Context_t EATTS_App_Context;
volatile uint32_t DataReceived1;
uint32_t DataReceived2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SVC1_Char1_SendNotification(void);
static void SVC1_Char2_SendNotification(void);

/* USER CODE BEGIN PFP */
static void BLE_App_Delay_DataThroughput_Bearer1(void);
static void BLE_App_Delay_DataThroughput_Bearer2(void);
static void DataThroughput_Bearer1_proc(void *arg);
static void DataThroughput_Bearer2_proc(void *arg);
static uint8_t Notification_Data_Buffer1[DATA_NOTIFICATION_MAX_PACKET_SIZE]; /* DATA_NOTIFICATION_MAX_PACKET_SIZE data + CRC */
static uint8_t Notification_Data_Buffer2[DATA_NOTIFICATION_MAX_PACKET_SIZE]; /* DATA_NOTIFICATION_MAX_PACKET_SIZE data + CRC */
static void SendDataUpdate1( void );
static void SendDataUpdate2( void );

extern uint16_t packet_lost;
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void SVC1_Notification(SVC1_NotificationEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service1_Notification_1 */

  /* USER CODE END Service1_Notification_1 */
  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service1_Notification_Service1_EvtOpcode */
    case BEARER1_SVC1_CHAR1_WRITE_NO_RESP_EVT:
      /* USER CODE BEGIN Service1Char1_WRITE_EVT */
      if (DataReceived1 == 0)
      {
        /* start timer */
        DataReceived1 += p_Notification->DataTransfered.Length;
        UTIL_TIMER_Start(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer1_Id));
      }
      else
      {
        DataReceived1 += p_Notification->DataTransfered.Length;
      }
      /* USER CODE END Service1Char1_WRITE_EVT */
      break;
      
    case BEARER2_SVC1_CHAR1_WRITE_NO_RESP_EVT:
      /* USER CODE BEGIN Service1Char1_WRITE_EVT */
      if (DataReceived2 == 0)
      {
        /* start timer */
        DataReceived2 += p_Notification->DataTransfered.Length;
        UTIL_TIMER_Start(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer2_Id));
      }
      else
      {
        DataReceived2 += p_Notification->DataTransfered.Length;
      }
      /* USER CODE END Service1Char1_WRITE_EVT */
      break;
    /* USER CODE END Service1_Notification_Service1_EvtOpcode */

    case SVC1_CHAR1_WRITE_NO_RESP_EVT:
      /* USER CODE BEGIN Service1Char1_WRITE_NO_RESP_EVT */

      /* USER CODE END Service1Char1_WRITE_NO_RESP_EVT */
      break;

    case SVC1_CHAR1_WRITE_EVT:
      /* USER CODE BEGIN Service1Char1_WRITE_EVT */
      if (DataReceived1 == 0)
      {
        /* start timer */
        DataReceived1 += p_Notification->DataTransfered.Length;
        UTIL_TIMER_Start(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer1_Id));
      }
      else
      {
        DataReceived1 += p_Notification->DataTransfered.Length;
      }
      /* USER CODE END Service1Char1_WRITE_EVT */
      break;

    case SVC1_CHAR1_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char1_NOTIFY_ENABLED_EVT */
      EATTS_App_Context.NotificationTransferReq = EATTS_APP_TRANSFER_REQ_ON;
      /* USER CODE END Service1Char1_NOTIFY_ENABLED_EVT */
      break;

    case SVC1_CHAR1_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char1_NOTIFY_DISABLED_EVT */

      /* USER CODE END Service1Char1_NOTIFY_DISABLED_EVT */
      break;

    case SVC1_CHAR2_WRITE_NO_RESP_EVT:
      /* USER CODE BEGIN Service1Char2_WRITE_NO_RESP_EVT */

      /* USER CODE END Service1Char2_WRITE_NO_RESP_EVT */
      break;

    case SVC1_CHAR2_WRITE_EVT:
      /* USER CODE BEGIN Service1Char2_WRITE_EVT */

      /* USER CODE END Service1Char2_WRITE_EVT */
      break;

    case SVC1_CHAR2_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char2_NOTIFY_ENABLED_EVT */

      /* USER CODE END Service1Char2_NOTIFY_ENABLED_EVT */
      break;

    case SVC1_CHAR2_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char2_NOTIFY_DISABLED_EVT */

      /* USER CODE END Service1Char2_NOTIFY_DISABLED_EVT */
      break;

    default:
      /* USER CODE BEGIN Service1_Notification_default */

      /* USER CODE END Service1_Notification_default */
      break;
  }
  /* USER CODE BEGIN Service1_Notification_2 */

  /* USER CODE END Service1_Notification_2 */
  return;
}

void SVC1_APP_EvtRx(SVC1_APP_ConnHandleNotEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service1_APP_EvtRx_1 */

  /* USER CODE END Service1_APP_EvtRx_1 */

  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service1_APP_EvtRx_Service1_EvtOpcode */

    /* USER CODE END Service1_APP_EvtRx_Service1_EvtOpcode */
    case SVC1_CONN_HANDLE_EVT :
      /* USER CODE BEGIN Service1_APP_CONN_HANDLE_EVT */
      EATTS_App_Context.connectionstatus = APP_BLE_CONNECTED_SERVER;
      UTIL_SEQ_SetTask(1U << CFG_TASK_CONN_UPDATE_REG_ID, CFG_SEQ_PRIO_0);
      /* USER CODE END Service1_APP_CONN_HANDLE_EVT */
      break;

    case SVC1_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN Service1_APP_DISCON_HANDLE_EVT */

      /* USER CODE END Service1_APP_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN Service1_APP_EvtRx_default */

      /* USER CODE END Service1_APP_EvtRx_default */
      break;
  }

  /* USER CODE BEGIN Service1_APP_EvtRx_2 */

  /* USER CODE END Service1_APP_EvtRx_2 */

  return;
}

void SVC1_APP_Init(void)
{
  UNUSED(SVC1_APP_Context);
  SVC1_Init();

  /* USER CODE BEGIN Service1_APP_Init */
  /**
  * Create timer for Data Throughput process (write data)
  */
  UTIL_TIMER_Create(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer1_Id),
                    0,
                    (UTIL_TIMER_Mode_t)UTIL_TIMER_ONESHOT,
                    &DataThroughput_Bearer1_proc, 
                    0);
  
  UTIL_TIMER_SetPeriod(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer1_Id), DELAY_1S);
  
  UTIL_TIMER_Create(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer2_Id),
                    0,
                    (UTIL_TIMER_Mode_t)UTIL_TIMER_ONESHOT,
                    &DataThroughput_Bearer2_proc, 
                    0);
  
  UTIL_TIMER_SetPeriod(&(SVC1_APP_Context.TimerDataThroughputWrite_Bearer2_Id), DELAY_1S);
  
  UTIL_SEQ_RegTask(1U << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER1_ID, UTIL_SEQ_RFU, SendDataUpdate1);
  UTIL_SEQ_RegTask(1U << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER2_ID, UTIL_SEQ_RFU, SendDataUpdate2);
  
  UTIL_SEQ_RegTask(1U << CFG_TASK_DATA_WRITE_BEARER1_ID, UTIL_SEQ_RFU, BLE_App_Delay_DataThroughput_Bearer1);
  UTIL_SEQ_RegTask(1U << CFG_TASK_DATA_WRITE_BEARER2_ID, UTIL_SEQ_RFU, BLE_App_Delay_DataThroughput_Bearer2);
  
  /**
   * Initialize data buffer
   */
  uint8_t i;
  for (i = 0 ; i< (DATA_NOTIFICATION_MAX_PACKET_SIZE - 1) ; i++)
  {
    Notification_Data_Buffer1[i] = i;
    Notification_Data_Buffer2[i] = i;
  }

  EATTS_App_Context.ButtonTransferReq = EATTS_APP_TRANSFER_REQ_OFF;
  EATTS_App_Context.NotificationTransferReq = EATTS_APP_TRANSFER_REQ_OFF;
  EATTS_App_Context.DtFlowStatus = EATTS_APP_FLOW_ON;
  EATTS_App_Context.connectionstatus = APP_BLE_IDLE;
  /* USER CODE END Service1_APP_Init */
  return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
__USED void SVC1_Char1_SendNotification(void) /* Property Notification */
{
  SVC1_APP_SendInformation_t notification_on_off = Char1_NOTIFICATION_OFF;
  SVC1_Data_t svc1_notification_data;

  svc1_notification_data.p_Payload = (uint8_t*)a_SVC1_UpdateCharData;
  svc1_notification_data.Length = 0;

  /* USER CODE BEGIN Service1Char1_NS_1*/

  /* USER CODE END Service1Char1_NS_1*/

  if (notification_on_off != Char1_NOTIFICATION_OFF)
  {
    SVC1_UpdateValue(SVC1_CHAR1, &svc1_notification_data);
  }

  /* USER CODE BEGIN Service1Char1_NS_Last*/

  /* USER CODE END Service1Char1_NS_Last*/

  return;
}

__USED void SVC1_Char2_SendNotification(void) /* Property Notification */
{
  SVC1_APP_SendInformation_t notification_on_off = Char2_NOTIFICATION_OFF;
  SVC1_Data_t svc1_notification_data;

  svc1_notification_data.p_Payload = (uint8_t*)a_SVC1_UpdateCharData;
  svc1_notification_data.Length = 0;

  /* USER CODE BEGIN Service1Char2_NS_1*/

  /* USER CODE END Service1Char2_NS_1*/

  if (notification_on_off != Char2_NOTIFICATION_OFF)
  {
    SVC1_UpdateValue(SVC1_CHAR2, &svc1_notification_data);
  }

  /* USER CODE BEGIN Service1Char2_NS_Last*/

  /* USER CODE END Service1Char2_NS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/
void APPE_Button1Action(void)
{
  if (EATTS_App_Context.connectionstatus != APP_BLE_CONNECTED_SERVER)
  {

  }
  else
  {
    if(EATTS_App_Context.ButtonTransferReq != EATTS_APP_TRANSFER_REQ_OFF)
    {
      BSP_LED_Off(LED_BLUE);
      EATTS_App_Context.ButtonTransferReq = EATTS_APP_TRANSFER_REQ_OFF;
    }
    else
    {
      BSP_LED_On(LED_BLUE);
      EATTS_App_Context.ButtonTransferReq = EATTS_APP_TRANSFER_REQ_ON;
      UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER1_ID, CFG_SEQ_PRIO_0);
    }
  }
  BleStackCB_Process();
  
  return;
}

static void BLE_App_Delay_DataThroughput_Bearer1(void)
{
  uint32_t DataThroughput1;
  
  DataThroughput1 = (uint32_t)(DataReceived1/TIMEUNIT);
  LOG_INFO_APP("  DataThroughput1 = %ld  bytes/s lost = %ld \n", DataThroughput1, packet_lost);
  
  DataReceived1 = 0;
  packet_lost = 0;

  BleStackCB_Process();
}

static void BLE_App_Delay_DataThroughput_Bearer2(void)
{
  uint32_t DataThroughput;
  
  DataThroughput = (uint32_t)(DataReceived2/TIMEUNIT);
  LOG_INFO_APP("  DataThroughput2 = %ld  bytes/s lost = %ld \n", DataThroughput, packet_lost);
  
  DataReceived2 = 0;
  packet_lost = 0;

  BleStackCB_Process();
}

static void DataThroughput_Bearer1_proc(void *arg){
  
  UTIL_SEQ_SetTask(1U << CFG_TASK_DATA_WRITE_BEARER1_ID, CFG_SEQ_PRIO_0);
}

static void DataThroughput_Bearer2_proc(void *arg){
  
  UTIL_SEQ_SetTask(1U << CFG_TASK_DATA_WRITE_BEARER2_ID, CFG_SEQ_PRIO_0);
}

static void SendDataUpdate1( void )
{
  tBleStatus status = BLE_STATUS_INVALID_PARAMS;
  uint8_t crc_result;

  if( (EATTS_App_Context.ButtonTransferReq != EATTS_APP_TRANSFER_REQ_OFF)
      && (EATTS_App_Context.NotificationTransferReq != EATTS_APP_TRANSFER_REQ_OFF)
      && (EATTS_App_Context.DtFlowStatus != EATTS_APP_FLOW_OFF) )
  {   
    /*Data Packet to send to remote*/
    Notification_Data_Buffer1[0] += 1;
    MTUSizeValue = 241;
    /* compute CRC */
    crc_result = APP_BLE_ComputeCRC8((uint8_t*) Notification_Data_Buffer1, (MTUSizeValue - 1));
    Notification_Data_Buffer1[MTUSizeValue - 1] = crc_result;

    EATTS_App_Context.TxData.p_Payload = Notification_Data_Buffer1;
    EATTS_App_Context.TxData.Length =  241; //MTUSizeValue;
    EATT_Bearer1_connHdl = BleCoCEATTContext.EATT_Bearer_connHdl[0];
    EATT_Bearer_connHdl = EATT_Bearer1_connHdl;
    
    status = SVC1_UpdateValue_Ext(SVC1_CHAR1, (SVC1_Data_t *) &EATTS_App_Context.TxData, EATT_Bearer_connHdl);
    
    if (status == BLE_STATUS_INSUFFICIENT_RESOURCES)
    {
      EATTS_App_Context.DtFlowStatus = EATTS_APP_FLOW_OFF;
      (Notification_Data_Buffer1[0])-=1;
    }
    else
    {
      UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER1_ID, CFG_SEQ_PRIO_0);
    }
  }

  BleStackCB_Process();
  return;
}

static void SendDataUpdate2( void )
{
  tBleStatus status = BLE_STATUS_INVALID_PARAMS;
  uint8_t crc_result;

  if( (EATTS_App_Context.ButtonTransferReq != EATTS_APP_TRANSFER_REQ_OFF)
      && (EATTS_App_Context.NotificationTransferReq != EATTS_APP_TRANSFER_REQ_OFF)
      && (EATTS_App_Context.DtFlowStatus != EATTS_APP_FLOW_OFF) )
  {   
    /*Data Packet to send to remote*/
    Notification_Data_Buffer2[0] += 1;
    MTUSizeValue = 241;
    /* compute CRC */
    crc_result = APP_BLE_ComputeCRC8((uint8_t*) Notification_Data_Buffer2, (MTUSizeValue - 1));
    Notification_Data_Buffer2[MTUSizeValue - 1] = crc_result;

    EATTS_App_Context.TxData.p_Payload = Notification_Data_Buffer2;
    EATTS_App_Context.TxData.Length =  241;
    EATT_Bearer2_connHdl = BleCoCEATTContext.EATT_Bearer_connHdl[1];
    EATT_Bearer_connHdl = EATT_Bearer2_connHdl;

    status = SVC1_UpdateValue_Ext(SVC1_CHAR1, (SVC1_Data_t *) &EATTS_App_Context.TxData, EATT_Bearer_connHdl);
    
    if (status == BLE_STATUS_INSUFFICIENT_RESOURCES)
    {
      EATTS_App_Context.DtFlowStatus = EATTS_APP_FLOW_OFF;
      (Notification_Data_Buffer2[0])-=1;
    }
    else
    {
      UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER2_ID, CFG_SEQ_PRIO_0);
    }
  }

  BleStackCB_Process();
  return;
}

void Resume_Notification1(void)
{
  EATTS_App_Context.DtFlowStatus = EATTS_APP_FLOW_ON;
  UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER1_ID, CFG_SEQ_PRIO_0);
}

void Resume_Notification2(void)
{
  EATTS_App_Context.DtFlowStatus = EATTS_APP_FLOW_ON;
  UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_TRANSFER_UPDATE_BEARER2_ID, CFG_SEQ_PRIO_0);
}
/* USER CODE END FD_LOCAL_FUNCTIONS*/
