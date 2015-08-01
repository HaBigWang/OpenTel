/**
  ******************************************************************************
  * @file    stm3210b_eval.c
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file provides
  *            - set of firmware functions to manage Leds, push-button and COM ports
  *            - low level initialization functions for SD card (on SPI), SPI serial
  *              flash (sFLASH) and temperature sensor (LM75)
  *          available on STM3210B-EVAL evaluation board from STMicroelectronics.    
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
  
/* Includes ------------------------------------------------------------------*/
#include "rfTranl.h"
#include "bk242x.h"
 
#define NULL (void*)0

#define TRUE 1
#define FALSE 0


/**@brief ���ݰ���֣�Ȼ��ֵ���仺������Ϊд����Ƶ�豸��׼��
  *@param
	*  1) *Dev���豸ָ�롣���������ݰ�����ض����豸�ڲ�
	*  
  *@return 
	*  1)TRUE(���ݴ�����) 
	*  2)FALSE(������)
  */
uint8_t RFPackageSplit(RFDevice *Dev)
{
	RFPackage *package, *pacRes;
	uint8_t* FIFO; 
	
	uint8_t *point = NULL;   
	uint16_t index, length;
	
	
	//Step1 ���ݰ�Ϊ�ջ��߷�����ɣ����¸�ֵ
	if((package->length == 0) || (package->style.sequence*RF_FRAME_DATA_LENGTH >= package->length)){
		if((package->id != pacRes->id) || pacRes->style.repeat){			
			for(index = 0; index < pacRes->length; index++) package->buf[index] = pacRes->buf[index];
			package->length = pacRes->length;
			package->id = pacRes->id;
			package->style.sequence = 0;			
		} 
	}  
	
	//Step2 ��ȡ����������λ��	
	if((package->length != 0) && (package->style.sequence*RF_FRAME_DATA_LENGTH < package->length)){
		point = &package->buf[package->style.sequence * RF_FRAME_DATA_LENGTH];
	}
	
	//Step3 ���·��仺����
	//1) FIFO[0]    �����к�|����ֺ�������кţ���ռ��λ 
	//2) FIFO[1]    ��5λ=���ݳ���(0~30)�����λ=���ݰ��Ƿ����
	//3) FIFO[2~31] ����
	if(point != NULL){		
		length = package->length - package->style.sequence*RF_FRAME_DATA_LENGTH;
		if(length > RF_FRAME_DATA_LENGTH) {length = RF_FRAME_DATA_LENGTH; FIFO[1] = 0;}
		else {FIFO[1] = 0x80;}
		
		FIFO[0] = ((uint8_t)package->id << 4) | ((uint8_t)package->style.sequence & 0x0f); 
		FIFO[1] |= length;
		for(index=0; index<length; index++){
			FIFO[2+index] = point[index];
		}
		
		package->style.sequence++;                      
		
		return TRUE;
	}	
	
	return FALSE;
}

/**@brief ������������д�뷢�仺����
  *@Parm
  *  *Dev    ��Ƶ�豸
  *  Package ����������ָ��
  *  Length  ���������ݳ��ȡ����������ݰ���󳤶ȣ���ȡ���
  *  Repeat  =1���ظ����䣻=0��ֻ����һ��
  */
void RFSendPackage(RFDevice *Dev, uint8_t* Package, uint16_t Length, uint8_t Repeat) {
	RFPackage* pacRes;
	uint16_t index; 
	
	if(Length > RF_PACKAGE_LENGTH) Length = RF_PACKAGE_LENGTH;	
	for(index=0; index<Length; index++) pacRes->buf[index] = Package[index];
	pacRes->length = Length;
	pacRes->id++;
	pacRes->style.repeat = Repeat;	
}


/**@brief ���ݰ�ƴ��
  *
  */
uint8_t RFPackageSplice(uint8_t* FIFO) {
  RFPackage* package;
	uint8_t i, length;
	
	if(package->id == (FIFO[0] & 0xf0)){
		if((package->style.sequence+1) == (FIFO[0]&0x0f)) {
			package->style.sequence++; 
			length = FIFO[1] & 0x1f;
			for(i=0; i<length; i++) package->buf[package->length+i] = FIFO[2+i];
			package->length += length;			
			
			if(FIFO[1] & 0x80) {
				return TRUE;
			}
		}
	} else {
		package->id = FIFO[0] & 0xf0;
		package->style.sequence = FIFO[0] & 0x0f;
		
		length = FIFO[1] & 0x1f;
		package->length = length;
		for(i=0; i<length; i++) package->buf[i] = FIFO[i+2];
	}
	
	return FALSE;
}






