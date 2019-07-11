// Visible Light Transceiver
// by Bizan Nishimura

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "project.h"
#include "Timer.h"

//#define AUTO_TX_TEST
//#define LOOP_BACK_TEST
//#define UART_BRIDGE_TEST
//#define CONTROLLER_TEST
#define RECEIVER_TEST

#define USBFS_DEVICE        0
#define USBUART_BUFFER_SIZE 64
#define DATA_SIZE 3
#define TX_CYCLE 20 // [msec]
#define RX_TIMEOUT 100 // [msec]

static volatile uint16_t servo = 307;

// servo pwm interrupt
CY_ISR(ISR_PWM_Servo)
{
    PWM_Servo_ReadStatusRegister();
    
    PWM_Servo_WriteCompare(servo);
}

// every 100msec, get SW and Volume input => VLCom send
static void input_and_send(void)
{
    if(Timer_hasCycled()){
        
        Pin_Test1_Write(1);
        
        // SW input
        uint8_t tx_sw1 = Pin_SW1_Read();
        uint8_t tx_sw2 = Pin_SW2_Read();
        uint8_t tx_sw3 = Pin_SW3_Read();
        uint8_t tx_sw123 = 0;
        if(tx_sw1 != 0) tx_sw123 |= 0x01;
        if(tx_sw2 != 0) tx_sw123 |= 0x02;
        if(tx_sw3 != 0) tx_sw123 |= 0x04;
        
        // Volume ADC input
        ADC_Volume_StartConvert();
        ADC_Volume_IsEndConversion(ADC_Volume_WAIT_FOR_RESULT);
        uint16_t tx_adc = ADC_Volume_GetResult16();
        
        // message packet
        uint8_t tx_data[7];
        tx_data[0] = 0xA5;
        tx_data[1] = 3;
        tx_data[2] = (uint8_t)(tx_adc >> 8);
        tx_data[3] = (uint8_t)(tx_adc & 0xFF);
        tx_data[4] = tx_sw123;
        tx_data[5] = 0x5A;
        tx_data[6] = tx_data[1] + tx_data[2] + tx_data[3] + tx_data[4];
        
        // send message
        int i;
        for(i=0;i<7;i++){
            while(TxEmpty_Read() == 0){;} // wait for empty
            TxData_Write(tx_data[i]);
            TxWr_Write(1);
            while(TxEmpty_Read() == 1){;} // wait for busy
        }
        Pin_Test1_Write(0);
    }
}

