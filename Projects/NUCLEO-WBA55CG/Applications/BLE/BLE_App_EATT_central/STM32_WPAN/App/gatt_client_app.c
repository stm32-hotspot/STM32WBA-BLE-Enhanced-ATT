/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gatt_client_app.c
  * @author  MCD Application Team
  * @brief   GATT Client Application
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "log_module.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "gatt_client_app.h"
#include "stm32_seq.h"
#include "app_ble.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32_timer.h"
#include "stm32wbaxx_nucleo.h"
#include "host_stack_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */


typedef struct
{
  /**
   * ID of the Write timeout
   */
//  UTIL_TIMER_Object_t TimerDataThroughput_Id;
  UTIL_TIMER_Object_t TimerDataThroughput_Update_Bearer1_Id;
  UTIL_TIMER_Object_t TimerDataThroughput_Update_Bearer2_Id;
  /* USER CODE END Service1_APP_Context_t */
  uint16_t              ConnectionHandle;
} EATT_CLI_APP_Context_t;
/* USER CODE END PTD */

typedef enum
{
  NOTIFICATION_INFO_RECEIVED_EVT,
  /* USER CODE BEGIN GATT_CLIENT_APP_Opcode_t */
  NOTIFICATION_BEARER1_SVC1_CHAR1_UPDATE_EVT,
  NOTIFICATION_BEARER2_SVC1_CHAR1_UPDATE_EVT,
  /* USER CODE END GATT_CLIENT_APP_Opcode_t */
}GATT_CLIENT_APP_Opcode_t;

typedef struct
{
  uint8_t *p_Payload;
  uint8_t length;
}GATT_CLIENT_APP_Data_t;

typedef struct
{
  GATT_CLIENT_APP_Opcode_t Client_Evt_Opcode;
  GATT_CLIENT_APP_Data_t   DataTransfered;
}GATT_CLIENT_APP_Notification_evt_t;

typedef struct
{
  EATTC_STM_Payload_t TxData;
  EATTC_App_Transfer_Req_Status_t NotificationTransferReq;
  EATTC_App_Transfer_Req_Status_t ButtonTransferReq;
  EATTC_App_Flow_Status_t DtFlowStatus;
  uint8_t connectionstatus;
} EATTC_Context_t;

typedef struct
{
  EATTC_STM_Payload_t     DataTransfered;
  uint16_t                ConnectionHandle;
  uint8_t                 ServiceInstance;

  /* USER CODE BEGIN Service1_NotificationEvt_t */
  GATT_CLIENT_APP_Opcode_t       EvtOpcode;
  /* USER CODE END Service1_NotificationEvt_t */

} EATT_CLI_NotificationEvt_t;

typedef struct
{
  GATT_CLIENT_APP_State_t state;

  APP_BLE_ConnStatus_t connStatus;
  uint16_t connHdl;

  uint16_t ALLServiceStartHdl;
  uint16_t ALLServiceEndHdl;

  uint16_t GAPServiceStartHdl;
  uint16_t GAPServiceEndHdl;

  uint16_t GATTServiceStartHdl;
  uint16_t GATTServiceEndHdl;

  uint16_t ServiceChangedCharStartHdl;
  uint16_t ServiceChangedCharValueHdl;
  uint16_t ServiceChangedCharDescHdl;
  uint16_t ServiceChangedCharEndHdl;
  uint8_t ServiceChangedCharProperties;
  /* EATT bearer */
  uint16_t EATT_Bearer_connHdl[6];
  /* handles of the P2P service */
  uint16_t EATTServiceHdl;
  uint16_t EATTServiceEndHdl;

  /* handles of the Tx characteristic - Write To Server */
  uint16_t EATTChar1CharHdl;
  uint16_t EATTChar1ValueHdl;
  uint16_t EATTChar1DescHdl;

  /* handles of the Rx characteristic - Notification From Server */
  uint16_t EATTChar2CharHdl;
  uint16_t EATTChar2ValueHdl;
  uint16_t EATTChar2DescHdl;
  uint16_t MTUSizeValue;

  /* USER CODE BEGIN BleClientAppContext_t */

/* USER CODE END BleClientAppContext_t */

}BleClientAppContext_t;

/* Private defines ------------------------------------------------------------*/
#define GATT_EAB_PREFIX                         0xEAU

/* USER CODE BEGIN PD */
#define EATT_SERVICE_UUID (0xAAAA)
#define EATT_CHAR1_CHAR_UUID (0x1111)
#define EATT_CHAR2_CHAR_UUID (0x2222)

#define DELAY_1S (1000)
#define TIMEUNIT  1
/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
#define UNPACK_2_BYTE_PARAMETER(ptr)  \
        (uint16_t)((uint16_t)(*((uint8_t *)ptr))) |   \
        (uint16_t)((((uint16_t)(*((uint8_t *)ptr + 1))) << 8))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

static BleClientAppContext_t a_ClientContext[BLE_CFG_CLT_MAX_NBR_CB];
static uint16_t gattCharStartHdl = 0;
static uint16_t gattCharValueHdl = 0;

/* USER CODE BEGIN PV */
EATTC_Context_t EATTC_Context;
uint8_t Notification_Data_Buffer1[DATA_NOTIFICATION_MAX_PACKET_SIZE];
uint8_t Notification_Data_Buffer2[DATA_NOTIFICATION_MAX_PACKET_SIZE];
static EATT_CLI_NotificationEvt_t NotificationData1;
static EATT_CLI_NotificationEvt_t NotificationData2;
//static uint8_t g_Notification1_n_1 = 0;
//static uint8_t g_Notification2_n_1 = 0;
static uint16_t packet_lost;
static uint32_t DataThroughputValue1;
static uint32_t DataThroughputValue2;
static uint32_t packet_lost_local;
//static void DataT_Notification_Data( void );
static void DataT_Notification_Data_Bearer1( void );
static void DataT_Notification_Data_Bearer2( void );
uint32_t dtc_n=0;
uint32_t DataTransfered1 = 0;
uint32_t DataTransfered2 = 0;
EATT_CLI_APP_Context_t EATT_CLI_APP_Context;
static uint16_t EATT_Bearer_connHdl;
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/
/* USER CODE BEGIN GV */
extern BleCoC_EATTContext_t BleCoC_EATTContextCentral;
/* USER CODE END GV */

