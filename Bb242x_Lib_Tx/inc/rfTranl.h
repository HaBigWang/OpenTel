/**
  ******************************************************************************
  * @file    stm3210b_eval.h
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file contains definitions for STM3210B_EVAL's Leds, push-buttons
  *          COM ports, SD Card (on SPI), sFLASH (on SPI) and Temperature sensor 
  *          LM75 (on I2C) hardware resources.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RFTRANL_H
#define __RFTRANL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"    
	
#define RF_FRAME_DATA_LENGTH  30
#define RF_PACKAGE_LENGTH     300

//RF ���ݰ�
//@brief ���ݷ����������˫������ơ�һ�����ݰ���ʾ���ڷ�������ݣ���һ��Ϊ׼��Ҫ���͵����ݡ�
typedef union {
	uint8_t sequence;                         //���ݷ��估����ʱ�����ݰ���ֺ�����к�
	uint8_t repeat;                           //����׼������ʱ���Ƿ��ظ������־
} RFPagStyle;	 
	 
typedef struct {
	uint8_t buf[RF_PACKAGE_LENGTH];           //���ݰ���ʵ�ʴ洢λ��
	uint16_t length;                          //���ݳ���(С�ڵ���RF_PACKAGE_LENGTH)
	uint16_t id;                              //���ݰ����
	RFPagStyle style;                         //��ͬ���ݰ���ʾ���岻ͬ
} RFPackage;

	 
#ifdef __cplusplus
}
#endif
  
#endif /* __RFTRANL_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/