// on VLCom receive, set LED and Servo output
static void receive_and_output(void)
{
    static int rx_cnt = 0;
    static int data_len = 0;
    static uint8_t data_sum = 0;
    static uint8_t data_buf[DATA_SIZE];
    // VLCom receive
    if(RxFull_Read() == 1){ // if full
        uint8_t rx_data = RxData_Read();
        RxRd_Write(1);
        while(RxFull_Read() == 1){;} // wait for clear of full
        
        // precess message packet
        if(rx_cnt == 0){
            if(rx_data == 0xA5){
                Pin_Test1_Write(1);
                data_sum = 0;
                rx_cnt++;
                Timer_start(RX_TIMEOUT);
            }
        }else if(rx_cnt == 1){
            data_len = rx_data;
            data_sum += rx_data;
            rx_cnt++;
            if(data_len > DATA_SIZE) rx_cnt = 0;
        }else if(rx_cnt < 2 + data_len){
            data_buf[rx_cnt - 2] = rx_data;
            data_sum += rx_data;
            rx_cnt++;
        }else if(rx_cnt == 2 + data_len){
            if(rx_data == 0x5A){
                rx_cnt++;
            }else{
                rx_cnt = 0;
            }
        }else if(rx_cnt == 3 + data_len){
            if(data_sum == rx_data){
                //Pin_Test1_Write(1);
                
                // message received!
                uint16_t rx_adc = ((uint16_t)data_buf[0] << 8) | (uint16_t)data_buf[1];
                uint8_t rx_sw1 = (data_buf[2] & 0x01) ? 1 : 0;
                uint8_t rx_sw2 = (data_buf[2] & 0x02) ? 1 : 0;
                uint8_t rx_sw3 = (data_buf[2] & 0x04) ? 1 : 0;
                
                // LED output
                uint8_t led1 = (rx_sw1 == 0) ? 1 : 0;
                uint8_t led2 = (rx_sw2 == 0) ? 1 : 0;
                uint8_t led3 = (rx_sw3 == 0) ? 1 : 0;
                Pin_LED1_Write(led1);
                Pin_LED2_Write(led2);
                Pin_LED3_Write(led3);
                // Servo output
                servo = (uint16_t)(307 + 184 * ((signed int)rx_adc - 2048)  / 2048);
#if 0                
                // USB-Serial log output
                if (UsbUart_GetConfiguration() != 0) {
                    static char uart_buff[64];
                    sprintf(uart_buff, "Servo = %4d, Volume = %4d, SW1 = %d, SW2 = %d, SW3 = %d\n", 
                            servo, rx_adc, rx_sw1, rx_sw2, rx_sw3);
                    while (UsbUart_CDCIsReady() == 0){;}
                    UsbUart_PutString(uart_buff);
                }
#endif
                Pin_Test1_Write(0);
            }else{
                rx_cnt = 0;
            }
        }else{
            rx_cnt = 0;
        }
        
        if(Timer_isUp()) rx_cnt = 0;
    }
}

// main function
int main(void)
{
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
#ifdef LOOP_BACK_TEST    
    LoopBackEn_Write(1);
#else
    LoopBackEn_Write(0);
#endif
    Clock_80k_Start();
    Clock_640k_Start();
    TxRes_Write(1);
    RxRes_Write(1);
    CyDelay(20);
    
    Clock_Servo_Start();
    isrPWM_Servo_StartEx(ISR_PWM_Servo);
    PWM_Servo_Start();
    ADC_Volume_Start();
    
#ifdef CONTROLLER_TEST
    Timer_init(TX_CYCLE);
#else
    Timer_init(0);
#endif

    for(;;)
    {
        // USB-Serial initialize
        if (UsbUart_IsConfigurationChanged() != 0) {
            if (UsbUart_GetConfiguration() != 0) {
                UsbUart_CDC_Init();
            }
        }
        
#ifdef AUTO_TX_TEST
        if(TxEmpty_Read() == 1){ // if empty
            uint8_t txData = 'A';
            TxData_Write(txData);
            TxWr_Write(1);
            while(TxEmpty_Read() == 1){;} // wait for busy
        }
#endif
        
#ifdef UART_BRIDGE_TEST
        // USB-Serial communication
        if (UsbUart_GetConfiguration() != 0) {
            // UART receive => VLCom send
            if (UsbUart_DataIsReady() != 0) {
                // UART receive
                static uint8 buffer[64];
                int count = UsbUart_GetAll(buffer);
                if (count != 0) {
                    // VLCom send
                    uint8_t txData = buffer[0];
                    while(TxEmpty_Read() == 0){;} // wait for empty
                    TxData_Write(txData);
                    TxWr_Write(1);
                    while(TxEmpty_Read() == 1){;} // wait for busy
                }
            }
            // VLCom receive => UART send
            if(RxFull_Read() == 1){ // if full
                // VLCom receive
                uint8_t rxData = RxData_Read();
                RxRd_Write(1);
                // UART send
                while (UsbUart_CDCIsReady() == 0){;}
                UsbUart_PutData(&rxData, 1);
                
                while(RxFull_Read() == 1){;} // wait for clear of full
            }
        }
#endif
#ifdef CONTROLLER_TEST
        input_and_send();
#endif
#ifdef RECEIVER_TEST
        receive_and_output();
#endif
    }
}