/* Private function prototypes -----------------------------------------------*/
static SVCCTL_EvtAckStatus_t Event_Handler(void *Event);
static void gatt_parse_services(aci_att_read_by_group_type_resp_event_rp0 *p_evt);
static void gatt_parse_services_by_UUID(aci_att_find_by_type_value_resp_event_rp0 *p_evt);
static void gatt_parse_chars(aci_att_read_by_type_resp_event_rp0 *p_evt);
static void gatt_parse_descs(aci_att_find_info_resp_event_rp0 *p_evt);
static void gatt_parse_notification(aci_gatt_notification_event_rp0 *p_evt);
static void gatt_Notification(GATT_CLIENT_APP_Notification_evt_t *p_Notif);
static void client_discover_all(void);
static void gatt_cmd_resp_release(void);
static void gatt_cmd_resp_wait(void);
/* USER CODE BEGIN PFP */
static void SendDataWrite1( void );
static void Resume_Write1( void );
static void SendDataWrite2( void );
static void Resume_Write2( void );
static void DataThroughputNotif_Bearer1_proc(void *arg);
static void DataThroughputNotif_Bearer2_proc(void *arg);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void GATT_CLIENT_APP_Init(void)
{
  uint8_t index =0;
  /* USER CODE BEGIN GATT_CLIENT_APP_Init_1 */
  uint8_t i;
  /* USER CODE END GATT_CLIENT_APP_Init_1 */
  for(index = 0; index < BLE_CFG_CLT_MAX_NBR_CB; index++)
  {
    a_ClientContext[index].connStatus = APP_BLE_IDLE;
  }

  /* Register the event handler to the BLE controller */
  SVCCTL_RegisterCltHandler(Event_Handler);

  /* Register a task allowing to discover all services and characteristics and enable all notifications */
  UTIL_SEQ_RegTask(1U << CFG_TASK_DISCOVER_SERVICES_ID, UTIL_SEQ_RFU, client_discover_all);

  /* USER CODE BEGIN GATT_CLIENT_APP_Init_2 */
  
   UTIL_TIMER_Create(&(EATT_CLI_APP_Context.TimerDataThroughput_Update_Bearer1_Id),
                    0,
                    (UTIL_TIMER_Mode_t)UTIL_TIMER_ONESHOT,
                    &DataThroughputNotif_Bearer1_proc, 
                    0);
  
  UTIL_TIMER_SetPeriod(&(EATT_CLI_APP_Context.TimerDataThroughput_Update_Bearer1_Id), DELAY_1S);
  
  UTIL_TIMER_Create(&(EATT_CLI_APP_Context.TimerDataThroughput_Update_Bearer2_Id),
                    0,
                    (UTIL_TIMER_Mode_t)UTIL_TIMER_ONESHOT,
                    &DataThroughputNotif_Bearer2_proc, 
                    0);
  
  UTIL_TIMER_SetPeriod(&(EATT_CLI_APP_Context.TimerDataThroughput_Update_Bearer2_Id), DELAY_1S);
  
  UTIL_SEQ_RegTask(1U << CFG_TASK_DATA_FROM_SERVER_BEARER1_ID, UTIL_SEQ_RFU, DataT_Notification_Data_Bearer1); 
  
  UTIL_SEQ_RegTask(1U << CFG_TASK_DATA_FROM_SERVER_BEARER2_ID, UTIL_SEQ_RFU, DataT_Notification_Data_Bearer2); 
  
  UTIL_SEQ_RegTask(1U << CFG_TASK_WRITE_DATA_BEARER1_ID, UTIL_SEQ_RFU, SendDataWrite1);
  
  UTIL_SEQ_RegTask(1U << CFG_TASK_WRITE_DATA_BEARER2_ID, UTIL_SEQ_RFU, SendDataWrite2);
  /**
   * Initialize data buffer
   */
  for (i = 0 ; i < (DATA_NOTIFICATION_MAX_PACKET_SIZE - 1) ; i++)
  {
    Notification_Data_Buffer1[i] = i;
    Notification_Data_Buffer2[i] = i;
  }

  a_ClientContext[0].state = GATT_CLIENT_APP_IDLE;
  a_ClientContext[0].connHdl = 0xFFFF;
  EATTC_Context.connectionstatus = APP_BLE_IDLE;
  EATTC_Context.NotificationTransferReq = EATTC_APP_TRANSFER_REQ_OFF;
  EATTC_Context.ButtonTransferReq = EATTC_APP_TRANSFER_REQ_OFF;
  EATTC_Context.DtFlowStatus = EATTC_APP_FLOW_ON;
  /* USER CODE END GATT_CLIENT_APP_Init_2 */
  return;
}

void GATT_CLIENT_APP_Notification(GATT_CLIENT_APP_ConnHandle_Notif_evt_t *p_Notif)
{
  /* USER CODE BEGIN GATT_CLIENT_APP_Notification_1 */

  /* USER CODE END GATT_CLIENT_APP_Notification_1 */
  switch(p_Notif->ConnOpcode)
  {
    /* USER CODE BEGIN ConnOpcode */
    
    /* USER CODE END ConnOpcode */

    case PEER_CONN_HANDLE_EVT :
      /* USER CODE BEGIN PEER_CONN_HANDLE_EVT */

      /* USER CODE END PEER_CONN_HANDLE_EVT */
      break;

    case PEER_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN PEER_DISCON_HANDLE_EVT */
    {
      uint8_t index = 0;

      while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
            (a_ClientContext[index].state != GATT_CLIENT_APP_IDLE))
      {
        a_ClientContext[index].state = GATT_CLIENT_APP_IDLE;
      }
    }
      /* USER CODE END PEER_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN ConnOpcode_Default */

      /* USER CODE END ConnOpcode_Default */
      break;
  }
  /* USER CODE BEGIN GATT_CLIENT_APP_Notification_2 */

  /* USER CODE END GATT_CLIENT_APP_Notification_2 */
  return;
}

uint8_t GATT_CLIENT_APP_Set_Conn_Handle(uint8_t index, uint16_t connHdl)
{
  uint8_t ret;

  if (index < BLE_CFG_CLT_MAX_NBR_CB)
  {
    a_ClientContext[index].connHdl = connHdl;
    ret = 0;
  }
  else
  {
    ret = 1;
  }

  return ret;
}

uint8_t GATT_CLIENT_APP_Get_State(uint8_t index)
{
  return a_ClientContext[index].state;
}

void GATT_CLIENT_APP_Discover_services(uint8_t index)
{
  GATT_CLIENT_APP_Procedure_Gatt(index, PROC_GATT_DISC_ALL_PRIMARY_SERVICES);
  GATT_CLIENT_APP_Procedure_Gatt(index, PROC_GATT_DISC_ALL_CHARS);
  GATT_CLIENT_APP_Procedure_Gatt(index, PROC_GATT_DISC_ALL_DESCS);
  GATT_CLIENT_APP_Procedure_Gatt(index, PROC_GATT_PROPERTIES_ENABLE_ALL);

  return;
}

