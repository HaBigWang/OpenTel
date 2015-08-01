
/***************************************************************************************************
						Bk2423 2G4�����շ�оƬ
***************************************************************************************************/   
#include "bk242x.h"	 
#include "eeprom.h"
	
/* RFģ��Ĵ������� ----------------------------------------------------------*/		
//In the array Bank1_Reg0_13,all[] the register value is the byte reversed!!!!!!!!!!!!!!!!!!!!!
const uint32_t Bank1_Reg0_13[]={  
#ifndef RF_STYLE_BK2423   //Bk2425  
	0xE2014B40,						//REG0
	0x00004BC0,						//REG1
	0x028CFCD0,						//REG2
	0x21390099,						//REG3,120517	
	0x1B8296F9,						/*REG4,120517	 		      1Mbps/2Mbps	       250Kbps
										      Normal Mode		        0x1B8296D9	       0x1B8AB6D9
										      High Power Mode		    0x1B8296F9	       0x1B8AB6F9
										      CW Normal Mode		    0x218296D9	       0x218AB6D9
										      CW High Power Mode	  0x218296F9	       0x218AB6F9	*/	 	
	0xA60F0624,						//REG5,120517,to enable RSSI measurement,use 0xA67F023C(maxium vthcd)
									      //0xA60F0624(20140813)
	0x00000000,						//6
	0x00000000,						//7
	0x00000000,						//8
	0x00000000,						//9
	0x00000000,						//10
	0x00000000,						//11
	0x00127300, 					/*REG12,120517
										      0x00127300:PLL locking time 120us compatible with BK2421;
										      0x00127305(chip default):PLL locking time 130us compatible with nRF24L01;	*/
	0x36B48000,						//REG13,120517	 	 0x46B48000
									      //	0x36B48000(20140813)	
#else  					        //Bk2423				
	0xE2014B40,						//REG0
	0x00004BC0,						//REG1
	0x028CFCD0,						//REG2
	0x21390099,						//REG3,120517	
	0x1B8296F9,						/*REG4,120517	 		1Mbps/2Mbps	        250Kbps
										      Normal Mode		    0x1B8296D9	       0x1B8AB6D9
										      High Power Mode		0x1B8296F9	       0x1B8AB6F9
										      CW Normal Mode		0x218296D9	       0x218AB6D9
										      CW High Power Mode	0x218296F9	       0x218AB6F9	*/	 	
	0xA67F0624,						//REG5,120517,to enable RSSI measurement,use 0xA67F023C(maxium vthcd)
	0x00000000,						//6
	0x00000000,						//7
	0x00000000,						//8
	0x00000000,						//9
	0x00000000,						//10
	0x00000000,						//11
	0x00127300, 					/*REG12,120517
										      0x00127300:PLL locking time 120us compatible with BK2421;
										      0x00127305(chip default):PLL locking time 130us compatible with nRF24L01;	*/		
	0x46B48000,						//REG13,120517
#endif	
};

const uint8_t Bank1_Reg14[] =
{
#ifndef RF_STYLE_BK2423	//Bk2425
	0x41,0x20,0x08,0x04,0x81,0x20,0xcf,0xf7,0xfe,0xff,0xff		//0x41 20 08 04 81 20 CF F7 FE FF FF(20140815)
#else						        //Bk2423  		  
	0x41,0x10,0x04,0x82,0x20,0x08,0x08,0xF2,0x7D,0xEF,0xFF		//0x41,0x10,0x04,0x82,0x20,0x08,0x08,0xF2,0x7D,0xEF,0xFF
#endif
};

//Bank0 register initialization value
const uint8_t Bank0_Reg[10][2]={
	{RX_PW_P0,		RF_FRAME_LENGTH}, 			//0		���ݳ���6~32�ֽ�	
	{RF_STATUS,		0x70}, 			              //1		������״̬��־
	{RF_SETUP,		0x07}, 			              //2		1Mbps,5dBm��High gain
	{SETUP_RETR,	0x35}, 			              //3		���1000us �ط�5��
	{SETUP_AW,		0x03}, 			              //4		��ַ���5�ֽ�
	{EN_RXADDR,		0x01}, 			              //5		ֻʹ��pip0 ����
	{EN_AA,			  0x01},  		              //6		Enb Auto ACK
	{CONFIG,		  0x0d}, 			              //7		Enable�ж�,2 Byte CRC��Rx��Power Off

	{FEATURE,		  0x04}, 			              //8		no dpl,no ack_pay,dis W_TX_PAYLOAD_NOACK command
	{DYNPD,			  0x01}, 			              //9
};		    


