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
#include "button.h"
#include "adc.h"

#define BUTTON_SUM             10
#define ADC_CHANNEL_BUTTON_1   ADC_Channel_8
#define ADC_CHANNEL_BUTTON_2   ADC_Channel_9

typedef struct {
	uint8_t bPushOrgLast:1;
	uint8_t bPushOrg:1;
	uint8_t bPushLast:1;
	uint8_t bPush:1;
	uint8_t bPositive:1;
	uint8_t bNegative:1;
	
	uint8_t continueTimer;
} Button;

Button ButtonBuf[BUTTON_SUM];
Button *BtnLeftStickRollL = &ButtonBuf[0], *BtnLeftStickRollR = &ButtonBuf[1], *BtnLeftStickPitchU = &ButtonBuf[2], *BtnLeftStickPitchD = &ButtonBuf[3], \
       *BtnRightStickRollL = &ButtonBuf[4], *BtnRightStickRollR = &ButtonBuf[5], *BtnRightStickPitchU = &ButtonBuf[6], *BtnRightStickPitchD = &ButtonBuf[7], \
       *BtnLeftTop = &ButtonBuf[8], *BtnRightTop = &ButtonBuf[9];

uint8_t ButtonTimer;


/** @Brief ���豸
  *
  */
int8_t BtnOpen(void) {
	uint8_t i;
	Button* btn = ButtonBuf;
	
	for(i=0; i<BUTTON_SUM; i++){
		btn[i].bPush = 0;
		btn[i].bPushLast = 0;
		btn[i].bPushOrg = 0;
		btn[i].bPushOrgLast = 0;
		btn[i].bPositive = 0;
		btn[i].bNegative = 0;
		btn[i].continueTimer = 0;
	}
	
  return 0;
}

/** @Brief ����ɨ��
  *   1) 20msɨ��һ�Σ���������Ƶ��1/(20ms*2)=25Hz��
  *
 	*	@Return 
  *   1) btn.bPush     = 1,     ��������
  *                    = 0,     ��������
  *   2) btn.bNegative = 1,     �½��ط���
  *                    = 0,     �ɰ������º�������
  *   3) btn.bPositive = 1,     �����ط���
  *                    = 0,     �ɰ������º�������
  *   4) btn.continueTimer      ��ǰ״̬����ʱ�䣬���ʱ255*20ms = 5.1s
  *
  * @Note ˵��
	* CHANNELA 
	*            |-----------------------------------ADC_A
	*   VDD--10k---1k---1k--
	*            |    |    |
	*           |    |    |
	*            |    |    |
	*           GND  GND  GND	
  *            
  *
	* CHANNELB
	*            |-----------------------------------ADC_B
	*   VDD--10k---1k---1k---1k---1k---1k---1k--
	*            |    |    |    |    |    |    |
	*           |    |    |    |    |    |    |
	*            |    |    |    |    |    |    |
	*           GND  GND  GND  GND  GND  GND  GND	
  *
  *
	* ADC����ֵ(12λ)
  *   NULL  Fst   Sec   Thd   4     5     6     7     8     9
	*   1     0     1/11  2/12  3/13  4/14  5/15  6/16  7/17  8/18 
  *   1023  0     93    170   236   292   341   384   421   455           
  *	
  */
void BtnScan(void) {
	uint16_t adc1, adc2;
	uint8_t i;
	Button* btn = ButtonBuf;
	
	//1����ȡADC��ֵ
	adc1 = UserADCGetValue(ADC_CHANNEL_BUTTON_1);
	adc2 = UserADCGetValue(ADC_CHANNEL_BUTTON_2);
	
	//2) ����ADC��ֵ��ð���˲ʱ״̬
	for(i=0; i<BUTTON_SUM; i++) btn[i].bPushOrg = 0;
	while(1) {
	  if(adc1 < 46) {BtnLeftStickRollL->bPushOrg = 1; break;}
		if(adc1 < 132) {BtnLeftStickRollR->bPushOrg = 1; break;}
  }	
	while(1) {
	  if(adc2 < 46) {BtnLeftStickPitchU->bPushOrg = 1; break;}
		if(adc2 < 132) {BtnLeftStickPitchD->bPushOrg = 1; break;}
		if(adc2 < 203) {BtnRightStickRollL->bPushOrg = 1; break;}
		if(adc2 < 264) {BtnRightStickRollR->bPushOrg = 1; break;}
		if(adc2 < 317) {BtnRightStickPitchU->bPushOrg = 1; break;}
		if(adc2 < 362) {BtnRightStickPitchD->bPushOrg = 1; break;}
		if(adc2 < 401) {BtnLeftTop->bPushOrg = 1; break;}
		if(adc2 < 438) {BtnRightTop->bPushOrg = 1; break;}
  }
	
	//3) ����ȥ��: �������μ��ֵ��ͬ��������Ч��
	//   a ���°���״̬����ʱ��
	//   b �洢��ǰ����־λ
	for(i=0; i<BUTTON_SUM; i++) {
		if(btn[i].bPushOrg == btn[i].bPushOrgLast) {
			btn[i].bPush = btn[i].bPushOrg;
		} else {
			btn[i].bPush = btn[i].bPushLast;
		}
		
		if(btn[i].bPush == btn[i].bPushLast){
			btn[i].continueTimer++;
		} else {
			btn[i].continueTimer = 0;
			if(btn[i].bPush) btn[i].bNegative = 1;
			else btn[i].bPositive = 1;
		}
		
    btn[i].bPushOrgLast = btn[i].bPushOrg;
    btn[i].bPushLast = btn[i].bPush;		
	}	
	
}
 		
/** @Brief ��������
  *
  */
void ButtonUpdate(void) {
	if(ButtonTimer >= 20) {
		ButtonTimer = 0;
		
		//״̬����
		BtnScan();
		
		//���ܸ���		
		
		
	}
}