uint8_t GATT_CLIENT_APP_Procedure_Gatt(uint8_t index, ProcGattId_t GattProcId)
{
  tBleStatus result = BLE_STATUS_SUCCESS;
  uint8_t status;

  if (index >= BLE_CFG_CLT_MAX_NBR_CB)
  {
    status = 1;
  }
  else
  {
    status = 0;
    switch (GattProcId)
    {
      case PROC_GATT_DISC_ALL_PRIMARY_SERVICES:
      {
        a_ClientContext[index].state = GATT_CLIENT_APP_DISCOVER_SERVICES;

        LOG_INFO_APP("GATT services discovery\n");
        result = aci_gatt_disc_all_primary_services(a_ClientContext[index].connHdl);

        if (result == BLE_STATUS_SUCCESS)
        {
          gatt_cmd_resp_wait();
          LOG_INFO_APP("PROC_GATT_DISC_ALL_PRIMARY_SERVICES services discovered Successfully\n\n");
        }
        else
        {
          LOG_INFO_APP("PROC_GATT_DISC_ALL_PRIMARY_SERVICES aci_gatt_disc_all_primary_services cmd NOK status =0x%02X\n\n", result);
        }
      }
      break; /* PROC_GATT_DISC_ALL_PRIMARY_SERVICES */

      case PROC_GATT_DISC_ALL_CHARS:
      {
        a_ClientContext[index].state = GATT_CLIENT_APP_DISCOVER_CHARACS;

        LOG_INFO_APP("DISCOVER_ALL_CHARS ConnHdl=0x%04X ALLServiceHandle[0x%04X - 0x%04X]\n",
                          a_ClientContext[index].connHdl,
                          a_ClientContext[index].ALLServiceStartHdl,
                          a_ClientContext[index].ALLServiceEndHdl);

        result = aci_gatt_disc_all_char_of_service(
                           a_ClientContext[index].connHdl,
                           a_ClientContext[index].ALLServiceStartHdl,
                           a_ClientContext[index].ALLServiceEndHdl);

        if (result == BLE_STATUS_SUCCESS)
        {
          gatt_cmd_resp_wait();
          LOG_INFO_APP("All characteristics discovered Successfully\n\n");
        }
        else
        {
          LOG_INFO_APP("All characteristics discovery Failed, status =0x%02X\n\n", result);
        }
      }
      break; /* PROC_GATT_DISC_ALL_CHARS */

      case PROC_GATT_DISC_ALL_DESCS:
      {
        a_ClientContext[index].state = GATT_CLIENT_APP_DISCOVER_WRITE_DESC;

        LOG_INFO_APP("DISCOVER_ALL_CHAR_DESCS [0x%04X - 0x%04X]\n",
                         a_ClientContext[index].ALLServiceStartHdl,
                         a_ClientContext[index].ALLServiceEndHdl);
        result = aci_gatt_disc_all_char_desc(
                         a_ClientContext[index].connHdl,
                         a_ClientContext[index].ALLServiceStartHdl,
                         a_ClientContext[index].ALLServiceEndHdl);

        if (result == BLE_STATUS_SUCCESS)
        {
          gatt_cmd_resp_wait();
          LOG_INFO_APP("All characteristic descriptors discovered Successfully\n\n");
        }
        else
        {
          LOG_INFO_APP("All characteristic descriptors discovery Failed, status =0x%02X\n\n", result);
        }
      }
      break; /* PROC_GATT_DISC_ALL_DESCS */
      case PROC_GATT_PROPERTIES_ENABLE_ALL:
      {
        uint16_t charPropVal = 0x0000;

        if (a_ClientContext[index].ServiceChangedCharDescHdl != 0x0000)
        {
          if(((a_ClientContext[index].ServiceChangedCharProperties) & CHAR_PROP_NOTIFY) == CHAR_PROP_NOTIFY)
          {
            charPropVal = 0x0001;
          }
          if(((a_ClientContext[index].ServiceChangedCharProperties) & CHAR_PROP_INDICATE) == CHAR_PROP_INDICATE)
          {
            charPropVal = 0x0002;
          }
          result = aci_gatt_write_char_desc(a_ClientContext[index].connHdl,
                                            a_ClientContext[index].ServiceChangedCharDescHdl,
                                            2,
                                            (uint8_t *) &charPropVal);
          gatt_cmd_resp_wait();
          LOG_INFO_APP(" ServiceChangedCharDescHdl =0x%04X\n",a_ClientContext[index].ServiceChangedCharDescHdl);
        }
        /* USER CODE BEGIN PROC_GATT_PROPERTIES_ENABLE_ALL */
        /* Enable TX notification */
        uint16_t enable = 0x0001;
        /* Enable RX notification */
        if (a_ClientContext[index].EATTChar1DescHdl != 0x0000)
        {
          result = aci_gatt_write_char_desc(a_ClientContext[index].connHdl,
                                            a_ClientContext[index].EATTChar1DescHdl,
                                            2,
                                            (uint8_t *) &enable);
          gatt_cmd_resp_wait();
          APP_DBG_MSG(" EATTChar1DescHdl =0x%04X\n",a_ClientContext[index].EATTChar1DescHdl);
        }
        /* Enable Throughput notification */
        if (a_ClientContext[index].EATTChar2DescHdl != 0x0000)
        {
          result = aci_gatt_write_char_desc(a_ClientContext[index].connHdl,
                                            a_ClientContext[index].EATTChar2DescHdl,
                                            2,
                                            (uint8_t *) &enable);
          gatt_cmd_resp_wait();
          APP_DBG_MSG(" EATTChar2DescHdl =0x%04X\n",a_ClientContext[index].EATTChar2DescHdl);
        }
        /* USER CODE END PROC_GATT_PROPERTIES_ENABLE_ALL */

        if (result == BLE_STATUS_SUCCESS)
        {
          LOG_INFO_APP("All notifications enabled Successfully\n\n");
          APP_BLE_Procedure_Gap_General(PROC_GATT_EXCHANGE_CONFIG);
        }
        else
        {
          LOG_INFO_APP("All notifications enabled Failed, status =0x%02X\n\n", result);
        }
      }
      break; /* PROC_GATT_PROPERTIES_ENABLE_ALL */

    default:
      break;
    }
  }

  return status;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
static SVCCTL_EvtAckStatus_t Event_Handler(void *Event)
{
  SVCCTL_EvtAckStatus_t return_value;
  hci_event_pckt *event_pckt;
  evt_blecore_aci *p_blecore_evt;

  GATT_CLIENT_APP_Notification_evt_t Notification;
  UNUSED(Notification);

  return_value = SVCCTL_EvtNotAck;
  event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)Event)->data);

  switch (event_pckt->evt)
  {
    case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:
    {
      p_blecore_evt = (evt_blecore_aci*)event_pckt->data;
      switch (p_blecore_evt->ecode)
      {
        case ACI_ATT_READ_BY_GROUP_TYPE_RESP_VSEVT_CODE:
        {
          aci_att_read_by_group_type_resp_event_rp0 *p_evt_rsp = (void*)p_blecore_evt->data;
          gatt_parse_services((aci_att_read_by_group_type_resp_event_rp0 *)p_evt_rsp);
        }
        break; /* ACI_ATT_READ_BY_GROUP_TYPE_RESP_VSEVT_CODE */
        case ACI_ATT_FIND_BY_TYPE_VALUE_RESP_VSEVT_CODE:
        {
          aci_att_find_by_type_value_resp_event_rp0 *p_evt_rsp = (void*) p_blecore_evt->data;
          gatt_parse_services_by_UUID((aci_att_find_by_type_value_resp_event_rp0 *)p_evt_rsp);
        }
        break; /* ACI_ATT_FIND_BY_TYPE_VALUE_RESP_VSEVT_CODE */
        case ACI_ATT_READ_BY_TYPE_RESP_VSEVT_CODE:
        {
          aci_att_read_by_type_resp_event_rp0 *p_evt_rsp = (void*)p_blecore_evt->data;
          gatt_parse_chars((aci_att_read_by_type_resp_event_rp0 *)p_evt_rsp);
        }
        break; /* ACI_ATT_READ_BY_TYPE_RESP_VSEVT_CODE */
        case ACI_ATT_FIND_INFO_RESP_VSEVT_CODE:
        {
          aci_att_find_info_resp_event_rp0 *p_evt_rsp = (void*)p_blecore_evt->data;
          gatt_parse_descs((aci_att_find_info_resp_event_rp0 *)p_evt_rsp);
        }
        break; /* ACI_ATT_FIND_INFO_RESP_VSEVT_CODE */
        case ACI_GATT_NOTIFICATION_VSEVT_CODE:
        {
          aci_gatt_notification_event_rp0 *p_evt_rsp = (void*)p_blecore_evt->data;
          gatt_parse_notification((aci_gatt_notification_event_rp0 *)p_evt_rsp);
        }
        break;/* ACI_GATT_NOTIFICATION_VSEVT_CODE */
        case ACI_GATT_PROC_COMPLETE_VSEVT_CODE:
        {
          aci_gatt_proc_complete_event_rp0 *p_evt_rsp = (void*)p_blecore_evt->data;

          uint8_t index;
          for (index = 0 ; index < BLE_CFG_CLT_MAX_NBR_CB ; index++)
          {
            if (a_ClientContext[index].connHdl == p_evt_rsp->Connection_Handle)
            {
              gatt_cmd_resp_release();
              break;
            }
          }
        }
        break;/* ACI_GATT_PROC_COMPLETE_VSEVT_CODE */
        case ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE:
        {
          aci_att_exchange_mtu_resp_event_rp0 *tx_pool_available;
          tx_pool_available = (aci_att_exchange_mtu_resp_event_rp0 *)p_blecore_evt->data;
          UNUSED(tx_pool_available);
          /* USER CODE BEGIN ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE */
          uint8_t index;
          for(index = 0; index < BLE_CFG_CLT_MAX_NBR_CB; index++)
          {
            if (EATT_Bearer_connHdl == a_ClientContext[index].EATT_Bearer_connHdl[0])
              Resume_Write2();
            else
              Resume_Write1();
          }
          /* USER CODE END ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE */
        }
        break;/* ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE*/
        case ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE:
        {
          aci_att_exchange_mtu_resp_event_rp0 * exchange_mtu_resp;
          exchange_mtu_resp = (aci_att_exchange_mtu_resp_event_rp0 *)p_blecore_evt->data;
          LOG_INFO_APP("  MTU exchanged size = %d\n",exchange_mtu_resp->Server_RX_MTU );
          UNUSED(exchange_mtu_resp);
          /* USER CODE BEGIN ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE */
          uint8_t index;
          for(index = 0; index < BLE_CFG_CLT_MAX_NBR_CB; index++)
          {
            if (exchange_mtu_resp->Server_RX_MTU < DATA_NOTIFICATION_MAX_PACKET_SIZE)
            {
              a_ClientContext[index].MTUSizeValue = exchange_mtu_resp->Server_RX_MTU - 3;
            }
            else
            {
              a_ClientContext[index].MTUSizeValue = DATA_NOTIFICATION_MAX_PACKET_SIZE;
            }
            LOG_INFO_APP("  MTU_size = %d\n", a_ClientContext[index].MTUSizeValue);
            tBleStatus status;
            status = hci_le_set_data_length(a_ClientContext[index].connHdl,251,2120);
            if (status != BLE_STATUS_SUCCESS)
            {
              LOG_INFO_APP("  Fail   : set data length command   : error code: 0x%x \n\r", status);
            }
            else
            {
              LOG_INFO_APP("  Success: set data length command  \n\r");
            }
          }
          /* USER CODE END ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE */
        }
        break;

        case (ACI_GATT_EATT_BEARER_VSEVT_CODE):
        {
          aci_gatt_eatt_bearer_event_rp0 *eatt_bearer_event;
		  uint8_t index;

          eatt_bearer_event = (aci_gatt_eatt_bearer_event_rp0*)p_blecore_evt->data;
          APP_DBG_MSG(">>== ACI_GATT_EATT_BEARER_VSEVT_CODE\n");
          APP_DBG_MSG(">>== Channel_Index = %d\n",eatt_bearer_event->Channel_Index );

          /* USER CODE BEGIN ACI_GATT_EATT_BEARER_VSEVT_CODE_1 */

          /* USER CODE END ACI_GATT_EATT_BEARER_VSEVT_CODE_1 */
		  for(index = 0; index < BLE_CFG_CLT_MAX_NBR_CB; index++)
          {
            a_ClientContext[index].EATT_Bearer_connHdl[eatt_bearer_event->Channel_Index] = (GATT_EAB_PREFIX << 8) | eatt_bearer_event->Channel_Index;
          }

		  /* USER CODE BEGIN ACI_GATT_EATT_BEARER_VSEVT_CODE_2 */

          /* USER CODE END ACI_GATT_EATT_BEARER_VSEVT_CODE_2 */
        }
        break;

        default:
          break;
      }/* end switch (p_blecore_evt->ecode) */
    }
    break; /* HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE */
    default:
      break;
  }/* end switch (event_pckt->evt) */

  return(return_value);
}