/* �û��Զ��� ----------------------------------------------------------------*/
//RF channel frequence
const uint8_t RFFrequencyTable[RF_CHANNEL_SUM] = {	
				48,72,64,56,
        48,72,64,56,	   
	      48,72,64,56,
	      48,72,64,56,	};				  						  
							

#define BK242X_DEV_SUM 2

RFDevice Bk242xDev[BK242X_DEV_SUM];
RFDevice *Bk242xTx, *Bk242xRx;
  

/*=======================================================================================================
=======================================================================================================*/
void RFInitialization(RFDevice *Dev);
void RFTxCtrl(RFDevice *Dev);
void RFRxCtrl(RFDevice *Dev);


/** @Brief ��Ƶ�豸��ʼ��
  *   1) �ϵ���ȵ���SPI��ʼ��������Ȼ���ٵ��ô˺�����
  *   2) �˴�������¼�������:
	*			a�����շ��豸ָ�븳ֵ
	*			b�������豸��CE��CS�ܽ�(����SPI�г�ʼ��)
	*			c����ȡ������ַ����Ϊ�豸Bk242xTx��ַ����ȡԶ�˵�ַ����ΪBk242xRx��ַ��
	*			d����ʼ����ƵоƬ
	*
	* @Note
	*   1) ������Bk242x 2.4GоƬ��������оƬ����SPI�ӿڣ���CS��CE�Ÿ��Զ�����
	*   2) һ��2.4GоƬ�����䣬����Bk242xTx����һ�����գ�ΪBk242xRx��
	*   3) Bk242xTx�ĵ�ַ��оƬUnique ID��Bk242xRx��ַ�洢��EEPROM�С�
	*
  */
void RFDeviceOpen(void) {
	uint8_t i;
	
	Bk242xTx = &Bk242xDev[0]; Bk242xRx = &Bk242xDev[1];	
	
	Bk242xTx->CE_HIGH = Bk242xTx_CE_HIGH;  
	Bk242xTx->CE_LOW  = Bk242xTx_CE_LOW;
  Bk242xTx->CS_HIGH = Bk242xTx_CS_HIGH;
  Bk242xTx->CS_LOW  = Bk242xTx_CS_LOW;	
	Bk242xRx->CE_HIGH = Bk242xRx_CE_HIGH;  
	Bk242xRx->CE_LOW  = Bk242xRx_CE_LOW;
  Bk242xRx->CS_HIGH = Bk242xRx_CS_HIGH;
  Bk242xRx->CS_LOW  = Bk242xRx_CS_LOW;			
	
	Bk242xTx->flag.bPackageSent	= 0;
	Bk242xTx->flag.bPackageReceive = 0;
	Bk242xTx->packageT.length = 0;
	Bk242xTx->packageT.status = ineffect;	
	Bk242xRx->flag.bPackageSent	= 0;
	Bk242xRx->flag.bPackageReceive = 0;
	Bk242xRx->packageT.length = 0;
	Bk242xRx->packageT.status = ineffect;
	
	RFGetUinqueAddr(Bk242xTx->Addr); Bk242xTx->Addr[4] = RF_PUBLIC_ADDR_TX;
	for(i=0; i<5; i++){
		Bk242xRx->Addr[i] = ConfigGlobal.rfAddr[i];
	}
	
	RFInitialization(Bk242xTx);
	RFInitialization(Bk242xRx);
}

/** @Brief ����
  *   1) ���ڷ����(ͨ��ָң����)
  *   2) ��֮��Ӧ�Ľ��ն˶������Ϊ RFRxBind()
  *
  * @Note 
  * @ RF_PUBLIC_ADDR_TX, RF_PUBLIC_ADDR_RX������������:
  *   1)������ң�ض˺ͽ��նˡ�
  *   2)�����ֲ�ͬң��������ͬ���նˡ�
  *   ȡֵ��Χ: RF_PUBLIC_ADDR_TX = 10000001 ~ 11111111; RF_PUBLIC_ADDR_RX = 00000001 ~ 01111111��
  *
  */ 
