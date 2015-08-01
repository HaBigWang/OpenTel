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
#include "rfPackage.h"
#include "bk242x.h"
 
#define NULL (void*)0

#define TRUE  1
#define FALSE 0


/**@brief ���ݰ���֣���ֵ���仺������Ϊд����Ƶ�豸��׼��
  *@param
	*  1) *Dev���豸ָ�롣���������ݰ�����ض����豸�ڲ�
	*  
  *@return 
	*  1)TRUE(���ݴ�����) 
	*  2)FALSE(������)
  */
uint8_t RFPackageSplit(RFDevice *Dev)
{
	RFPackage* package = &Dev->packageT; 
	RFPackage* pacRes = &Dev->packageReserve;
	uint8_t* FIFO = Dev->TxBuffer; 	
	uint8_t *point = NULL;   
	uint16_t index, length;	
	
	//Step1 ���ݰ�Ϊ�ջ��߷�����ɣ����¸�ֵ
	if((package->length == 0) || (package->style.sequence*RF_FRAME_DATA_LENGTH >= package->length)){
		if((pacRes->status == newPackage) || pacRes->style.repeat){			
			for(index = 0; index < pacRes->length; index++) package->buf[index] = pacRes->buf[index];
			package->length = pacRes->length;
			package->id = pacRes->id;
			package->style.sequence = 0;		
			
			pacRes->status = copied;
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

/**@brief ���ݰ�ƴ��
  *  1) ��Ƶ�豸��ȡһ֡���ݺ���ô˺�������һ֡���֡����ƴ�ӳ�һ�����������ݰ�
  *
  *@param
  *  1) *Dev ��Ƶ�豸
  *  
  *@return 
  *  1) TRUE ��ȡһ����ɵ����ݰ���FALSE ��������ݰ�δ���
  *
  *@note ��ң���豸�ظ�����ͬһ�����ݰ�ʱ���˺���Ҳ���ظ�������ͬ�����ݰ���
  */
uint8_t RFPackageSplice(RFDevice *Dev) {
  RFPackage* package = &Dev->packageR;
	uint8_t* FIFO = Dev->RxBuffer;
	uint8_t i, length;
	
	//Case1 Sequence==0����ʼһ�������ݰ��Ľ���(���ܺ�֮ǰ�����ݰ���ͬ����ͬID)
	//  1) ֻҪsequence=0���������������ݰ����գ������֮ǰ״̬
	//  2) ��֡��Ƶ����Ҳ������һ�����������ݰ����������Ҫ���FIFO[1].7��־λ
	if((FIFO[0] & 0x0f) == 0){                       
		package->id = FIFO[0] & 0xf0;
		package->style.sequence = 0;		
		package->length = FIFO[1] & 0x1f;		
		for(i=0; i<package->length; i++) package->buf[i] = FIFO[i+2];
		
		if(FIFO[1] & 0x80){
			package->status = receive;
			return TRUE;
		} 
		else return FALSE;
	} 
	
	//Case2 ��֡���һ�����ݰ�
	//  1) ֻ�е�ǰ��Ƶ֡sequence����֮ǰ��1ʱ�Ÿ��¡�
	if(package->id == (FIFO[0] & 0xf0)){                    
		if((package->style.sequence + 1) == (FIFO[0]&0x0f)){  
			package->style.sequence++; 
			length = FIFO[1] & 0x1f;
			for(i=0; i<length; i++) package->buf[package->length+i] = FIFO[2+i];
			package->length += length;			
			
			if(FIFO[1] & 0x80){
			  package->status = receive;
			  return TRUE;
		  }
		}
	} 
	
	return FALSE;
}


/**@brief ������������д�뷢�仺����
  *  1) ���Ƕ���ӿں�����
  *  2) ������������ָ�봫�ݸ��ú��������ݼ��ᱻд�뷢�仺������
  *  3) ��Ƶ�豸��ɵ�ǰ�����󣬽��Զ�������ݷ��䡣
  *  
  *@param
  *  *Dev    ��Ƶ�豸
  *  *Data   ����������ָ��
  *  Length  ���������ݳ��ȡ����������ݰ���󳤶ȣ���ȡ���
  *  Repeat  =1���ظ����䣻=0��ֻ����һ��
  */
void RFSendPackage(RFDevice *Dev, uint8_t* Data, uint16_t Length, uint8_t Repeat) {
	RFPackage* pacRes = &Dev->packageReserve;
	uint16_t index; 
	
	if(Length > RF_PACKAGE_LENGTH) Length = RF_PACKAGE_LENGTH;	
	for(index=0; index<Length; index++) pacRes->buf[index] = Data[index];
	pacRes->length = Length;
	pacRes->id++;
	pacRes->style.repeat = Repeat;	
	pacRes->status = newPackage;
}


/**@brief ��⺯��
  *  1)ʵʱ���ã�������Ƶ�豸��
  *  2)ʵ�ַ��䣬��ɽ���
  */






