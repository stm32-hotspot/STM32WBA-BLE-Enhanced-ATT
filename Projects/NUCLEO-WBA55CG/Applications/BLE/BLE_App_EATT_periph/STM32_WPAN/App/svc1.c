/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service1.c
  * @author  MCD Application Team
  * @brief   service1 definition.
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
#include "log_module.h"
#include "common_blesvc.h"
#include "svc1.h"

/* USER CODE BEGIN Includes */
#include "app_ble.h"
#include "svc1_app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

typedef struct{
  uint16_t  Svc1SvcHdle;                  /**< Svc1 Service Handle */
  uint16_t  Char1CharHdle;                  /**< CHAR1 Characteristic Handle */
  uint16_t  Char2CharHdle;                  /**< CHAR2 Characteristic Handle */
/* USER CODE BEGIN Context */
  /* Place holder for Characteristic Descriptors Handle*/

/* USER CODE END Context */
}SVC1_Context_t;

/* Private defines -----------------------------------------------------------*/
#define UUID_128_SUPPORTED  1

#if (UUID_128_SUPPORTED == 1)
#define BM_UUID_LENGTH  UUID_TYPE_128
#else
#define BM_UUID_LENGTH  UUID_TYPE_16
#endif

#define BM_REQ_CHAR_SIZE    (3)

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */
extern uint16_t EATT_Bearer1_connHdl;
extern uint16_t EATT_Bearer2_connHdl;
uint16_t EATT_Bearer_connHdl;
/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
#define CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET         2
#define CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET              1
/* USER CODE BEGIN PM */
static uint8_t g_notification1_n_1;
static uint8_t g_notification2_n_1;

uint16_t packet_lost = 0;
uint32_t DTS_N=0;
uint16_t MTUSizeValue;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static const uint16_t SizeChar1 = 255;
static const uint16_t SizeChar2 = 255;

static SVC1_Context_t SVC1_Context;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static SVCCTL_EvtAckStatus_t SVC1_EventHandler(void *p_pckt);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
/* USER CODE BEGIN PFD */

/* USER CODE END PFD */

/* Private functions ----------------------------------------------------------*/

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
    uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
    uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
    uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* USER CODE BEGIN PF */

/* USER CODE END PF */

/**
 * @brief  Event handler
 * @param  p_Event: Address of the buffer holding the p_Event
 * @retval Ack: Return whether the p_Event has been managed or not
 */