void RFTxBind(RFDevice *Dev)
{	
	RFFlags* rfFlag = &Dev->flag;
  RFVariable* rfCtrl = &Dev->variable;	
	uint8_t* rxBuf = Dev->packageR.buf;
	uint8_t buffer[8];
		
	rfFlag->bBinding = 1;
	rfFlag->bDataReturn  = 0;

	//-------------------------------------------------------------------------------
	//Step1	��һ������
	//	@ �������䱾����ַ
	//  @ ����ɹ������ڶ�������
	//-------------------------------------------------------------------------------	
RFTxBindingRoll:		
	while(1) {
		RFFlushTx(Dev);
		RFStandby(Dev);
		RFFlushRx(Dev);
		RFClearFlags(Dev);
		 				
 		//-------------------------------------------------------------------
	  //Step1	��һ������
	  //	@ �������䱾����ַ
	  //  @ ����ɹ������ڶ�������
 		//-------------------------------------------------------------------		
	  rfFlag->bPackageSent	= 0;
	  rfFlag->bPackageReceive = 0;
	  rfCtrl->txPeriod  = 0;
		rfCtrl->frequencyHopPeriod = 0;
		rfCtrl->bindingTimer = 0;
		 	
	  buffer[0] = RF_PUBLIC_ADDR0; 
		buffer[1] = RF_PUBLIC_ADDR1;
		buffer[2] = RF_PUBLIC_ADDR2;
		buffer[3] = RF_PUBLIC_ADDR3;
		buffer[4] = RF_PUBLIC_ADDR4;  								   
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);
		
		Dev->packageT.length = 0;
		Dev->packageT.status = ineffect;
		buffer[0] = RF_SHANKHAND_FIRST;
		buffer[1] = Dev->Addr[0];
	  buffer[2] = Dev->Addr[1];
		buffer[3] = Dev->Addr[2];
	  buffer[4] = Dev->Addr[3];
		buffer[5] = RF_PUBLIC_ADDR_TX;			
		RFSendPackage(Dev, buffer, 6, 0);
 		 	
		while(!rfFlag->bPackageSent){
			RFBindCallFunction();				
			RFTxCtrl(Dev);
		}
		rfFlag->bPackageSent = 0;
		RFFlushTx(Dev);
		RFStandby(Dev);
		RFClearFlags(Dev);
				
 		//-------------------------------------------------------------------
 		//Step2  �ڶ�������
		//  @Brief ��ȡ���ն˵�ַ��������buffer[]�� 
		//  @ ��������ַд��RFģ�飬Ȼ���л�Ϊ����ģʽ
		//  @ ��������100ms
 		//------------------------------------------------------------------- 		
	  buffer[0] = Dev->Addr[0]; 
		buffer[1] = Dev->Addr[1];
		buffer[2] = Dev->Addr[2];
		buffer[3] = Dev->Addr[3];
		buffer[4] = RF_PUBLIC_ADDR_TX;  								   
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);
 								
	  rfFlag->bPackageReceive = 0;
	  rfCtrl->rxPeriod = 0;
		rfCtrl->bindingTimer = 0; 		
	  while(rfCtrl->bindingTimer <= 100) {			
		  rfCtrl->channelEffect = rfCtrl->channel;
		  rfCtrl->frequencyHopTimer = 0;
		  rfFlag->bFrequencyAlternative = 0;					
			RFRxCtrl(Dev);
			
			if(rfFlag->bPackageReceive){
				rfFlag->bPackageReceive = 0;
				
				if(rxBuf[2] == RF_SHANKHAND_SECOND){
					buffer[0] = rxBuf[3];
					buffer[1] = rxBuf[4];
					buffer[2] = rxBuf[5];
					buffer[3] = rxBuf[6];
					buffer[4] = rxBuf[7];					
				}
			}
		}		
		RFStandby(Dev);
		RFFlushRx(Dev);
		RFClearFlags(Dev);
		if(!rfFlag->bPackageReceive) goto RFTxBindingRoll;
		rfFlag->bPackageReceive = 0;
 		 		
 		//-------------------------------------------------------------------
 		//Step3  ����������
		//  @Brief ����ȷ��
		//  @ �����ն˵�ַд��RFģ�飬Ȼ���л�Ϊ����ģʽ
		//  @ ��������100ms
 		//-------------------------------------------------------------------
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);	

		buffer[0] = RF_SHANKHAND_THRID;
		RFSendPackage(Dev, buffer, 6, 1);
	  rfFlag->bPackageSent = 0;
	  rfCtrl->txPeriod  = 0;
		rfCtrl->bindingTimer = 0;		
		
		while(rfCtrl->bindingTimer <= 100) {
		  rfCtrl->frequencyHopTimer = 0;
			rfCtrl->frequencyOccupyTimer = 0;
			RFTxCtrl(Dev);
		}
		if(!rfFlag->bPackageSent) goto RFTxBindingRoll;		
		rfFlag->bPackageSent = 0;
		RFFlushTx(Dev);
		RFStandby(Dev);
		RFClearFlags(Dev);
 		
 		//-------------------------------------------------------------------
 		//�������
		//  @Brief ������ַд��RFģ�飬�˳��������
 		//-------------------------------------------------------------------		 		 		
	  buffer[0] = Dev->Addr[0]; 
		buffer[1] = Dev->Addr[1];
		buffer[2] = Dev->Addr[2];
		buffer[3] = Dev->Addr[3];
		buffer[4] = RF_PUBLIC_ADDR_TX;								   
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);		

		Dev->packageT.length = 0;
		Dev->packageT.status = ineffect;
    rfFlag->bBinding = 0; 	
	}		
} 