__USED static void gatt_Notification(GATT_CLIENT_APP_Notification_evt_t *p_Notif)
{
  /* USER CODE BEGIN gatt_Notification_1*/

  /* USER CODE END gatt_Notification_1 */
  switch (p_Notif->Client_Evt_Opcode)
  {
    /* USER CODE BEGIN Client_Evt_Opcode */
    case NOTIFICATION_BEARER1_SVC1_CHAR1_UPDATE_EVT:
      break;
      
    case NOTIFICATION_BEARER2_SVC1_CHAR1_UPDATE_EVT:
      break;

    /* USER CODE END Client_Evt_Opcode */

    case NOTIFICATION_INFO_RECEIVED_EVT:
      /* USER CODE BEGIN NOTIFICATION_INFO_RECEIVED_EVT */

      /* USER CODE END NOTIFICATION_INFO_RECEIVED_EVT */
      break;

    default:
      /* USER CODE BEGIN Client_Evt_Opcode_Default */

      /* USER CODE END Client_Evt_Opcode_Default */
      break;
  }
  /* USER CODE BEGIN gatt_Notification_2*/

  /* USER CODE END gatt_Notification_2 */
  return;
}

/**
* function of GATT service parse
*/
static void gatt_parse_services(aci_att_read_by_group_type_resp_event_rp0 *p_evt)
{
  uint16_t uuid, ServiceStartHdl, ServiceEndHdl;
  uint8_t uuid_offset, uuid_size = 0U, uuid_short_offset = 0U;
  uint8_t i, idx, numServ, index;

  LOG_INFO_APP("ACI_ATT_READ_BY_GROUP_TYPE_RESP_VSEVT_CODE - ConnHdl=0x%04X\n",
                p_evt->Connection_Handle);

  for (index = 0 ; index < BLE_CFG_CLT_MAX_NBR_CB ; index++)
  {
    if (a_ClientContext[index].connHdl == p_evt->Connection_Handle)
    {
      break;
    }
  }

  /* index < BLE_CFG_CLT_MAX_NBR_CB means connection handle identified */
  if (index < BLE_CFG_CLT_MAX_NBR_CB)
  {
    /* Number of attribute value tuples */
    numServ = (p_evt->Data_Length) / p_evt->Attribute_Data_Length;

    /* event data in Attribute_Data_List contains:
    * 2 bytes for start handle
    * 2 bytes for end handle
    * 2 or 16 bytes data for UUID
    */
    uuid_offset = 4;           /*UUID offset in bytes in Attribute_Data_List */
    if (p_evt->Attribute_Data_Length == 20) /* we are interested in the UUID is 128 bit.*/
    {
      idx = 16;                /*UUID index of 2 bytes read part in Attribute_Data_List */
      uuid_size = 16;          /*UUID size in bytes */
      uuid_short_offset = 12;  /*UUID offset of 2 bytes read part in UUID field*/
    }
    if (p_evt->Attribute_Data_Length == 6) /* we are interested in the UUID is 16 bit.*/
    {
      idx = 4;
      uuid_size = 2;
      uuid_short_offset = 0;
    }
    UNUSED(idx);
    UNUSED(uuid_size);

    /* Loop on number of attribute value tuples */
    for (i = 0; i < numServ; i++)
    {
      ServiceStartHdl =  UNPACK_2_BYTE_PARAMETER(&p_evt->Attribute_Data_List[uuid_offset - 4]);
      ServiceEndHdl = UNPACK_2_BYTE_PARAMETER(&p_evt->Attribute_Data_List[uuid_offset - 2]);
      uuid = UNPACK_2_BYTE_PARAMETER(&p_evt->Attribute_Data_List[uuid_offset + uuid_short_offset]);
      LOG_INFO_APP("  %d/%d short UUID=0x%04X, handle [0x%04X - 0x%04X]",
                   i + 1, numServ, uuid, ServiceStartHdl,ServiceEndHdl);

      /* complete context fields */
      if ( (a_ClientContext[index].ALLServiceStartHdl == 0x0000) || (ServiceStartHdl < a_ClientContext[index].ALLServiceStartHdl) )
      {
        a_ClientContext[index].ALLServiceStartHdl = ServiceStartHdl;
      }
      if ( (a_ClientContext[index].ALLServiceEndHdl == 0x0000) || (ServiceEndHdl > a_ClientContext[index].ALLServiceEndHdl) )
      {
        a_ClientContext[index].ALLServiceEndHdl = ServiceEndHdl;
      }

      if (uuid == GAP_SERVICE_UUID)
      {
        a_ClientContext[index].GAPServiceStartHdl = ServiceStartHdl;
        a_ClientContext[index].GAPServiceEndHdl = ServiceEndHdl;

        LOG_INFO_APP(", GAP_SERVICE_UUID found\n");
      }
      else if (uuid == GENERIC_ATTRIBUTE_SERVICE_UUID)
      {
        a_ClientContext[index].GATTServiceStartHdl = ServiceStartHdl;
        a_ClientContext[index].GATTServiceEndHdl = ServiceEndHdl;

        LOG_INFO_APP(", GENERIC_ATTRIBUTE_SERVICE_UUID found\n");
      }
/* USER CODE BEGIN gatt_parse_services_1 */
      else if (uuid == EATT_SERVICE_UUID)
      {
        a_ClientContext[index].EATTServiceHdl = ServiceStartHdl;
        a_ClientContext[index].EATTServiceEndHdl = ServiceEndHdl;

        APP_DBG_MSG(", EATT_SERVICE_UUID found\n");
      }
/* USER CODE END gatt_parse_services_1 */
      else
      {
        LOG_INFO_APP("\n");
      }

      uuid_offset += p_evt->Attribute_Data_Length;
    }
  }
  else
  {
    LOG_INFO_APP("ACI_ATT_READ_BY_GROUP_TYPE_RESP_VSEVT_CODE, failed no free index in connection table !\n");
  }

  return;
}