static SVCCTL_EvtAckStatus_t SVC1_EventHandler(void *p_Event)
{
  SVCCTL_EvtAckStatus_t return_value;
  hci_event_pckt *p_event_pckt;
  evt_blecore_aci *p_blecore_evt;
  aci_gatt_attribute_modified_event_rp0 *p_attribute_modified;
  SVC1_NotificationEvt_t                 notification;
  /* USER CODE BEGIN Service1_EventHandler_1 */
  SVC1_NotificationEvt_t                 notification1;
  SVC1_NotificationEvt_t                 notification2;
  notification1.DataTransfered.pPayload_n_1 = g_notification1_n_1;
  notification2.DataTransfered.pPayload_n_1 = g_notification2_n_1;
  uint8_t CRC_Result;
  uint8_t CRC_Received;
  /* USER CODE END Service1_EventHandler_1 */

  return_value = SVCCTL_EvtNotAck;
  p_event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)p_Event)->data);

  switch(p_event_pckt->evt)
  {
    case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:
      p_blecore_evt = (evt_blecore_aci*)p_event_pckt->data;
      switch(p_blecore_evt->ecode)
      {
        case ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE:
        {
          /* USER CODE BEGIN EVT_BLUE_GATT_ATTRIBUTE_MODIFIED_BEGIN */

          /* USER CODE END EVT_BLUE_GATT_ATTRIBUTE_MODIFIED_BEGIN */
          p_attribute_modified = (aci_gatt_attribute_modified_event_rp0*)p_blecore_evt->data;
          notification.ConnectionHandle         = p_attribute_modified->Connection_Handle;
          notification.AttributeHandle          = p_attribute_modified->Attr_Handle;
          notification.DataTransfered.Length    = p_attribute_modified->Attr_Data_Length;
          notification.DataTransfered.p_Payload = p_attribute_modified->Attr_Data;
          if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char1CharHdle + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))
          {
            return_value = SVCCTL_EvtAckFlowEnable;
            /* USER CODE BEGIN Service1_Char_1 */

            /* USER CODE END Service1_Char_1 */
            switch(p_attribute_modified->Attr_Data[0])
            {
              /* USER CODE BEGIN Service1_Char_1_attribute_modified */

              /* USER CODE END Service1_Char_1_attribute_modified */

              /* Disabled Notification management */
              case (!(COMSVC_Notification)):
                /* USER CODE BEGIN Service1_Char_1_Disabled_BEGIN */

                /* USER CODE END Service1_Char_1_Disabled_BEGIN */
                notification.EvtOpcode = SVC1_CHAR1_NOTIFY_DISABLED_EVT;
                SVC1_Notification(&notification);
                /* USER CODE BEGIN Service1_Char_1_Disabled_END */

                /* USER CODE END Service1_Char_1_Disabled_END */
                break;

              /* Enabled Notification management */
              case COMSVC_Notification:
                /* USER CODE BEGIN Service1_Char_1_COMSVC_Notification_BEGIN */

                /* USER CODE END Service1_Char_1_COMSVC_Notification_BEGIN */
                notification.EvtOpcode = SVC1_CHAR1_NOTIFY_ENABLED_EVT;
                SVC1_Notification(&notification);
                /* USER CODE BEGIN Service1_Char_1_COMSVC_Notification_END */
                LOG_INFO_APP("Svc1 Char1 notification enabled \r\n");
                /* USER CODE END Service1_Char_1_COMSVC_Notification_END */
                break;

              default:
                /* USER CODE BEGIN Service1_Char_1_default */
                
                /* USER CODE END Service1_Char_1_default */
                break;
            }
          }  /* if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char1CharHdle + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))*/

          else if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char2CharHdle + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))
          {
            return_value = SVCCTL_EvtAckFlowEnable;
            /* USER CODE BEGIN Service1_Char_2 */

            /* USER CODE END Service1_Char_2 */
            switch(p_attribute_modified->Attr_Data[0])
            {
              /* USER CODE BEGIN Service1_Char_2_attribute_modified */

              /* USER CODE END Service1_Char_2_attribute_modified */

              /* Disabled Notification management */
              case (!(COMSVC_Notification)):
                /* USER CODE BEGIN Service1_Char_2_Disabled_BEGIN */

                /* USER CODE END Service1_Char_2_Disabled_BEGIN */
                notification.EvtOpcode = SVC1_CHAR2_NOTIFY_DISABLED_EVT;
                SVC1_Notification(&notification);
                /* USER CODE BEGIN Service1_Char_2_Disabled_END */

                /* USER CODE END Service1_Char_2_Disabled_END */
                break;

              /* Enabled Notification management */
              case COMSVC_Notification:
                /* USER CODE BEGIN Service1_Char_2_COMSVC_Notification_BEGIN */

                /* USER CODE END Service1_Char_2_COMSVC_Notification_BEGIN */
                notification.EvtOpcode = SVC1_CHAR2_NOTIFY_ENABLED_EVT;
                SVC1_Notification(&notification);
                /* USER CODE BEGIN Service1_Char_2_COMSVC_Notification_END */
                LOG_INFO_APP("Svc1 Char2 notification enabled \r\n");
                /* USER CODE END Service1_Char_2_COMSVC_Notification_END */
                break;

              default:
                /* USER CODE BEGIN Service1_Char_2_default */

                /* USER CODE END Service1_Char_2_default */
                break;
            }
          }  /* if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char2CharHdle + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))*/

          else if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char1CharHdle + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))
          {
            return_value = SVCCTL_EvtAckFlowEnable;

            notification.EvtOpcode = SVC1_CHAR1_WRITE_NO_RESP_EVT;
            /* USER CODE BEGIN Service1_Char_1_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */
            if (notification.ConnectionHandle == 0xEA20)
            {
              notification1.EvtOpcode = BEARER1_SVC1_CHAR1_WRITE_NO_RESP_EVT;
              notification1.DataTransfered.Length = p_attribute_modified->Attr_Data_Length; 
              notification1.DataTransfered.p_Payload = (p_attribute_modified->Attr_Data);
              notification1.DataTransfered.pPayload_n = *((uint32_t*) &(p_attribute_modified->Attr_Data[0]));
              
              if ((notification1.DataTransfered.pPayload_n - notification1.DataTransfered.pPayload_n_1) != 0)
              {
                __disable_irq();
                if (notification1.DataTransfered.pPayload_n >= (notification1.DataTransfered.pPayload_n_1 + 2))
                {
                  packet_lost +=
                    ((notification1.DataTransfered.pPayload_n - notification1.DataTransfered.pPayload_n_1) - 1);
                  LOG_INFO_APP("** Missing packets [ %3d - %3d ]  %3d \r\n",(uint32_t)notification1.DataTransfered.pPayload_n_1 + 1, (uint32_t)notification1.DataTransfered.pPayload_n - 1, (uint32_t)notification1.DataTransfered.pPayload_n - (uint32_t)notification1.DataTransfered.pPayload_n_1 - 1);
                  
                }
                __enable_irq();
                /* CRC computation */
                CRC_Result = APP_BLE_ComputeCRC8((uint8_t*) (p_attribute_modified->Attr_Data), (p_attribute_modified->Attr_Data_Length) - 1);
                /* get low weight byte */
                CRC_Received = (uint8_t) (p_attribute_modified->Attr_Data[notification.DataTransfered.Length - 1]);
                
                if (CRC_Received != CRC_Result)
                {
                  DTS_N+=1;
                  LOG_INFO_APP("** data error **  N= %d \r\n",(int)DTS_N);
                }
              }
              notification1.DataTransfered.pPayload_n_1 = notification1.DataTransfered.pPayload_n;
              g_notification1_n_1 = notification1.DataTransfered.pPayload_n;
              SVC1_Notification(&notification1);
            }
            if (notification.ConnectionHandle == 0xEA21)
            {
              notification2.EvtOpcode = BEARER2_SVC1_CHAR1_WRITE_NO_RESP_EVT;
              notification2.DataTransfered.Length = p_attribute_modified->Attr_Data_Length; 
              
              notification2.DataTransfered.p_Payload = (p_attribute_modified->Attr_Data);
              notification2.DataTransfered.pPayload_n = *((uint32_t*) &(p_attribute_modified->Attr_Data[0]));
              
              if ((notification2.DataTransfered.pPayload_n - notification2.DataTransfered.pPayload_n_1) != 0)
              {
                __disable_irq();
                if (notification2.DataTransfered.pPayload_n >= (notification2.DataTransfered.pPayload_n_1 + 2))
                {
                  packet_lost +=
                    ((notification2.DataTransfered.pPayload_n - notification2.DataTransfered.pPayload_n_1) - 1);
                  LOG_INFO_APP("** Missing packets [ %3d - %3d ]  %3d \r\n",(uint32_t)notification2.DataTransfered.pPayload_n_1 + 1, (uint32_t)notification2.DataTransfered.pPayload_n - 1, (uint32_t)notification2.DataTransfered.pPayload_n - (uint32_t)notification2.DataTransfered.pPayload_n_1 - 1);
                  
                }
                __enable_irq();
                /* CRC computation */
                CRC_Result = APP_BLE_ComputeCRC8((uint8_t*) (p_attribute_modified->Attr_Data), (p_attribute_modified->Attr_Data_Length) - 1);
                /* get low weight byte */
                CRC_Received = (uint8_t) (p_attribute_modified->Attr_Data[notification.DataTransfered.Length - 1]);
                
                if (CRC_Received != CRC_Result)
                {
                  DTS_N+=1;
                  LOG_INFO_APP("** data error **  N= %d \r\n",(int)DTS_N);
                }
              }
              notification2.DataTransfered.pPayload_n_1 = notification2.DataTransfered.pPayload_n;
              g_notification2_n_1 = notification2.DataTransfered.pPayload_n;
              SVC1_Notification(&notification2);
            }
            /* USER CODE END Service1_Char_1_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */
            SVC1_Notification(&notification);
          } /* if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char1CharHdle + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))*/
          else if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char2CharHdle + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))
          {
            return_value = SVCCTL_EvtAckFlowEnable;

            notification.EvtOpcode = SVC1_CHAR2_WRITE_NO_RESP_EVT;
            /* USER CODE BEGIN Service1_Char_2_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */

            /* USER CODE END Service1_Char_2_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */
            SVC1_Notification(&notification);
          } /* if(p_attribute_modified->Attr_Handle == (SVC1_Context.Char2CharHdle + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))*/

          /* USER CODE BEGIN EVT_BLUE_GATT_ATTRIBUTE_MODIFIED_END */

          /* USER CODE END EVT_BLUE_GATT_ATTRIBUTE_MODIFIED_END */
          break;/* ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */
        }
        case ACI_GATT_READ_PERMIT_REQ_VSEVT_CODE :
        {
          /* USER CODE BEGIN EVT_BLUE_GATT_READ_PERMIT_REQ_BEGIN */

          /* USER CODE END EVT_BLUE_GATT_READ_PERMIT_REQ_BEGIN */

          /* USER CODE BEGIN EVT_BLUE_GATT_READ_PERMIT_REQ_END */

          /* USER CODE END EVT_BLUE_GATT_READ_PERMIT_REQ_END */
          break;/* ACI_GATT_READ_PERMIT_REQ_VSEVT_CODE */
        }
        case ACI_GATT_WRITE_PERMIT_REQ_VSEVT_CODE:
        {
          /* USER CODE BEGIN EVT_BLUE_GATT_WRITE_PERMIT_REQ_BEGIN */

          /* USER CODE END EVT_BLUE_GATT_WRITE_PERMIT_REQ_BEGIN */

          /* USER CODE BEGIN EVT_BLUE_GATT_WRITE_PERMIT_REQ_END */

          /* USER CODE END EVT_BLUE_GATT_WRITE_PERMIT_REQ_END */
          break;/* ACI_GATT_WRITE_PERMIT_REQ_VSEVT_CODE */
        }
        case ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE:
        {
          aci_gatt_tx_pool_available_event_rp0 *p_tx_pool_available_event;
          p_tx_pool_available_event = (aci_gatt_tx_pool_available_event_rp0 *) p_blecore_evt->data;
          UNUSED(p_tx_pool_available_event);

          /* USER CODE BEGIN ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE */
          if (EATT_Bearer_connHdl == EATT_Bearer1_connHdl)
            Resume_Notification2();
          else
            Resume_Notification1();
          /* USER CODE END ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE */
          break;/* ACI_GATT_TX_POOL_AVAILABLE_VSEVT_CODE*/
        }
        case ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE:
        {
          aci_att_exchange_mtu_resp_event_rp0 *p_exchange_mtu;
          p_exchange_mtu = (aci_att_exchange_mtu_resp_event_rp0 *)  p_blecore_evt->data;
          UNUSED(p_exchange_mtu);

          /* USER CODE BEGIN ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE */
          LOG_INFO_APP(">>== MTU_size = %d\n",p_exchange_mtu->Server_RX_MTU );
          LOG_INFO_APP("\n");            
          LOG_INFO_APP("\n>>== CONNECTION READY\n");
          break;
          /* USER CODE END ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE */
          break;/* ACI_ATT_EXCHANGE_MTU_RESP_VSEVT_CODE */
        }
        /* USER CODE BEGIN BLECORE_EVT */

        /* USER CODE END BLECORE_EVT */
        default:
          /* USER CODE BEGIN EVT_DEFAULT */

          /* USER CODE END EVT_DEFAULT */
          break;
      }
      /* USER CODE BEGIN EVT_VENDOR*/

      /* USER CODE END EVT_VENDOR*/
      break; /* HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE */

      /* USER CODE BEGIN EVENT_PCKT_CASES*/

      /* USER CODE END EVENT_PCKT_CASES*/

    default:
      /* USER CODE BEGIN EVENT_PCKT*/

      /* USER CODE END EVENT_PCKT*/
      break;
  }

  /* USER CODE BEGIN Service1_EventHandler_2 */

  /* USER CODE END Service1_EventHandler_2 */

  return(return_value);
}/* end SVC1_EventHandler */

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void SVC1_Init(void)
{
  Char_UUID_t  uuid;
  tBleStatus ret;
  uint8_t max_attr_record;

  /* USER CODE BEGIN SVCCTL_InitService1Svc_1 */

  /* USER CODE END SVCCTL_InitService1Svc_1 */

  /**
   *  Register the event handler to the BLE controller
   */
  SVCCTL_RegisterSvcHandler(SVC1_EventHandler);

  /**
   * SVC1
   *
   * Max_Attribute_Records = 1 + 2*2 + 1*no_of_char_with_notify_or_indicate_property + 1*no_of_char_with_broadcast_property
   * service_max_attribute_record = 1 for SVC1 +
   *                                2 for CHAR1 +
   *                                2 for CHAR2 +
   *                                1 for CHAR1 configuration descriptor +
   *                                1 for CHAR2 configuration descriptor +
   *                              = 7
   * This value doesn't take into account number of descriptors manually added
   * In case of descriptors added, please update the max_attr_record value accordingly in the next SVCCTL_InitService User Section
   */
  max_attr_record = 7;

  /* USER CODE BEGIN SVCCTL_InitService */
  /* max_attr_record to be updated if descriptors have been added */

  /* USER CODE END SVCCTL_InitService */

  uuid.Char_UUID_16 = 0xaaaa;
  ret = aci_gatt_add_service(UUID_TYPE_16,
                             (Service_UUID_t *) &uuid,
                             PRIMARY_SERVICE,
                             max_attr_record,
                             &(SVC1_Context.Svc1SvcHdle));
  if (ret != BLE_STATUS_SUCCESS)
  {
    LOG_INFO_APP("  Fail   : aci_gatt_add_service command: SVC1, error code: 0x%x \n\r", ret);
  }
  else
  {
    LOG_INFO_APP("  Success: aci_gatt_add_service command: SVC1 \n\r");
  }

  /**
   * CHAR1
   */
  uuid.Char_UUID_16 = 0x1111;
  ret = aci_gatt_add_char(SVC1_Context.Svc1SvcHdle,
                          UUID_TYPE_16,
                          (Char_UUID_t *) &uuid,
                          SizeChar1,
                          CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY,
                          ATTR_PERMISSION_NONE,
                          GATT_NOTIFY_ATTRIBUTE_WRITE,
                          0x10,
                          CHAR_VALUE_LEN_VARIABLE,
                          &(SVC1_Context.Char1CharHdle));
  if (ret != BLE_STATUS_SUCCESS)
  {
    LOG_INFO_APP("  Fail   : aci_gatt_add_char command   : CHAR1, error code: 0x%2X\n", ret);
  }
  else
  {
    LOG_INFO_APP("  Success: aci_gatt_add_char command   : CHAR1\n");
  }

  /* USER CODE BEGIN SVCCTL_InitService1Char1 */
  /* Place holder for Characteristic Descriptors */

  /* USER CODE END SVCCTL_InitService1Char1 */

  /**
   * CHAR2
   */
  uuid.Char_UUID_16 = 0x2222;
  ret = aci_gatt_add_char(SVC1_Context.Svc1SvcHdle,
                          UUID_TYPE_16,
                          (Char_UUID_t *) &uuid,
                          SizeChar2,
                          CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE | CHAR_PROP_NOTIFY,
                          ATTR_PERMISSION_NONE,
                          GATT_NOTIFY_ATTRIBUTE_WRITE,
                          0x10,
                          CHAR_VALUE_LEN_VARIABLE,
                          &(SVC1_Context.Char2CharHdle));
  if (ret != BLE_STATUS_SUCCESS)
  {
    LOG_INFO_APP("  Fail   : aci_gatt_add_char command   : CHAR2, error code: 0x%2X\n", ret);
  }
  else
  {
    LOG_INFO_APP("  Success: aci_gatt_add_char command   : CHAR2\n");
  }

  /* USER CODE BEGIN SVCCTL_InitService1Char2 */
  /* Place holder for Characteristic Descriptors */

  /* USER CODE END SVCCTL_InitService1Char2 */

  /* USER CODE BEGIN SVCCTL_InitService1Svc_2 */

  /* USER CODE END SVCCTL_InitService1Svc_2 */

  return;
}