/** @Brief ����
  *   1) ���ڽ��ն�(ͨ��ָ���ջ�)
  *   2) ��֮��Ӧ�ķ���˶������Ϊ RFTxBind()
  *
  * @Note 
  * @ RF_PUBLIC_ADDR_TX, RF_PUBLIC_ADDR_RX������������:
  *   1)������ң�ض˺ͽ��նˡ�
  *   2)�����ֲ�ͬң��������ͬ���նˡ�
  *   ȡֵ��Χ: RF_PUBLIC_ADDR_TX = 10000001 ~ 11111111; RF_PUBLIC_ADDR_RX = 00000001 ~ 01111111��
  *
  */ 
void RFRxBind(RFDevice *Dev){
	RFFlags* rfFlag = &Dev->flag;
  RFVariable* rfCtrl = &Dev->variable;	
	uint8_t *rxBuf = Dev->packageR.buf;
	uint8_t buffer[8], i;
		
	rfFlag->bBinding = 1;
	rfFlag->bDataReturn  = 0;
	
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------	
RFRxBindingRoll:		
	while(1) {
		RFFlushTx(Dev);
		RFStandby(Dev);
		RFFlushRx(Dev);
		RFClearFlags(Dev);
		 				
 		//-------------------------------------------------------------------
	  //Step1	��һ������
	  //	@ ��������
	  //  @ ���ճɹ������ڶ�������
 		//-------------------------------------------------------------------		 	
	  buffer[0] = RF_PUBLIC_ADDR0; 
		buffer[1] = RF_PUBLIC_ADDR1;
		buffer[2] = RF_PUBLIC_ADDR2;
		buffer[3] = RF_PUBLIC_ADDR3;
		buffer[4] = RF_PUBLIC_ADDR4;  								   
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);
 		 	
	  rfFlag->bPackageSent = 0;
	  rfFlag->bPackageReceive	= 0;
	  rfCtrl->rxPeriod  = 0;
		rfCtrl->frequencyHopTimer = 100;         
		while(!rfFlag->bPackageReceive){
			RFBindCallFunction();						
			RFRxCtrl(Dev);
		}
		rfFlag->bPackageReceive = 0;
		RFStandby(Dev);
		RFFlushRx(Dev);
		RFClearFlags(Dev);
	
		if(rxBuf[2] != RF_SHANKHAND_FIRST) goto RFRxBindingRoll;
		
 		//-------------------------------------------------------------------
	  //Step2	�ڶ�������
 		//-------------------------------------------------------------------	
	  buffer[0] = rxBuf[3]; buffer[1] = rxBuf[4]; buffer[2] = rxBuf[5]; buffer[3] = rxBuf[6]; buffer[4] = rxBuf[7];		 		   
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);
		for(i=0; i<5; i++) Dev->Addr[i] = buffer[i];
		
		Dev->packageT.length = 0;
		Dev->packageT.status = ineffect;
	  buffer[0] = RF_SHANKHAND_SECOND;		
		RFGetUinqueAddr(&buffer[1]);
		buffer[5] = RF_PUBLIC_ADDR_RX;		
		RFSendPackage(Dev, buffer, 6, 1);
				
	  rfFlag->bPackageSent	= 0;
	  rfCtrl->txPeriod  = 0;
		rfCtrl->bindingTimer = 0;
		while(rfCtrl->bindingTimer <= 100){
		  rfCtrl->frequencyHopTimer = 0;
		  rfCtrl->frequencyOccupyTimer = 0;
			RFTxCtrl(Dev);
		}
		RFFlushTx(Dev);
		RFStandby(Dev);
		RFClearFlags(Dev);		
		if(!rfFlag->bPackageSent) goto RFRxBindingRoll;
		rfFlag->bPackageSent = 0;
		Dev->packageT.length = 0;
		Dev->packageT.status = ineffect;
		
 		//-------------------------------------------------------------------
		//Step3 ����������
 		//-------------------------------------------------------------------
		RFGetUinqueAddr(buffer); buffer[4] = RF_PUBLIC_ADDR_RX;		   
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);
		
	  rfFlag->bPackageReceive = 0;
	  rfCtrl->rxPeriod  = 0;
		rfCtrl->bindingTimer = 0;
		while(rfCtrl->bindingTimer <= 100){
		  rfCtrl->channelEffect = rfCtrl->channel;
		  rfCtrl->frequencyHopTimer = 0;
		  rfFlag->bFrequencyAlternative = 0;
			RFRxCtrl(Dev);
		}
		RFStandby(Dev);
		RFFlushRx(Dev);
		RFClearFlags(Dev);		
		if(!rfFlag->bPackageReceive) goto RFRxBindingRoll;		
		rfFlag->bPackageReceive = 0;
		
 		//-------------------------------------------------------------------
 		//�������
 		//-------------------------------------------------------------------	  
		for(i=0; i<5; i++) buffer[i] = Dev->Addr[i];	
		RFWriteBuf(Dev, TX_ADDR,    buffer, 5);	
		RFWriteBuf(Dev, RX_ADDR_P0, buffer, 5);
				
	  for(i=0; i<5; i++){
		  ConfigGlobal.rfAddr[i] = Dev->Addr[i];
	  }
		EEPROM_Write();		
	}
}

   
/*=================================================================================================
=================================================================================================*/	   
/** @Brief ��ʼ��Bk242x 2.4GоƬ
  *
  */ 