/**
* function of GATT service parse by UUID
*/
static void gatt_parse_services_by_UUID(aci_att_find_by_type_value_resp_event_rp0 *p_evt)
{
  uint8_t i;

  LOG_INFO_APP("ACI_ATT_FIND_BY_TYPE_VALUE_RESP_VSEVT_CODE - ConnHdl=0x%04X, Num_of_Handle_Pair=%d\n",
                p_evt->Connection_Handle,
                p_evt->Num_of_Handle_Pair);

  for(i = 0 ; i < p_evt->Num_of_Handle_Pair ; i++)
  {
    LOG_INFO_APP("ACI_ATT_FIND_BY_TYPE_VALUE_RESP_VSEVT_CODE - PaitId=%d Found_Attribute_Handle=0x%04X, Group_End_Handle=0x%04X\n",
                  i,
                  p_evt->Attribute_Group_Handle_Pair[i].Found_Attribute_Handle,
                  p_evt->Attribute_Group_Handle_Pair[i].Group_End_Handle);
  }

/* USER CODE BEGIN gatt_parse_services_by_UUID_1 */

/* USER CODE END gatt_parse_services_by_UUID_1 */

  return;
}

/**
* function of GATT characteristics parse
*/
static void gatt_parse_chars(aci_att_read_by_type_resp_event_rp0 *p_evt)
{
  uint16_t uuid, CharStartHdl, CharValueHdl;
  uint8_t uuid_offset, uuid_size = 0U, uuid_short_offset = 0U;
  uint8_t i, idx, numHdlValuePair, index;
  uint8_t CharProperties;

  LOG_INFO_APP("ACI_ATT_READ_BY_TYPE_RESP_VSEVT_CODE - ConnHdl=0x%04X\n",
                p_evt->Connection_Handle);

  for (index = 0 ; index < BLE_CFG_CLT_MAX_NBR_CB ; index++)
  {
    if (a_ClientContext[index].connHdl == p_evt->Connection_Handle)
    {
      break;
    }
  }

  /* index < BLE_CFG_CLT_MAX_NBR_CB means connection handle identified */
  if (index < BLE_CFG_CLT_MAX_NBR_CB)
  {
    /* event data in Attribute_Data_List contains:
    * 2 bytes for start handle
    * 1 byte char properties
    * 2 bytes handle
    * 2 or 16 bytes data for UUID
    */

    /* Number of attribute value tuples */
    numHdlValuePair = p_evt->Data_Length / p_evt->Handle_Value_Pair_Length;

    uuid_offset = 5;           /* UUID offset in bytes in Attribute_Data_List */
    if (p_evt->Handle_Value_Pair_Length == 21) /* we are interested in  128 bit UUIDs */
    {
      idx = 17;                /* UUID index of 2 bytes read part in Attribute_Data_List */
      uuid_size = 16;          /* UUID size in bytes */
      uuid_short_offset = 12;  /* UUID offset of 2 bytes read part in UUID field */
    }
    if (p_evt->Handle_Value_Pair_Length == 7) /* we are interested in  16 bit UUIDs */
    {
      idx = 5;
      uuid_size = 2;
      uuid_short_offset = 0;
    }
    UNUSED(idx);
    UNUSED(uuid_size);

    p_evt->Data_Length -= 1;

    LOG_INFO_APP("  ConnHdl=0x%04X, number of value pair = %d\n", a_ClientContext[index].connHdl, numHdlValuePair);
    /* Loop on number of attribute value tuples */
    for (i = 0; i < numHdlValuePair; i++)
    {
      CharStartHdl = UNPACK_2_BYTE_PARAMETER(&p_evt->Handle_Value_Pair_Data[uuid_offset - 5]);
      CharProperties = p_evt->Handle_Value_Pair_Data[uuid_offset - 3];
      CharValueHdl = UNPACK_2_BYTE_PARAMETER(&p_evt->Handle_Value_Pair_Data[uuid_offset - 2]);
      uuid = UNPACK_2_BYTE_PARAMETER(&p_evt->Handle_Value_Pair_Data[uuid_offset + uuid_short_offset]);

      if ( (uuid != 0x0) && (CharProperties != 0x0) && (CharStartHdl != 0x0) && (CharValueHdl != 0) )
      {
        LOG_INFO_APP("    %d/%d short UUID=0x%04X, Properties=0x%04X, CharHandle [0x%04X - 0x%04X]",
                     i + 1, numHdlValuePair, uuid, CharProperties, CharStartHdl, CharValueHdl);

        if (uuid == DEVICE_NAME_UUID)
        {
          LOG_INFO_APP(", GAP DEVICE_NAME charac found\n");
        }
        else if (uuid == APPEARANCE_UUID)
        {
          LOG_INFO_APP(", GAP APPEARANCE charac found\n");
        }
        else if (uuid == SERVICE_CHANGED_CHARACTERISTIC_UUID)
        {
          a_ClientContext[index].ServiceChangedCharStartHdl = CharStartHdl;
          a_ClientContext[index].ServiceChangedCharValueHdl = CharValueHdl;
          LOG_INFO_APP(", GATT SERVICE_CHANGED_CHARACTERISTIC_UUID charac found\n");
        }
/* USER CODE BEGIN gatt_parse_chars_1 */
        else if (uuid == EATT_CHAR1_CHAR_UUID)
        {
          a_ClientContext[index].EATTChar1CharHdl = CharStartHdl;
          a_ClientContext[index].EATTChar1ValueHdl = CharValueHdl;
          LOG_INFO_APP(", GATT EATT_CHARACTERISTIC1_UUID charac found\n");
          LOG_INFO_APP(", a_ClientContext[index].EATTChar1ValueHdl 0x%x\n", a_ClientContext[index].EATTChar1ValueHdl);
        }
        else if (uuid == EATT_CHAR2_CHAR_UUID)
        {
          a_ClientContext[index].EATTChar2CharHdl = CharStartHdl;
          a_ClientContext[index].EATTChar2ValueHdl = CharValueHdl;
          LOG_INFO_APP(", GATT EATT_CHARACTERISTIC2_UUID charac found\n");
        }
/* USER CODE END gatt_parse_chars_1 */
        else
        {
          LOG_INFO_APP("\n");
        }

      }
      uuid_offset += p_evt->Handle_Value_Pair_Length;
    }
  }
  else
  {
    LOG_INFO_APP("ACI_ATT_READ_BY_TYPE_RESP_VSEVT_CODE, failed handle not found in connection table !\n");
  }

  return;
}
/**
* function of GATT descriptor parse
*/
static void gatt_parse_descs(aci_att_find_info_resp_event_rp0 *p_evt)
{
  uint16_t uuid, handle;
  uint8_t uuid_offset, uuid_size, uuid_short_offset, handle_uuid_pair_size;
  uint8_t i, numDesc, index;

  LOG_INFO_APP("ACI_ATT_FIND_INFO_RESP_VSEVT_CODE - ConnHdl=0x%04X\n",
              p_evt->Connection_Handle);

  for (index = 0 ; index < BLE_CFG_CLT_MAX_NBR_CB ; index++)
  {
    if (a_ClientContext[index].connHdl == p_evt->Connection_Handle)
    {
      break;
    }
  }

  /* index < BLE_CFG_CLT_MAX_NBR_CB means connection handle identified */
  if (index < BLE_CFG_CLT_MAX_NBR_CB)
  {
    /* event data in Attribute_Data_List contains:
    * 2 bytes handle
    * 2 or 16 bytes data for UUID
    */
    uuid_offset = 2;
    if (p_evt->Format == UUID_TYPE_16)
    {
      uuid_size = 2;
      uuid_short_offset = 0;
      handle_uuid_pair_size = 4;
    }
    else if (p_evt->Format == UUID_TYPE_128)
    {
      uuid_size = 16;
      uuid_short_offset = 12;
      handle_uuid_pair_size = 18;
    }
    else
    {
      return;
    }
    UNUSED(uuid_size);

    /* Number of handle uuid pairs */
    numDesc = (p_evt->Event_Data_Length) / handle_uuid_pair_size;

    for (i = 0; i < numDesc; i++)
    {
      handle = UNPACK_2_BYTE_PARAMETER(&p_evt->Handle_UUID_Pair[uuid_offset - 2]);
      uuid = UNPACK_2_BYTE_PARAMETER(&p_evt->Handle_UUID_Pair[uuid_offset + uuid_short_offset]);

      if (uuid == PRIMARY_SERVICE_UUID)
      {
        LOG_INFO_APP("PRIMARY_SERVICE_UUID=0x%04X handle=0x%04X\n", uuid, handle);
      }
      else if (uuid == CHARACTERISTIC_UUID)
      {
        /* reset UUID & handle */
        gattCharStartHdl = 0;
        gattCharValueHdl = 0;

        gattCharStartHdl = handle;
        LOG_INFO_APP("reset - UUID & handle - CHARACTERISTIC_UUID=0x%04X CharStartHandle=0x%04X\n", uuid, handle);
      }
      else if ( (uuid == CHAR_EXTENDED_PROPERTIES_DESCRIPTOR_UUID)
             || (uuid == CLIENT_CHAR_CONFIG_DESCRIPTOR_UUID) )
      {

        LOG_INFO_APP("Descriptor UUID=0x%04X, handle=0x%04X-0x%04X-0x%04X",
                      uuid,
                      gattCharStartHdl,
                      gattCharValueHdl,
                      handle);
        if (a_ClientContext[index].ServiceChangedCharValueHdl == gattCharValueHdl)
        {
          a_ClientContext[index].ServiceChangedCharDescHdl = handle;
          LOG_INFO_APP(", Service Changed found\n");
        }
/* USER CODE BEGIN gatt_parse_descs_1 */
        else if (a_ClientContext[index].EATTChar1ValueHdl == gattCharValueHdl)
        {
          a_ClientContext[index].EATTChar1DescHdl = handle;
          APP_DBG_MSG("Char1 found : char Desc UUID=0x%04X handle=0x%04X-0x%04X-0x%04X\n",
                       uuid, gattCharStartHdl, gattCharValueHdl, handle);
        }
        else if (a_ClientContext[index].EATTChar2ValueHdl == gattCharValueHdl)
        {
          a_ClientContext[index].EATTChar2DescHdl = handle;
          APP_DBG_MSG("Char2 found : char Desc UUID=0x%04X handle=0x%04X-0x%04X-0x%04X\n",
                       uuid, gattCharStartHdl, gattCharValueHdl, handle);
        }
/* USER CODE END gatt_parse_descs_1 */
        else
        {
          LOG_INFO_APP("\n");
        }
        UNUSED(gattCharStartHdl);
      }
      else
      {
        gattCharValueHdl = handle;

        LOG_INFO_APP("  UUID=0x%04X, handle=0x%04X", uuid, handle);

        if (uuid == DEVICE_NAME_UUID)
        {
          LOG_INFO_APP(", found GAP DEVICE_NAME_UUID\n");
        }
        else if (uuid == APPEARANCE_UUID)
        {
          LOG_INFO_APP(", found GAP APPEARANCE_UUID\n");
        }
        else if (uuid == SERVICE_CHANGED_CHARACTERISTIC_UUID)
        {
          LOG_INFO_APP(", found GATT SERVICE_CHANGED_CHARACTERISTIC_UUID\n");
        }
/* USER CODE BEGIN gatt_parse_descs_2 */

/* USER CODE END gatt_parse_descs_2 */
        else
        {
          LOG_INFO_APP("\n");
        }
      }
    uuid_offset += handle_uuid_pair_size;
    }
  }
  else
  {
    LOG_INFO_APP("ACI_ATT_FIND_INFO_RESP_VSEVT_CODE, failed handle not found in connection table !\n");
  }

  return;
}