/**
 * @brief  Characteristic update
 * @param  CharOpcode: Characteristic identifier
 * @param  pData: Structure holding data to update
 *
 */
tBleStatus SVC1_UpdateValue(SVC1_CharOpcode_t CharOpcode, SVC1_Data_t *pData)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  /* USER CODE BEGIN Service1_App_Update_Char_1 */

  /* USER CODE END Service1_App_Update_Char_1 */

  switch(CharOpcode)
  {
    case SVC1_CHAR1:
      ret = aci_gatt_update_char_value(SVC1_Context.Svc1SvcHdle,
                                       SVC1_Context.Char1CharHdle,
                                       0, /* charValOffset */
                                       pData->Length, /* charValueLen */
                                       (uint8_t *)pData->p_Payload);
      if (ret != BLE_STATUS_SUCCESS)
      {
        LOG_INFO_APP("  Fail   : aci_gatt_update_char_value CHAR1 command, error code: 0x%2X\n", ret);
      }
      else
      {
        LOG_INFO_APP("  Success: aci_gatt_update_char_value CHAR1 command\n");
      }
      /* USER CODE BEGIN Service1_Char_Value_1*/

      /* USER CODE END Service1_Char_Value_1*/
      break;

    case SVC1_CHAR2:
      ret = aci_gatt_update_char_value(SVC1_Context.Svc1SvcHdle,
                                       SVC1_Context.Char2CharHdle,
                                       0, /* charValOffset */
                                       pData->Length, /* charValueLen */
                                       (uint8_t *)pData->p_Payload);
      if (ret != BLE_STATUS_SUCCESS)
      {
        LOG_INFO_APP("  Fail   : aci_gatt_update_char_value CHAR2 command, error code: 0x%2X\n", ret);
      }
      else
      {
        LOG_INFO_APP("  Success: aci_gatt_update_char_value CHAR2 command\n");
      }
      /* USER CODE BEGIN Service1_Char_Value_2*/

      /* USER CODE END Service1_Char_Value_2*/
      break;

    default:
      break;
  }

  /* USER CODE BEGIN Service1_App_Update_Char_2 */

  /* USER CODE END Service1_App_Update_Char_2 */

  return ret;
}
/**
 * @brief  Characteristic update per connection handle to notify
 * @param  CharOpcode: Characteristic identifier
 * @param  pData: Structure holding data to update
 * @param  EATT_Channel: Channel for EATT
 *
 */