void RFInitialization(RFDevice *Dev)
{				
 	uint8_t WriteArr[4];
	uint8_t i,j; 

	//1) �ϵ���ʱ
	//2) ���ʹ��CE�ţ�CE��Ϊ�ߵ�ƽ
	RFDelay(50000); 
	#ifdef RF_CE_USED
	Dev->CE_HIGH();
	#endif			

	//1) Bank0
	//2) �������ݳ��ȹ���ģʽ�ȣ�������ݶ��� 
	RFBankSelect(Dev, BANK0);	 
	for(i=0; i<8; i++)
		RFWriteReg(Dev, Bank0_Reg[i][0], Bank0_Reg[i][1]);

	i = RFReadReg(Dev, FEATURE);	
	if(i==0) 								            // i!=0 showed that chip has been actived.so do not active again.
		RFWriteCmd(Dev, ACTIVATE, 0x73);	// Active
	
	for(i=8; i<10; i++)
		RFWriteReg(Dev, Bank0_Reg[i][0], Bank0_Reg[i][1]);

	//1) Bank1
	//2) ����оƬҪ�󣬹̶�ֵ	
	RFBankSelect(Dev, BANK1);
	for(i=0; i<=8; i++){					      //reverse
		for(j=0; j<4; j++) WriteArr[j] = (Bank1_Reg0_13[i]>>(8*(j))) & 0xff;	 		
		RFWriteBuf(Dev, i, &(WriteArr[0]), 4);
	}
	for(i=9; i<=13; i++){
		for(j=0;j<4;j++) WriteArr[j] = (Bank1_Reg0_13[i]>>(8*(3-j))) & 0xff;
		RFWriteBuf(Dev, i, &(WriteArr[0]), 4);
	}
	RFWriteBuf(Dev, 14, (uint8_t *)&(Bank1_Reg14[0]), 11);

	//toggle REG4<25,26>
	for(j=0; j<4; j++) WriteArr[j]=( Bank1_Reg0_13[4]>>(8*(j)) ) & 0xff;

	WriteArr[0] = WriteArr[0]|0x06;
	RFWriteBuf(Dev, 4, &(WriteArr[0]), 4);	
	WriteArr[0] = WriteArr[0] & 0xf9;
	RFWriteBuf(Dev, 4, &(WriteArr[0]), 4);

	//1) �л���Bank0
	//2) д���ַ������������	
	RFBankSelect(Dev, BANK0);
	RFDelay(10000);   	
								   
	RFWriteBuf(Dev, TX_ADDR,    Dev->Addr, 5);	
	RFWriteBuf(Dev, RX_ADDR_P0, Dev->Addr, 5);		
} 

