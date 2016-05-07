/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *	 2014/1/1, v1.0 create this file.
 *	 Feb 2016, v2.0 edit by JUIXE julian3@umbc.edu
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
#include "qwifi.h"
#include "driver/softuart.h"


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns	  : none
*******************************************************************************/
 
void user_rf_pre_init(void)
{
}

extern UartDevice UartDev;
LOCAL os_timer_t init_timer;
int sequencem = 0;
int i = 0;
int gps_edit_state = 0;
//create global softuart instances
Softuart softuart;

// change speed to 200ms, see https://github.com/sarthakkaingade/gps_config/blob/master/gps_config.c
char DeviceBuffer[16] = {0};
//char ms200string[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A};
void ICACHE_FLASH_ATTR
gps_uart_init(void)
{
	switch(gps_edit_state){

		case 0:
		//change baud
		//uart0_sendStr("\n\r$PMTK251,57600*2C\r\n");
		//UART_SetBaudrate(0,57600);
		os_printf("REQUESTING MIN DATA ONLY\n");
		//requests RMC only
		uart0_sendStr("\n\r\n\r$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\n\r");
		//startup
		Softuart_Init(&softuart,9600);
		Softuart_Puts(&softuart, "\n\r\n\r$PUBX,40,GLL,0,0,0,0,0,0*5C\n\r");
		break;
		case 1:
		// increase fix rate to 5Hz
		uart0_sendStr("\n\r$PMTK300,200,0,0,0,0*2F\n\r");
		// increase output rate to 10Hz
		uart0_sendStr("\n\r$PMTK220,200*2C\n\r");
		uart0_sendStr("$PMTK605*31\n\r");
		//turn off VTG
		Softuart_Puts(&softuart, "$PUBX,40,VTG,0,0,0,0,0,0*5E\n\r");
		break;
		case 2:
		//turn off GGA
		Softuart_Puts(&softuart, "$PUBX,40,GGA,0,0,0,0,0,0*5A\n\r");
		break;
		case 3:
		//turn off GSA
		Softuart_Puts(&softuart, "$PUBX,40,GSA,0,0,0,0,0,0*4E\n\r");
		break;
		case 4:
		// turn off GSV
		Softuart_Puts(&softuart, "$PUBX,40,GSV,0,0,0,0,0,0*59\n\r");
		break;
		case 5:
		// change speed to 200ms
		//,0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A);
		Softuart_Putchar(&softuart, 0xB5);
		Softuart_Putchar(&softuart, 0x62);
		Softuart_Putchar(&softuart, 0x06);
		Softuart_Putchar(&softuart, 0x08);
		Softuart_Putchar(&softuart, 0x06);
		Softuart_Putchar(&softuart, 0x00);
		Softuart_Putchar(&softuart, 0xC8);
		Softuart_Putchar(&softuart, 0x00);
		Softuart_Putchar(&softuart, 0x01);
		Softuart_Putchar(&softuart, 0x00);
		Softuart_Putchar(&softuart, 0x01);
		Softuart_Putchar(&softuart, 0x00);
		Softuart_Putchar(&softuart, 0xDE);
		Softuart_Putchar(&softuart, 0x6A);
		Softuart_Putchar(&softuart, '\n');
		Softuart_Putchar(&softuart, '\r');
		break;
	}
	if(gps_edit_state < 10){
		os_timer_arm(&init_timer, 1519, 0); 
		gps_edit_state++;
	}
}

void ICACHE_FLASH_ATTR
user_init(void)
{
	gpio_init();
	gpio_output_set(0, 0, 0, 0xFFFFFFFF);

	//init software uart
	Softuart_SetPinRx(&softuart,13); 	
	Softuart_SetPinTx(&softuart,15);

	gpio_pin_wakeup_disable();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);

	UART_SetPrintPort(UART1);
	UartDev.data_bits = EIGHT_BITS;
	UartDev.parity = NONE_BITS;
	UartDev.stop_bits = ONE_STOP_BIT;
	uart_init(BIT_RATE_9600, BIT_RATE_115200);


	//clear noise
	uart0_sendStr("\r\n\r\n\r\n");

	//wifi_set_phy_mode(3);
	system_phy_set_powerup_option(3);
	wifi_set_opmode(STATION_MODE); //Set station mode
	system_phy_set_max_tpw(82); //MAX POWERR!
	system_phy_set_tpw_via_vdd33(system_get_vdd33());
	user_set_station_config();

	os_timer_disarm(&init_timer);
	os_timer_setfn(&init_timer, (os_timer_func_t *)gps_uart_init, NULL);
	os_timer_arm(&init_timer, 1000, 0); 
}