tBleStatus SVC1_UpdateValue_Ext(SVC1_CharOpcode_t CharOpcode, SVC1_Data_t *pData, uint16_t EATT_Channel)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  uint8_t update_type = 0x01;
  /* USER CODE BEGIN Service1_App_Update_Char_Ext_1 */

  /* USER CODE END Service1_App_Update_Char_Ext_1 */

  switch(CharOpcode)
  {
    case SVC1_CHAR1:
      ret = aci_gatt_update_char_value_ext(EATT_Channel,
										SVC1_Context.Svc1SvcHdle,
										SVC1_Context.Char1CharHdle,
										update_type,
										pData->Length, /* charValueLen */
										0, /* charValOffset */
										pData->Length, /* charValueLen */
										(uint8_t *)pData->p_Payload);
      if (ret != BLE_STATUS_SUCCESS)
      {
        LOG_DEBUG_APP("  Fail   : aci_gatt_update_char_value_ext CHAR1 command, error code: 0x%2X\n", ret);
      }
      else
      {
        LOG_DEBUG_APP("  Success: aci_gatt_update_char_value_ext CHAR1 command\n");
      }
      /* USER CODE BEGIN Service1_Char_Value_Ext1*/

      /* USER CODE END Service1_Char_Value_Ext1*/
      break;

    case SVC1_CHAR2:
      ret = aci_gatt_update_char_value_ext(EATT_Channel,
										SVC1_Context.Svc1SvcHdle,
										SVC1_Context.Char2CharHdle,
										update_type,
										pData->Length, /* charValueLen */
										0, /* charValOffset */
										pData->Length, /* charValueLen */
										(uint8_t *)pData->p_Payload);
      if (ret != BLE_STATUS_SUCCESS)
      {
        LOG_DEBUG_APP("  Fail   : aci_gatt_update_char_value_ext CHAR2 command, error code: 0x%2X\n", ret);
      }
      else
      {
        LOG_DEBUG_APP("  Success: aci_gatt_update_char_value_ext CHAR2 command\n");
      }
      /* USER CODE BEGIN Service1_Char_Value_Ext2*/

      /* USER CODE END Service1_Char_Value_Ext2*/
      break;

    default:
      break;
  }

  /* USER CODE BEGIN Service1_App_Update_Char_Ext_2 */

  /* USER CODE END Service1_App_Update_Char_Ext_2 */

  return ret;
}