/** @brief �����Ƶ��
  */
uint8_t RFGetRandomFreq(RFDevice *Dev) {	 	    //����ָ��
  if(++Dev->variable.channelIndex >= RF_CHANNEL_SUM) Dev->variable.channelIndex = 0; 
  return RFFrequencyTable[Dev->variable.channelIndex];
} 
uint8_t RFGetNextRandomFreq(RFDevice *Dev) {	 	//������ָ��
  uint8_t i;	
	i = Dev->variable.channelIndex;
	if(++i > RF_CHANNEL_SUM) i = 0;
  return RFFrequencyTable[i];
} 
uint8_t RFGetNNextRandomFreq(RFDevice *Dev) {	 	//������ָ�� 
  uint8_t i;	
	i = Dev->variable.channelIndex;
	if(++i > RF_CHANNEL_SUM) i = 0;
	if(++i > RF_CHANNEL_SUM) i = 0;
  return RFFrequencyTable[i];
} 

/** @brief ���ݷ���
  * @   
  */
void RFTxCtrl(RFDevice *Dev) {	 
  RFFlags* rfFlag = &Dev->flag;
  RFVariable* rfCtrl = &Dev->variable;	
	uint8_t* FIFO = Dev->TxBuffer;
	uint8_t txReady = 0;
	 
  if(rfCtrl->txTimer >= rfCtrl->txPeriod) {
    rfCtrl->txTimer = 0;
    rfCtrl->txPeriod = 5;
		rfCtrl->frequencyHopPeriod = 50;
					 
		//Part1 ��ⷢ��״̬
		//  1) ����ɹ������±�־λ
    if(RFReadReg(Dev, RF_STATUS) & (uint8_t)bTXDS){
      txReady = 1;
      rfCtrl->frequencyHopTimer = 0;			

      if(FIFO[1] & 0x80){ 
				Dev->packageT.status = sent;				
				rfFlag->bPackageSent = 1;
			}					
    }
    RFFlushTx(Dev);		 	 
    RFStandby(Dev);
    RFClearFlags(Dev);		
			 
		//Part2 ��Ƶ
    //  1) ��Ƶ
	  //  2) ��Ҫ��ֹ��Ƶ(����ʱ)����ʹ:
		//    a�rrfCtrl->frequencyHopTimer = 0;
		//    b�rfCtrl->frequencyOccupyTimer = 0;
    if((rfCtrl->frequencyHopTimer >= rfCtrl->frequencyHopPeriod) || \
       (rfCtrl->frequencyOccupyTimer >= 360)) {		
      rfCtrl->frequencyHopTimer = 0;						 
      rfCtrl->frequencyOccupyTimer = 0;	 

      rfCtrl->channel = RFGetRandomFreq(Dev);
      RFWriteReg(Dev, RF_CH, rfCtrl->channel);
    }	

		//Part3 ���������������״̬
		//  1) 
    //  2) 
    if(txReady || ((Dev->packageT.status != tranling) && (Dev->packageReserve.status == newPackage))){  
		  if(RFPackageSplit(Dev)){						
        RFFlushTx(Dev);
        RFWritePayload(Dev, WR_TX_PLOAD, Dev->TxBuffer, RF_FRAME_LENGTH);
        RFTxStart(Dev);
			  Dev->packageT.status = tranling;
			} else {						
        RFFlushTx(Dev);		 	 
        RFStandby(Dev);
        RFClearFlags(Dev);	
						
			  Dev->packageT.status = ineffect;
			}
		} else {
			if(Dev->packageT.status == tranling){					
        RFFlushTx(Dev);
        RFWritePayload(Dev, WR_TX_PLOAD, Dev->TxBuffer, RF_FRAME_LENGTH);
        RFTxStart(Dev);						
			} else {
        RFFlushTx(Dev);		 	 
        RFStandby(Dev);
        RFClearFlags(Dev);	
						
		   	Dev->packageT.status = ineffect;
			}
		}
  }    
}
  		