static void gatt_parse_notification(aci_gatt_notification_event_rp0 *p_evt)
{
  LOG_DEBUG_APP("ACI_GATT_NOTIFICATION_VSEVT_CODE - ConnHdl=0x%04X, Attribute_Handle=0x%04X\n",
                p_evt->Connection_Handle,
                p_evt->Attribute_Handle);
/* USER CODE BEGIN gatt_parse_notification_1 */
  uint8_t CRC_Result;
  uint8_t CRC_Received;
  a_ClientContext[0].MTUSizeValue = 241;
  
  if ((p_evt->Attribute_Handle == a_ClientContext[0].EATTChar1ValueHdl) && 
      (p_evt->Attribute_Value_Length > (2)))
  {
    if (p_evt->Connection_Handle == 0xEA00)
    {
      NotificationData1.EvtOpcode = NOTIFICATION_BEARER1_SVC1_CHAR1_UPDATE_EVT;
      NotificationData1.DataTransfered.Length = p_evt->Attribute_Value_Length;
      NotificationData1.DataTransfered.pPayload = (p_evt->Attribute_Value);
      NotificationData1.DataTransfered.pPayload_n = *((uint32_t*) &(p_evt->Attribute_Value[0]));
      
      __disable_irq();
      if (NotificationData1.DataTransfered.pPayload_n >= (NotificationData1.DataTransfered.pPayload_n_1 + 2))
      {
        packet_lost +=
          ((NotificationData1.DataTransfered.pPayload_n - NotificationData1.DataTransfered.pPayload_n_1) - 1);
      }
      __enable_irq();
      /* CRC computation */
      CRC_Result = APP_BLE_ComputeCRC8((uint8_t*) (p_evt->Attribute_Value), (p_evt->Attribute_Value_Length) - 1);
      /* get low weight byte */
      CRC_Received = (uint8_t) (p_evt->Attribute_Value[a_ClientContext[0].MTUSizeValue - 1]);
      
      if (CRC_Received != CRC_Result)
      {
        dtc_n += 1;
        LOG_INFO_APP("** data error **  N= %d \r\n",(int32_t)dtc_n);
      }
      NotificationData1.DataTransfered.pPayload_n_1 = NotificationData1.DataTransfered.pPayload_n; 
      
      if (DataTransfered1 == 0)
      {
        UTIL_TIMER_Start(&(EATT_CLI_APP_Context.TimerDataThroughput_Update_Bearer1_Id));
      }
      DataTransfered1 += NotificationData1.DataTransfered.Length;
    }
    if (p_evt->Connection_Handle == 0xEA01)
    {
      NotificationData2.EvtOpcode = NOTIFICATION_BEARER1_SVC1_CHAR1_UPDATE_EVT;
      NotificationData2.DataTransfered.Length = p_evt->Attribute_Value_Length;
      NotificationData2.DataTransfered.pPayload = (p_evt->Attribute_Value);
      NotificationData2.DataTransfered.pPayload_n = *((uint32_t*) &(p_evt->Attribute_Value[0]));
      
      __disable_irq();
      if (NotificationData2.DataTransfered.pPayload_n >= (NotificationData2.DataTransfered.pPayload_n_1 + 2))
      {
        packet_lost +=
          ((NotificationData2.DataTransfered.pPayload_n - NotificationData2.DataTransfered.pPayload_n_1) - 1);
      }
      __enable_irq();
      /* CRC computation */
      CRC_Result = APP_BLE_ComputeCRC8((uint8_t*) (p_evt->Attribute_Value), (p_evt->Attribute_Value_Length) - 1);
      /* get low weight byte */
      CRC_Received = (uint8_t) (p_evt->Attribute_Value[a_ClientContext[0].MTUSizeValue - 1]);
      
      if (CRC_Received != CRC_Result)
      {
        dtc_n += 1;
        LOG_INFO_APP("** data error **  N= %d \r\n",(int32_t)dtc_n);
      }
      NotificationData2.DataTransfered.pPayload_n_1 = NotificationData2.DataTransfered.pPayload_n;

      if (DataTransfered2 == 0)
      {
        UTIL_TIMER_Start(&(EATT_CLI_APP_Context.TimerDataThroughput_Update_Bearer2_Id));
      }
      DataTransfered2 += NotificationData2.DataTransfered.Length;
    }
    
  }
  
/* USER CODE END gatt_parse_notification_1 */

  return;
}

