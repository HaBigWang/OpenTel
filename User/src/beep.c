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
#include "beep.h"
#include "timer.h"

#define BEEP_SUM 2

Beep BeepBuf[BEEP_SUM];
Beep *Beep1, *Vibration;


/** @Brief ����������\��
  * @P 
  *   1) ����������ռ�ձȺͳ���ʱ��
  * @Note
  *   1) ����\�𶯵���һ��������ɺ���Ч
  *   2) ��֮ǰ�޶�����������Ч
  */
void BeepNew(Beep *Dev, uint16_t Tune, uint8_t Duty, uint16_t Dur){
	Dev->tune = Tune;
	Dev->duty = Duty;
	Dev->durationNew = Dur;	
	Dev->bReStart = 1;
}

/** @Brief ִ������\��
  *   1) д�����裬���ɿ��Ʋ���
  *   2) ���豸�ж��������������ڳ�ʼ��ʱ����
  */
void Beep1Start(uint16_t Tune, uint8_t Duty){
	Tim3Channel1Update(Tune, Duty);
}
void Beep2Start(uint16_t Tune, uint8_t Duty){
	Tim3Channel2Update(Tune, Duty);  	 
}

/** @Brief �ر�����\��
  */
void BeepStop(Beep *Dev){
	Dev->Start(Dev->tune, 0);
}


/** @Brief ��ʼ��
  *   1) �ϵ�󣬴��ײ�Ӳ����ز��ֳ�ʼ����ɺ����
  *   2) ����ָ�롢��ʼ������
  */
int8_t BeepOpen(void) {
	uint8_t i;
	
	Beep1 = &BeepBuf[0];
	Vibration = &BeepBuf[1];
	
	for(i=0; i<BEEP_SUM; i++) {
		BeepBuf[i].bRunning = 0;
		BeepBuf[i].bReStart = 0;
	}
	
	Beep1->Start = Beep1Start;
	Vibration->Start = Beep2Start;
		
	return 0;
}

/** @Brief ����\�����س�����ʵʱ����
  *   
  */
void BeepCtrl(Beep *Dev){
	if(Dev->bRunning){
		if(Dev->timer >= Dev->duration) {
			Dev->bRunning = 0;
			BeepStop(Dev); 
		}
	}
	
	if(!Dev->bRunning && Dev->bReStart){
		Dev->bReStart = 0;
		Dev->duration = Dev->durationNew;
		
		Dev->timer = 0;
		Dev->Start(Dev->tune, Dev->duty);
		Dev->bRunning = 1;
	}
}



 
