/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
 *     Feb 2016, v2.0 edit by JUIXE julian3@umbc.edu
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "driver/uart.h"
#include "c_types.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "start_wifi.h"


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
 
void user_rf_pre_init(void)
{
}

extern UartDevice UartDev;
LOCAL os_timer_t init_timer;
int sequencem = 0;
int i = 0;
int gps_edit_state = 0;
void ICACHE_FLASH_ATTR
gps_uart_init(void)
{
	//if(gps_edit_state == 0){
		//os_printf("CHANGING GPS1 BAUD\n");
		//uart0_sendStr("$PMTK251,57600*2C\r\n");
		//UART_SetBaudrate(UART0,BIT_RATE_57600);
		//gps_edit_state++;
	//}
	if(gps_edit_state == 0){
		os_printf("REQUESTING MIN DATA ONLY\n");
		uart0_sendStr("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
		gps_edit_state++;
	}
	else{
		//os_printf("\n\nPINS: %d%d%d%d%d%d%d%d%d%d%d\n\n", GPIO_INPUT_GET(0),GPIO_INPUT_GET(2),GPIO_INPUT_GET(4),
		//	GPIO_INPUT_GET(5),GPIO_INPUT_GET(9), GPIO_INPUT_GET(10),GPIO_INPUT_GET(12),GPIO_INPUT_GET(13),
		//	GPIO_INPUT_GET(14),GPIO_INPUT_GET(15), GPIO_INPUT_GET(16));
		//for(i = 0; i < 16; i++)
		//	if(i != 0 & i!=1)
		//		os_printf(GPIO_INPUT_GET(i));
		//os_sprintf("%d, %d, %d \n", GPIO_INPUT_GET(5), GPIO_INPUT_GET(4),  GPIO_INPUT_GET(14));
		//os_printf("\n\nPINS: %04x\n", gpio_input_get());
		//os_printf("ADC READ:%d\n\n\n", system_adc_read());

	}
    os_timer_arm(&init_timer, 1000, 0); 
}

void user_init(void)
{
	gpio_init();
	gpio_output_set(0, 0, 0, 0xFFFFFFFF);
	gpio_pin_wakeup_disable();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	ETS_GPIO_INTR_DISABLE();

	UART_SetPrintPort(UART1);
	UartDev.data_bits = EIGHT_BITS;
	UartDev.parity = NONE_BITS;
	UartDev.stop_bits = ONE_STOP_BIT;
	uart_init(BIT_RATE_9600, BIT_RATE_115200);

	uart0_sendStr("\r\n\r\n\r\n");

	system_phy_set_powerup_option(3);
	wifi_set_opmode(STATION_MODE); //Set station mode
    system_phy_set_max_tpw(82); //MAX POWERR!
	system_phy_set_tpw_via_vdd33(system_get_vdd33());
    user_set_station_config();

    os_timer_disarm(&init_timer);
    os_timer_setfn(&init_timer, (os_timer_func_t *)gps_uart_init, NULL);
    os_timer_arm(&init_timer, 1000, 0); 
}