static void client_discover_all(void)
{
  uint8_t index = 0;
  /* USER CODE BEGIN client_discover_1 */

  /* USER CODE END client_discover_1 */

  GATT_CLIENT_APP_Discover_services(index);

  /* USER CODE BEGIN client_discover_2 */

  /* USER CODE END client_discover_2 */
  return;
}

static void gatt_cmd_resp_release(void)
{
  UTIL_SEQ_SetEvt(1U << CFG_IDLEEVT_PROC_GATT_COMPLETE);
  return;
}

static void gatt_cmd_resp_wait(void)
{
  UTIL_SEQ_WaitEvt(1U << CFG_IDLEEVT_PROC_GATT_COMPLETE);
  return;
}

/* USER CODE BEGIN LF */

static void SendDataWrite1( void )
{
  tBleStatus status = BLE_STATUS_INVALID_PARAMS;
  uint8_t crc_result;

  if( (EATTC_Context.ButtonTransferReq != EATTC_APP_TRANSFER_REQ_OFF) && 
      (EATTC_Context.DtFlowStatus != EATTC_APP_FLOW_OFF) )
  {   
    /*Data Packet to send to remote*/
    Notification_Data_Buffer1[0] += 1;
    a_ClientContext[0].MTUSizeValue = 241;
    /* compute CRC */
    crc_result = APP_BLE_ComputeCRC8((uint8_t*) Notification_Data_Buffer1, (a_ClientContext[0].MTUSizeValue - 1));
    Notification_Data_Buffer1[a_ClientContext[0].MTUSizeValue - 1] = crc_result;

    EATTC_Context.TxData.pPayload = Notification_Data_Buffer1;
    EATTC_Context.TxData.Length =  241; //a_ClientContext[0].MTUSizeValue;
    
    EATT_Bearer_connHdl = a_ClientContext[0].EATT_Bearer_connHdl[0];
    
    status = aci_gatt_write_without_resp( EATT_Bearer_connHdl,
                                         a_ClientContext[0].EATTChar1ValueHdl,
                                         241, //a_ClientContext[0].MTUSizeValue,
                                         (const uint8_t*)(EATTC_Context.TxData.pPayload));
    
    if (status == BLE_STATUS_INSUFFICIENT_RESOURCES)
    {
      EATTC_Context.DtFlowStatus = EATTC_APP_FLOW_OFF;
      (Notification_Data_Buffer1[0])-=1;
    }
    else
    {
      UTIL_SEQ_SetTask(1U << CFG_TASK_BLE_HOST, CFG_SEQ_PRIO_0);
      UTIL_SEQ_SetTask(1U << CFG_TASK_WRITE_DATA_BEARER1_ID, CFG_SEQ_PRIO_0);
    }
  }
  BleStackCB_Process();

  return;
}

