
/***************************************************************************************************
						Bk2423 ���Ӻ���
***************************************************************************************************/
#include "bk242x_sub.h"	 

/*================================================================================================================							
================================================================================================================*/
/**  �ⲿ����
  *  @ RF�����ڼ䣬����������ֱ��������ɲ��˳���
  *  @ �����ⲿ������Ҫ���ã���ָʾ�ơ��ɽ��ⲿ��������˴���
  *  @ ��RF��ʱ����Ĭ�����ж��е��������жϲ�����Ҫ����Ҫ�Զ����ʱ��������ô�������ҲӦ����˴���
  */
void RFBindCallFunction(void){
	//LedCtrl(make_pair);	
}
	  
/**
  *��ȡ��ƷΨһ���кţ���ֵ����
  */
void RFGetUinqueAddr(uint8_t *Addr)
{	   	
  Addr[0] = 1;
  Addr[1] = 2;
  Addr[2] = 3;
  Addr[3] = 4;
}
 
//================================================================================================================	
//						�Ӻ���
//================================================================================================================ 
//--------------------------------------------------------------
//				IRC 22M,��ȷ��������ʱ										  
//--------------------------------------------------------------
void RFDelay(uint16_t Count)
{
  /* Decrement nCount value */
  while (Count != 0)
  {
    Count--;
  }	
}
		
/* RF��д���� ----------------------------------------------------------------*/
/** @Brief RF��д����
  *
  */
//д�Ĵ���
void RFWriteReg(RFDevice *Dev, uint8_t Addr, uint8_t Data) {
	SPI_Write_Reg_2(BK_WRITE_REG|Addr, Data, Dev->CS_HIGH, Dev->CS_LOW); 
}

void RFWriteBuf(RFDevice *Dev, uint8_t Addr, uint8_t* Buf, uint8_t Length) {
  SPI_Write_Buf_2(BK_WRITE_REG|Addr, Buf, Length, Dev->CS_HIGH, Dev->CS_LOW);
}

//д����
void RFWriteCmd(RFDevice *Dev, uint8_t CMD, uint8_t ARG) {
	SPI_Write_Reg_2(CMD, ARG, Dev->CS_HIGH, Dev->CS_LOW);
}

//���Ĵ���
uint8_t RFReadReg(RFDevice *Dev, uint8_t Addr) {
  return SPI_Read_Reg_2(BK_READ_REG|Addr, Dev->CS_HIGH, Dev->CS_LOW);
}

//�շ��仺����
void RFWritePayload(RFDevice *Dev, uint8_t Style, uint8_t* Buf, uint8_t Length){
	SPI_Write_Buf_2(Style, Buf, Length, Dev->CS_HIGH, Dev->CS_LOW);
}
void RFReadPayload(RFDevice *Dev, uint8_t Style, uint8_t* Buf, uint8_t Length){
	SPI_Read_Buf_2(Style, Buf, Length, Dev->CS_HIGH, Dev->CS_LOW);
}


/* RF���ܺ��� ----------------------------------------------------------------*/
/** @brief RF���ܺ���
  *
  */  
//����
void RFPowerDown(RFDevice *Dev)
{	
	RFDelay(200);
	RFWriteReg(Dev, CONFIG, 0x7c); 
}	

//����
void RFStandby(RFDevice *Dev) {	  
	RFWriteReg(Dev, CONFIG, 0x0d);
	RFDelay(200);
}
											
//���־λ	   
void RFClearFlags(RFDevice *Dev)
{							  
	RFWriteReg(Dev, RF_STATUS, 0x70);
}
 
//�巢�仺����
void RFFlushTx(RFDevice *Dev)
{	
	RFWriteCmd(Dev, FLUSH_TX, BK_NOP);
}

//����ջ�����
void RFFlushRx(RFDevice *Dev)
{	
	RFWriteCmd(Dev, FLUSH_RX, BK_NOP);
}

//��������
void RFTxStart(RFDevice *Dev)
{
	RFWriteReg(Dev, CONFIG, CONFIG_INIT&0xfe);	
}

//��������
void RFRxStart(RFDevice *Dev)
{
	RFWriteReg(Dev, CONFIG, CONFIG_INIT|0x01);	
}

//--------------------------------------------------------------
//				bank�л�
//		��0x07.7,�жϵ�ǰbank��Ŀ��ֵ�Ƿ���ͬ
//		����ͬ,�˳�;��ͬ,�л�bank
//--------------------------------------------------------------
void RFBankSelect(RFDevice *Dev, uint8_t bank)
{	
	uint8_t bank_status;

	bank_status = RFReadReg(Dev, 0x07); 
	bank_status &= (uint8_t)0x80;
	
	if(bank_status != bank)
	{
		//��ǰbank��Ŀ��bank��ͬ
		//д��ACTIVATE_CMD+0x53�л� 				
		RFWriteCmd(Dev, ACTIVATE, 0x53);
	}
}


