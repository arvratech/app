/*============================================================

	File Name: SPI.c
	Description: S3C2442A SPI controller Function Test
   	Version: 0.1 

   	History:
      0.1: 2003. 07. 06, Programming Start.
      0.2: 2004. 05. 14, added keyboard test codes 
      	         		  
=============================================================*/

#include <string.h>
#include "2442addr.h"
#include "2442lib.h"
#include "spi.h"
#include "def.h"

#define spi_count 0x80
#define SPI_BUFFER _NONCACHE_STARTADDRESS

void __irq Spi_Int(void);
void __irq Dma1Tx_Int(void);
void __irq Dma1Rx_Int(void);

void Test_Spi1_MS_poll(void); // added by junon

volatile char *spiTxStr,*spiRxStr;
volatile int endSpiTx;
volatile int tx_dma1Done;
volatile int rx_dma1Done;
unsigned int spi_rGPECON,spi_rGPEDAT,spi_rGPEUP;
unsigned int spi_rGPGCON,spi_rGPGDAT,spi_rGPGUP;
unsigned int spi_rGPDCON,spi_rGPDDAT,spi_rGPDUP;

/////////////////////// 2004.05.14 added by junon
#define ONEBIT    0x1

int	putcToKBCTL(U8 c);
void getsFromKBCTL(U8 *m, int cnt);
void Test_Spikbd_IO(void);
void Test_Spikbd_keyscan(void);

unsigned int spikbd_rGPBCON,spikbd_rGPBDAT,spikbd_rGPBUP;
unsigned int spikbd_rGPDCON,spikbd_rGPDDAT,spikbd_rGPDUP;
unsigned int spikbd_rGPFCON,spikbd_rGPFDAT,spikbd_rGPFUP;
unsigned int spikbd_rGPGCON,spikbd_rGPGDAT,spikbd_rGPGUP;
///////////////////////


/****************************************************************
 *	             SMDK2400 SPI configuration                 *
 *  GPG2=nSS0, GPE11=SPIMISO0, GPE12=SPIMOSI0, GPE13=SPICLK0     *
 *  GPG3=nSS1, GPG5 =SPIMISO1, GPG6 =SPIMOSI1, GPG7 =SPICLK1      *
 *  SPI1 is tested by OS(WINCE). So, Only SPI0 is tested by this code          *               
 ****************************************************************/

void * func_spi_test[][2]=
{	
	(void *)Test_Spi_MS_int, 				"SPI0 RxTx Int  ",
	(void *)Test_Spi_MS_poll, 				"SPI0 RxTx POLL ",
	(void *)Test_Spi_M_Int, 				"SPI0 M Rx INT  ",
	(void *)Test_Spi_S_Int, 				"SPI0 S Tx INT  ",
	(void *)Test_Spi_M_Tx_DMA1, 			"SPI0 M Tx DMA1 ",
	(void *)Test_Spi_S_Rx_DMA1, 			"SPI0 S Rx DMA1 ",
	(void *)Test_Spi_M_Rx_DMA1, 			"SPI0 M Rx DMA1 ",
	(void *)Test_Spi_S_Tx_DMA1, 			"SPI0 S Tx DMA1 ",
	(void *)Test_Spi1_MS_poll, 				"SPI1 RxTx POLL ",	
		0,0
};

void SPI_Port_Init(int MASorSLV)
{
	// SPI channel 0 setting
    spi_rGPECON=rGPECON;
    spi_rGPEDAT=rGPEDAT;
    spi_rGPEUP=rGPEUP;
    rGPECON=((rGPECON&0xf03fffff)|0xa800000); // using SPI 0
	rGPEUP = (rGPEUP & ~(7<<11)) | (1<<13);
    spi_rGPGCON=rGPGCON;
    spi_rGPGDAT=rGPGDAT;
    spi_rGPGUP=rGPGUP;
    if(MASorSLV==1)
    {
        rGPGCON=((rGPGCON&0xffffffcf)|0x10); // Master(GPIO_Output)
        rGPGDAT|=0x4; // Activate nSS 
    }
    else
    	rGPGCON=((rGPGCON&0xffffffcf)|0x30); // Slave(nSS)
    rGPGUP|=0x4;
/*
	// SPI channel 1-1 setting --> Key board
	rGPGCON=(rGPGCON&0xffff033f)|(3<<6)|(3<<10)|(3<<12)|(1<<14); // MISO1, MOSI1, CLK1, Master
	rGPGDAT|=0x8;
	rGPGUP=(rGPGUP&~(7<<5))|(1<<7);
*/
	// SPI channel 1-2 setting --> VD16~18
	spi_rGPDCON=rGPDCON;
	spi_rGPDDAT=rGPDDAT;
	spi_rGPDUP=rGPDUP;
	rGPDCON=(rGPDCON&0xcfc0ffff)|(3<<16)|(3<<18)|(3<<20)|(1<<28); // MISO1, MOSI1, CLK1, Master
	rGPDDAT|=1<<14;
	rGPDUP=(rGPDUP&~(7<<8))|(1<<10);
}

