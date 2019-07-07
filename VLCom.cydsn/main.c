// Visible Light Transceiver
// by Bizan Nishimura

#include "project.h"

//#define AUTO_TX_TEST

#define USBFS_DEVICE        0
#define USBUART_BUFFER_SIZE 64

int main(void)
{
    uint16 count;
    uint8 buffer[64];
    
    CyGlobalIntEnable;
    
    UsbUart_Start(USBFS_DEVICE, UsbUart_5V_OPERATION);
    Opamp_HPF_Start();
    Opamp_LPF_Start();
    Opamp_Vref_Start();
    PGA_1_Start();
    PGA_2_Start();
    PGA_1_SetGain(PGA_1_GAIN_01);
    PGA_2_SetGain(PGA_2_GAIN_08);
    Comp_Start();
    //LoopBackEn_Write(1); // loop-back test
    LoopBackEn_Write(0); // not loop-back test
    Clock_80k_Start();
    Clock_640k_Start();
    TxRes_Write(1);
    RxRes_Write(1);
    CyDelay(20);

    unsigned char rxData;
    unsigned char txData;
    for(;;)
    {
#ifdef AUTO_TX_TEST
        if(TxEmpty_Read() != 0){
            txData = 'A';
            TxData_Write(txData);
            TxWr_Write(1);
            CyDelay(20);
        }
#endif    
        // USB-Serial initialize
        if (UsbUart_IsConfigurationChanged() != 0) {
            if (UsbUart_GetConfiguration() != 0) {
                UsbUart_CDC_Init();
            }
        }
        
        // USB-Serial communication
        if (UsbUart_GetConfiguration() != 0) {
            // received data ready?
            if (UsbUart_DataIsReady() != 0) {
                // get received data
                count = UsbUart_GetAll(buffer);
                if (count != 0) {
                    
                    // send by VLC
                    txData = buffer[0];
                    while(TxEmpty_Read() == 0){;}
                    TxData_Write(txData);
                    TxWr_Write(1);
                    CyDelay(20);
                }
            }
            if(RxFull_Read() == 1){
                rxData = RxData_Read();
                RxRd_Write(1);
                
                // send by USB-Serial
                while (UsbUart_CDCIsReady() == 0){;}
                UsbUart_PutData(&rxData, 1);
                while(RxFull_Read() == 1){;}
                //if (count == USBUART_BUFFER_SIZE) {
                //    while (0u == USBUART_CDCIsReady()){;}
                //    USBUART_PutData(NULL, 0u);
                //}
            }
        }
    }
}