/** @brief ���ݽ���
  *
  */
void RFRxCtrl(RFDevice *Dev) {
  RFFlags* rfFlag = &Dev->flag;
  RFVariable* rfCtrl = &Dev->variable;	
	uint8_t rxReady = 0;
  
	if(rfCtrl->rxTimer >= rfCtrl->rxPeriod){
		rfCtrl->rxTimer = 0;
    rfCtrl->rxPeriod = 5;
		
		//Part1 ������״̬
    //  1) ��״̬�Ĵ������ж��Ƿ�������
		//  2) ��������ɣ�����ɱ�־λ������Ƶ��ʱ
		//  3) ������ЧƵ��Ϊ��ǰ����Ƶ��
		if(RFReadReg(Dev, RF_STATUS) & (uint8_t)bRXDR){	 	 
      RFStandby(Dev);
      RFReadPayload(Dev, RD_RX_PLOAD, Dev->RxBuffer, RF_FRAME_LENGTH);	 
			if(RFPackageSplice(Dev)) rfFlag->bPackageReceive = 1;
      RFFlushRx(Dev);	
      RFClearFlags(Dev);				
			
      rxReady = 1;
			rfCtrl->frequencyHopTimer = 0;
			rfFlag->bFrequencyAlternative = 0;
			
			if(rfCtrl->channel != RFFrequencyTable[rfCtrl->channelIndex]){
				if(rfCtrl->channel == rfCtrl->channelNext) RFGetRandomFreq(Dev); 
				else if(rfCtrl->channel == rfCtrl->channelNextNext){
					RFGetRandomFreq(Dev);
					RFGetRandomFreq(Dev);
				} 
			} 
			rfCtrl->channelEffect = rfCtrl->channel;	
		}		
		
 		//1) ��û�гɹ����գ���ʼ��Ƶ
		//2) ǰ50ms��ѭ�������ڵ�ǰƵ�����һ��Ƶ��
		//3) 50~100ms��ѭ����������һ��Ƶ������¸�Ƶ��
		//4) ����100ms�����ɨ��Ƶ�ʱ�
		//5) ��Ҫ��ֹ��Ƶ(����ʱ)����ʹ:
		//*    a��rfCtrl->frequencyHopTimer = 0;
		//*    b��rfFlag->bFrequencyAlternative = 0;
		//*    c��rfCtrl->channelEffect = rfCtrl->channel;
		//*
		if(!rxReady){	
      RFStandby(Dev); 
      RFFlushRx(Dev);	
      RFClearFlags(Dev);				
		
			rfCtrl->channelNext = RFGetNextRandomFreq(Dev);		 	 	
			rfCtrl->channelNextNext = RFGetNNextRandomFreq(Dev);  

			if(rfCtrl->frequencyHopTimer < 50){				  				
				rfCtrl->channel = rfCtrl->channelEffect;
				if(rfFlag->bFrequencyAlternative) rfCtrl->channel = rfCtrl->channelNext;	
			} else if(rfCtrl->frequencyHopTimer < 100){ 
				rfCtrl->channel = rfCtrl->channelNext;
				if(rfFlag->bFrequencyAlternative) rfCtrl->channel = rfCtrl->channelNextNext;			
			} else {
				rfCtrl->frequencyHopTimer = 100;
				rfCtrl->channel = RFGetRandomFreq(Dev);
			}	
		
			rfFlag->bFrequencyAlternative = ~rfFlag->bFrequencyAlternative;
			RFWriteReg(Dev, RF_CH, rfCtrl->channel);
		}
		
		//1) ��������
 		RFRxStart(Dev);			 
	}
} 		

/** @Brief �ж����Զ�����
  * @ ÿ1ms�����Լ�1
  */
void RFTick(void){
	RFVariable *timer;
	uint8_t i;

  for(i=0; i<BK242X_DEV_SUM; i++){
		timer = &Bk242xDev[i].variable;
	  timer->txTimer++;
	  timer->rxTimer++;
	  timer->bindingTimer++;
	  timer->dataReturnTimer++;
	  timer->frequencyHopTimer++;
	  timer->frequencyOccupyTimer++;
	}			
}