void SPI_Port_Return(void)
{
    rGPECON=spi_rGPECON;
    rGPEDAT=spi_rGPEDAT;
    rGPEUP=spi_rGPEUP;

    rGPGCON=spi_rGPGCON;
    rGPGDAT=spi_rGPGDAT;
    rGPGUP=spi_rGPGUP;

    rGPDCON=spi_rGPDCON;
    rGPDDAT=spi_rGPDDAT;
    rGPDUP=spi_rGPDUP;
}

void Test_Spi1_MS_poll(void)
{
    int i;
  
    char *txStr,*rxStr;
    SPI_Port_Init(1); 
    Uart_Printf("[SPI Polling Tx/Rx Test]\n");
    Uart_Printf("Connect SPIMOSI1 into SPIMISO1.\n");
    endSpiTx = 0;
    spiTxStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 - SPI1";
    spiRxStr = (char *) SPI_BUFFER;
    txStr = (char *)spiTxStr;
    rxStr = (char *)spiRxStr;

    rSPPRE1 = 0x0;	//if PCLK=50Mhz,SPICLK=25Mhz
    rSPCON1 = (0<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//Polling,en-SCK,master,low,A,normal
    rSPPIN1 = (0<<2)|(1<<1)|(0<<0);//dis-ENMUL,SBO,release
    
    while(endSpiTx==0) {
		if(rSPSTA1&0x1) {   //Check Tx ready state    
	    	if(*spiTxStr) rSPTDAT1 = *spiTxStr++;
	    	else	endSpiTx = 1;
	    	while(!(rSPSTA1&0x1)) ;   //Check Rx ready state 
			*spiRxStr++ = rSPRDAT1;
		}
    }
    rSPCON1 &= ~(1<<4);	//dis-SCK
    *(spiRxStr-1) = 0;	//remove last dummy data & attach End of String(Null)
    
    Uart_Printf("Tx Strings:%s\n", txStr);
    Uart_Printf("Rx Strings:%s :", rxStr);
    
    if(!strcmp(rxStr,txStr)) Uart_Printf("O.K.\n");
    else	Uart_Printf("ERROR!!!\n");
    SPI_Port_Return();
}

void Test_Spi_MS_int(void)
{
    char *txStr,*rxStr;

    SPI_Port_Init(1); 
    Uart_Printf("[SPI0 Interrupt Tx/Rx Test]\n");
    Uart_Printf("Connect SPIMOSI0 into SPIMISO0.\n");

    pISR_SPI0 = (unsigned)Spi_Int;
    endSpiTx = 0;
    spiTxStr = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    spiRxStr = (char *)SPI_BUFFER;
    txStr = (char *)spiTxStr;
    rxStr = (char *)spiRxStr;
    rSPPRE0 = 0x0;	//if PCLK=50Mhz,SPICLK=25Mhz
    rSPCON0 = (1<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|0;//int,en-SCK,master,low,A,normal
    rSPPIN0 = (0<<2)|(1<<1)|(0<<0);//dis-ENMUL,SBO,release
    rINTMSK = ~BIT_SPI0;
 
    while(endSpiTx==0) ;

	rSPCON0 &= ~((1<<5)|(1<<4)); //poll, dis-SCK
	Uart_Printf("Current Rx address = 0x%x\n",spiRxStr);
    *spiRxStr = 0;//attach End of String(Null)

    Uart_Printf("Tx Strings:%s\n", txStr);
    Uart_Printf("Rx Strings:%s :", rxStr+1);//remove first dummy data
    if(!strcmp(rxStr+1, txStr)) Uart_Printf("O.K.\n");
    else	Uart_Printf("ERROR!!!\n");
    SPI_Port_Return();
}

void __irq Spi_Int(void)
{
    unsigned int status;
		
	rINTMSK |= BIT_SPI0;
    ClearPending(BIT_SPI0); 
    status = rSPSTA0;
    if(rSPSTA0 & 0x6)  Uart_Printf("Data Collision or Multi Master Error(0x%x)!!!\n", status);
    while(!(rSPSTA0&0x1));   //Check ready state
    *spiRxStr++ = rSPRDAT0;    //First Rx data is garbage data
//	Uart_Printf("Current Rx address = 0x%x\n",spiRxStr);
    if(*spiTxStr) {
    	rSPTDAT0 = *spiTxStr++;
		rINTMSK &= ~BIT_SPI0;
    } else {
	    endSpiTx=1;
    }
}

void Test_Spi_M_Tx_DMA1(void)
{

    int i,first,second;
    unsigned char *tx_ptr;
    SPI_Port_Init(1); // Master(GPIO) 
    Uart_Printf("[SPI DMA1 Master Tx test]\n");
    Uart_Printf("This test should be configured two boards\nStart Rx first.\n");

    tx_ptr=(unsigned char *) SPI_BUFFER;
    for(i=0; i<spi_count; i++)
		*(tx_ptr+i)=i; 

    tx_dma1Done=0;

    pISR_DMA1 = (unsigned)Dma1Tx_Int;
    rINTMSK=~(BIT_DMA1);

    //Step1. SPI init
    rSPPRE0 = 0x0;	//if PCLK=50Mhz,SPICLK=25Mhz
    rSPCON0 = (2<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//DMA1,en-SCK,master,low,A,normal
//    rSPCON0=(2<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(0<<0);//DMA1,en-SCK,master,low,B,normal    
    rSPPIN0 = (0<<2)|(1<<1)|(0<<0);//dis-ENMUL,SBO,release

    //Step2. DMA1 init 
    rDISRC1 = (unsigned)tx_ptr;	//Address of Memory
    rDISRCC1 = (0<<1)|(0);		//AHB(Memory), inc
    rDIDST1 = (unsigned)0x59000010;//Address of SPTDAT Register
    rDIDSTC1 = (1<<1)|(1);		//APB(SPI), fix
    rDCON1 = (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(3<<24)|(1<<23)|(1<<22)|(0<<20)|(spi_count);
    //handshake, sync PCLK, TC int, single tx, single service, SPI, H/W request, 
    //off-reload, byte, 128Bytes

    rGPGDAT &= 0xfffffffb; // Activate nSS 
    rDMASKTRIG1 = (0<<2)|(1<<1)|(0);    //run, DMA1 channel on, no-sw trigger 

    while(tx_dma1Done==0);

    while(!(rSPSTA0&0x1)); // Tx is done

    rGPGDAT |= 0x4; // Deactivate nSS 
//    rSPCON0=(0<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//poll,dis-SCK,master,low,A,normal
    rSPCON0 = (0<<5)|(0<<4)|(1<<3)|(1<<2)|(1<<1)|(0<<0);//poll,dis-SCK,master,low,B,normal
    Uart_Printf("\nDMA1 transfer end\n");
    for(i=0; i<spi_count; i++)
		Uart_Printf("0x%02x,",*tx_ptr++);
    SPI_Port_Return();
}

void __irq Dma1Tx_Int(void)
{
    tx_dma1Done=1;
    rINTMSK|=BIT_DMA1;
    ClearPending(BIT_DMA1);
}

void __irq Dma1Rx_Int(void)
{
    rx_dma1Done=1;
    rINTMSK|=BIT_DMA1;
    ClearPending(BIT_DMA1);
}

void Test_Spi_M_Rx_DMA1(void)
{
    int i;
    unsigned char *rx_ptr;
    SPI_Port_Init(1); // Master(GPIO)
    Uart_Printf("[SPI DMA1 Master Rx Test]\n");
    Uart_Printf("This test should be configured two boards\nStart Tx first.\n");

    rx_ptr=(unsigned char *)SPI_BUFFER;
    for(i=0;i<0x500;i++)
		*(rx_ptr+i)=0x0; // Zero Initialize
    rx_dma1Done=0;

    pISR_DMA1 = (unsigned)Dma1Rx_Int;
    rINTMSK=~(BIT_DMA1);
    //Step1. SPI init

    rSPPRE0 = 0x1;	//if PCLK=50Mhz,SPICLK=25Mhz
//    rSPCON0=(2<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|(1<<0);//DMA1,en-SCK,master,low,A,TAGD    
    rSPCON0 = (2<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0);//DMA1,en-SCK,master,low,B,TAGD
    rGPGDAT &= 0xfffffffb; // Activate nSS 
    //When you use [Master Rx with DMA] function you should have to set TAGD bit
    rSPPIN0 = (0<<2)|(1<<1)|(0<<0);//dis-ENMUL,SBO,release

    //Step2. DMA1 init 
    rDISRC1 = (unsigned)0x59000014;//Address of SPRDAT Register
    rDISRCC1 = (1<<1)|(1);		//APB(SPI), fix
    rDIDST1 = (unsigned)rx_ptr;	//Address of Memory
    rDIDSTC1 = (0<<1)|(0);		//AHB(Memory), inc
    rDCON1 = (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(3<<24)|(1<<23)|(1<<22)|(0<<20)|(spi_count);
    //handshake, sync PCLK, TC int, single tx, single service, SPI, H/W request, 
    //off-reload, byte, 128Bytes
    rDMASKTRIG1 = (0<<2)|(1<<1)|(0);    //run, DMA1 channel on, no-sw trigger 

    while(rx_dma1Done==0);

    rSPCON0 = (0<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//poll,dis-SCK,master,low,A,normal
//    rSPCON0=(0<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(0<<0);//poll,dis-SCK,master,low,B,normal

    while(!rSPSTA0 & 0x1);

    *(rx_ptr+spi_count) = rSPRDAT0;
    rx_ptr = (unsigned char *)SPI_BUFFER;
    rGPGDAT |= 0x4; // Deactivate nSS 
    for(i = 1;i < spi_count+1;i++)
		Uart_Printf("0x%02x,", *(rx_ptr+i));//to remove first dummy data

    Uart_Printf("\nDMA1 receive end");
    SPI_Port_Return();
}

void Test_Spi_M_Int(void)
{
    char *rxStr,*txStr;
    SPI_Port_Init(1); // Master
    Uart_Printf("[SPI Interrupt Master Rx test]\n");
    Uart_Printf("This test should be configured two boards\nStart Slave first.\n");
    pISR_SPI0=(unsigned)Spi_Int;
    endSpiTx = 0;
    spiTxStr = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    spiRxStr = (char *)SPI_BUFFER;
    txStr = (char *)spiTxStr;
    rxStr = (char *)spiRxStr;
    rSPPRE0 = 0x1;	//if PCLK=50Mhz,SPICLK=12.5Mhz
    rSPCON0 = (1<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//int,en-SCK,master,low,A,normal
//    rSPCON0=(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(0<<0);//int,en-SCK,master,low,B,normal
    rSPPIN0 = (0<<2)|(1<<1)|(0<<0);//dis-ENMUL,SBO,release
    rGPGDAT &= 0xfffffffb; // Activate nSS 
    rINTMSK = ~(BIT_SPI0);

    while(endSpiTx==0);

    rGPGDAT |= 0x4; // Deactivate nSS 
    rSPCON0 = (0<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//Poll,dis-SCK,master,low,A,normal
//    rSPCON0=(0<<5)|(0<<4)|(1<<3)|(1<<2)|(1<<1)|(0<<0);//Poll,dis-SCK,master,low,B,normal
    Uart_Printf("Current address :0x%x\n",spiRxStr);
    *spiRxStr='\0';//attach End of String(Null)
    Uart_Printf("Tx Strings:%s\n",txStr);
    Uart_Printf("Rx Strings:%s :",rxStr+1);//remove first dummy data
    if(strcmp((rxStr+1),txStr)==0)
        Uart_Printf("O.K.\n");
    else 
        Uart_Printf("ERROR!!!\n");
    SPI_Port_Return();
}