static void SendDataWrite2( void )
{
  tBleStatus status = BLE_STATUS_INVALID_PARAMS;
  uint8_t crc_result;

  if( (EATTC_Context.ButtonTransferReq != EATTC_APP_TRANSFER_REQ_OFF) && 
      (EATTC_Context.DtFlowStatus != EATTC_APP_FLOW_OFF) )
  {   
    /*Data Packet to send to remote*/
    Notification_Data_Buffer2[0] += 1;
    a_ClientContext[0].MTUSizeValue = 241;
    /* compute CRC */
    crc_result = APP_BLE_ComputeCRC8((uint8_t*) Notification_Data_Buffer2, (a_ClientContext[0].MTUSizeValue - 1));
    Notification_Data_Buffer2[a_ClientContext[0].MTUSizeValue - 1] = crc_result;

    EATTC_Context.TxData.pPayload = Notification_Data_Buffer2;
    EATTC_Context.TxData.Length =  a_ClientContext[0].MTUSizeValue;
    
    EATT_Bearer_connHdl = a_ClientContext[0].EATT_Bearer_connHdl[1];

    status = aci_gatt_write_without_resp(EATT_Bearer_connHdl,
                                         a_ClientContext[0].EATTChar1ValueHdl,
                                         241, //a_ClientContext[0].MTUSizeValue,
                                         (const uint8_t*)(EATTC_Context.TxData.pPayload));

    if (status == BLE_STATUS_INSUFFICIENT_RESOURCES)
    {
      EATTC_Context.DtFlowStatus = EATTC_APP_FLOW_OFF;
      (Notification_Data_Buffer2[0])-=1;
    }
    else
    {
      UTIL_SEQ_SetTask(1U << CFG_TASK_BLE_HOST, CFG_SEQ_PRIO_0);
      UTIL_SEQ_SetTask(1U << CFG_TASK_WRITE_DATA_BEARER2_ID, CFG_SEQ_PRIO_0);
    }
  }
  BleStackCB_Process();

  return;
}

static void Resume_Write1(void)
{
  EATTC_Context.DtFlowStatus = EATTC_APP_FLOW_ON;
  UTIL_SEQ_SetTask(1U << CFG_TASK_WRITE_DATA_BEARER1_ID, CFG_SEQ_PRIO_0);
}

static void Resume_Write2(void)
{
  EATTC_Context.DtFlowStatus = EATTC_APP_FLOW_ON;
  UTIL_SEQ_SetTask(1U << CFG_TASK_WRITE_DATA_BEARER2_ID, CFG_SEQ_PRIO_0);
}


static void DataThroughputNotif_Bearer1_proc(void *arg)
{
  UTIL_SEQ_SetTask(1U << CFG_TASK_DATA_FROM_SERVER_BEARER1_ID, CFG_SEQ_PRIO_0);
}
                       
static void DataThroughputNotif_Bearer2_proc(void *arg)
{
  UTIL_SEQ_SetTask(1U << CFG_TASK_DATA_FROM_SERVER_BEARER2_ID, CFG_SEQ_PRIO_0);
}                       
                       

static void DataT_Notification_Data_Bearer1( void )
{
  DataThroughputValue1 = (uint32_t)(DataTransfered1 / TIMEUNIT);
  packet_lost_local = (uint32_t)(packet_lost / TIMEUNIT);
  DataTransfered1 = 0;
  packet_lost = 0;

  LOG_INFO_APP("  DataThroughput1 = %ld bytes/s lost = %ld \n",DataThroughputValue1, packet_lost_local);
  return;
}
                       
static void DataT_Notification_Data_Bearer2( void )
{
  DataThroughputValue2 = (uint32_t)(DataTransfered2 / TIMEUNIT);
  packet_lost_local = (uint32_t)(packet_lost / TIMEUNIT);
  DataTransfered2 = 0;
  packet_lost = 0;

  LOG_INFO_APP("  DataThroughput2 = %ld bytes/s lost = %ld \n",DataThroughputValue2, packet_lost_local);
  return;
}                       

#if defined(__GNUC__) && !defined (__clang__)
uint8_t __attribute__((optimize("Os"))) APP_BLE_ComputeCRC8( uint8_t *DataPtr , uint8_t Datalen )
#else
uint8_t APP_BLE_ComputeCRC8( uint8_t *DataPtr , uint8_t Datalen )
#endif
{
  uint8_t i, j;
  const uint8_t PolynomeCRC = 0x97;
  uint8_t CRC8 = 0x00;

  for (i = 0; i < Datalen; i++)
  {
    CRC8 ^= DataPtr[i];
    for (j = 0; j < 8; j++)
    {
      if ((CRC8 & 0x80) != 0)
      {
        CRC8 = (uint8_t) ((CRC8 << 1) ^ PolynomeCRC);
      }
      else
      {
        CRC8 <<= 1;
      }
    }
  }
  return (CRC8);
}

#if (CFG_BUTTON_SUPPORTED == 1)
void APPE_Button1Action(void)
{
  if (BleCoC_EATTContextCentral.connectionstatus != APP_BLE_CONNECTED_CLIENT)
  {
    UTIL_SEQ_SetTask(1U << CFG_TASK_START_SCAN_ID, CFG_SEQ_PRIO_0);
  }
  else
  {
    if(EATTC_Context.ButtonTransferReq != EATTC_APP_TRANSFER_REQ_OFF)
    {
      BSP_LED_Off(LED_BLUE);
      EATTC_Context.ButtonTransferReq = EATTC_APP_TRANSFER_REQ_OFF;
    }
    else
    {
      BSP_LED_On(LED_BLUE);
      EATTC_Context.ButtonTransferReq = EATTC_APP_TRANSFER_REQ_ON;
      UTIL_SEQ_SetTask(1U << CFG_TASK_WRITE_DATA_BEARER1_ID, CFG_SEQ_PRIO_0);
    }
  }
  BleStackCB_Process();
  return;
}
void APPE_Button2Action(void)
{
  tBleStatus status;
  if (BleCoC_EATTContextCentral.connectionstatus != APP_BLE_CONNECTED_CLIENT)
  {
    status = aci_gap_clear_security_db();
    if (status != BLE_STATUS_SUCCESS)
    {
      LOG_INFO_APP("Clear security DB cmd failure: 0x%02X\n", status);
    }
    else
    {
      LOG_INFO_APP("Clear security DB cmd success\n");
    }
  }
  else
  {

  }
  return;
}

void APPE_Button3Action(void)
{

  return;
}
#endif
/* USER CODE END LF */
