/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service1.h
  * @author  MCD Application Team
  * @brief   Header for service1.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SVC1_H
#define SVC1_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported defines ----------------------------------------------------------*/
/* USER CODE BEGIN ED */

/* USER CODE END ED */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  SVC1_CHAR1,
  SVC1_CHAR2,
  /* USER CODE BEGIN Service1_CharOpcode_t */

  /* USER CODE END Service1_CharOpcode_t */
  SVC1_CHAROPCODE_LAST
} SVC1_CharOpcode_t;

typedef enum
{
  SVC1_CHAR1_WRITE_NO_RESP_EVT,
  SVC1_CHAR1_WRITE_EVT,
  SVC1_CHAR1_NOTIFY_ENABLED_EVT,
  SVC1_CHAR1_NOTIFY_DISABLED_EVT,
  SVC1_CHAR2_WRITE_NO_RESP_EVT,
  SVC1_CHAR2_WRITE_EVT,
  SVC1_CHAR2_NOTIFY_ENABLED_EVT,
  SVC1_CHAR2_NOTIFY_DISABLED_EVT,
  /* USER CODE BEGIN Service1_OpcodeEvt_t */
  BEARER1_SVC1_CHAR1_WRITE_NO_RESP_EVT,
  BEARER2_SVC1_CHAR1_WRITE_NO_RESP_EVT,
  /* USER CODE END Service1_OpcodeEvt_t */
  SVC1_BOOT_REQUEST_EVT
} SVC1_OpcodeEvt_t;

typedef struct
{
  uint8_t *p_Payload;
  uint8_t Length;

  /* USER CODE BEGIN Service1_Data_t */
  uint8_t pPayload_n_1;
  uint8_t pPayload_n;
  /* USER CODE END Service1_Data_t */
} SVC1_Data_t;

typedef struct
{
  SVC1_OpcodeEvt_t       EvtOpcode;
  SVC1_Data_t             DataTransfered;
  uint16_t                ConnectionHandle;
  uint16_t                AttributeHandle;
  uint8_t                 ServiceInstance;
  /* USER CODE BEGIN Service1_NotificationEvt_t */

  /* USER CODE END Service1_NotificationEvt_t */
} SVC1_NotificationEvt_t;

/* USER CODE BEGIN ET */
typedef struct
{
  uint8_t *pPayload;
  uint32_t pPayload_n_1;
  uint32_t pPayload_n;
  uint32_t Length;
} DTS_STM_Payload_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void SVC1_Init(void);
void SVC1_Notification(SVC1_NotificationEvt_t *p_Notification);
tBleStatus SVC1_UpdateValue(SVC1_CharOpcode_t CharOpcode, SVC1_Data_t *pData);
tBleStatus SVC1_UpdateValue_Ext(SVC1_CharOpcode_t CharOpcode, SVC1_Data_t *pData, uint16_t EATT_Channel);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*SVC1_H */
