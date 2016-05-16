/******************************************************************************
	 * Copyright 2013-2014 Espressif Systems
	 *
*******************************************************************************/
#include "qwifi.h"  
LOCAL os_timer_t test_timer;
LOCAL os_timer_t send_timer;
LOCAL struct espconn user_udp_espconn;
LOCAL struct espconn user_udp_espconn_rx;
int sequence = 0;
int flystate = 0;
uint8_t initNavData = 1;
int adc_read_last = 1000;

#define ATREF   "AT*REF="
#define ATPCMD  "AT*PCMD="
#define TAKEOFF ",290718208"
#define LAND    ",290717696"

#define d50asSingle   1056964608LL
#define d75asSingle   1061158912LL
#define neg50asSingle 3204448256LL
#define neg75asSingle 3208642560LL
  
#define FLYSTATE_INITNAV       0
#define FLYSTATE_ACK1          1
#define FLYSTATE_OUTDOOR       2
#define FLYSTATE_OUTDOORSHELL  3
#define FLYSTATE_GROUNDED_START 4
#define FLYSTATE_TAKEOFF	   5
#define FLYSTATE_MANUAL1	   6
#define FLYSTATE_AUTOMATIC     7
#define FLYSTATE_MANUAL2	   8
#define FLYSTATE_GROUNDED_END  9
#define FLYSTATECOUNT          10

/*---------------------------------------------------------------------------*/
LOCAL struct espconn ptrespconn;

 /******************************************************************************
  * FunctionName : user_udp_recv_cb
  * Description  : Processing the received udp packet
  * Parameters	: arg -- Additional argument to pass to the callback function
  *				pusrdata -- The received data (or NULL when the connection has been closed!)
  *				length -- The length of received data
  * Returns	  : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_udp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	os_printf("recv udp data: %s\n", pusrdata);
}


uint8_t forward, left, right, needsCalibration;
 /******************************************************************************
	  * FunctionName : user_udp_send
	  * Description  : udp send data
	  * Parameters  : none
	  * Returns	  : none
 *******************************************************************************/

//this function is called every 50ms. it sends the appropriate command to the quadcoper
 LOCAL void ICACHE_FLASH_ATTR
 user_udp_send(void)
 {
	char DeviceBuffer[40] = {0};
	char hwaddr[6];
	struct ip_info ipconfig;
	unsigned long pitch;
	unsigned long speed;
	const char udp_remote_ip[4] = { 255, 255, 255, 255}; 
	os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4); // ESP8266 udp remote IP need to be set everytime we call espconn_sent
	user_udp_espconn.proto.udp->remote_port = 5556;  // ESP8266 udp remote port need to be set everytime we call espconn_sent
	

//	os_printf("start b0:\n");
	asyncProcess(0);
//	os_printf("end b0:\n");
//	os_printf("start b1:\n");
	asyncProcess(1);
//	os_printf("end b1:\n");

	os_printf("LAT0: %d LONG0: %d, TIME0: %d LAT1: %d LONG1: %d TIME1: %d\n", 
		(int)(getGPS(0, RMC_LAT)*100000), (int)(getGPS(0, RMC_LONG)*100000), 
		(int)getGPS(0, RMC_TIME), (int)(getGPS(1, RMC_LAT)*100000), 
		(int)(getGPS(1, RMC_LONG)*100000), (int)getGPS(1, RMC_TIME));
	//os_printf("TEST: %d\n", (int)10.0);

	os_printf("decQClat: %d, decGVlat: %d, decQClong: %d, decGVlong: %d\n", 
		(int)(NMEAtoDecimalDegrees(getGPS(1, RMC_LAT))*100000),
		(int)(NMEAtoDecimalDegrees(getGPS(0, RMC_LAT))*100000),
		(int)(NMEAtoDecimalDegrees(getGPS(1, RMC_LONG))*100000),
		(int)(NMEAtoDecimalDegrees(getGPS(0, RMC_LONG))*100000));

	// calculate x distance
	double xdist = calculate_xdistance(NMEAtoDecimalDegrees(getGPS(1, RMC_LAT)), NMEAtoDecimalDegrees(getGPS(0, RMC_LAT)));
	// calculate y distance
	double ydist = calculate_ydistance(NMEAtoDecimalDegrees(getGPS(1, RMC_LONG)), NMEAtoDecimalDegrees(getGPS(0, RMC_LONG)));

	os_printf("xdist: %d, ydist: %d\n", (int)(xdist*1000), (int)(ydist*1000));

	wifi_get_macaddr(STATION_IF, hwaddr);
 	
	//detect adc button push, and increase state
	if(system_adc_read() < 100 & adc_read_last > 100){
		flystate = (flystate + 1)%FLYSTATECOUNT;
	}

	//initialise navigational data if necessary
	if(flystate == FLYSTATE_INITNAV){
		os_sprintf(DeviceBuffer, "AT*CONFIG=\"general:navdata_demo\",\"TRUE\"\\r");
		//os_sprintf(DeviceBuffer, "AT*CONFIG=%d%s%s\r", "\"general:navdata_demo\"","\"TRUE\"");
		flystate++;

	//acknowledge something
	}else if(flystate == FLYSTATE_ACK1){
		os_sprintf(DeviceBuffer, "AT*CTRL=0\r");
		flystate++;

	// tell drone its flying with outdoor shell
	}else if(flystate == FLYSTATE_OUTDOOR){
		os_sprintf(DeviceBuffer, "AT*CONFIG=%d,%s,%s\r", sequence++, "\"control:flight_without_shell\"","\"TRUE\"");
		flystate++;
		
	//AT*CONFIG=605,"control:flight_without_shell","TRUE"
	}else if(flystate == FLYSTATE_OUTDOORSHELL){
		os_sprintf(DeviceBuffer, "AT*CONFIG=%d,%s,%s\r", sequence++, "\"control:outdoor\"","\"TRUE\"");
		flystate++;
		
	// takeoff!
	}else if(flystate == FLYSTATE_TAKEOFF){
		os_sprintf(DeviceBuffer, "%s%d%s\r", ATREF, sequence++, TAKEOFF);
		needsCalibration = 1;

	//operate manual control
	}else if(flystate == FLYSTATE_MANUAL1 | flystate == FLYSTATE_MANUAL2){
		forward = !GPIO_INPUT_GET(5);
		left = !GPIO_INPUT_GET(0);
		right = !GPIO_INPUT_GET(4);
		
		if(needsCalibration){
			os_sprintf(DeviceBuffer, "AT*CALIB=%d,%d\r", sequence++, 0);
			needsCalibration = 0;
		}else{
			speed = forward*neg50asSingle;
			//determine values frim input
			if(left & !right)
				pitch = neg50asSingle;	
			else if(right & !left)
				pitch = d50asSingle;
			else
				pitch = 0;
			os_sprintf(DeviceBuffer, "%s%d,%d,%d,%d,%d,%d\r", ATPCMD, sequence++, (left || right || forward), 
				0, speed,0, pitch);
		//os_sprintf(DeviceBuffer, "%s" MACSTR "!" , ESP8266_MSG, MAC2STR(hwaddr));
		}

	//otherwse perform pathing with reference to GPS data
	}else if(flystate == FLYSTATE_AUTOMATIC)
		os_sprintf(DeviceBuffer, "AT*COMWDG=%d\r", sequence++);

	//send ftrim if grounded
	else if(flystate == FLYSTATE_GROUNDED_START)
		os_sprintf(DeviceBuffer, "AT*FTRIM=%d\r", sequence++);

	//send land command
	else if(flystate == FLYSTATE_GROUNDED_END)
		os_sprintf(DeviceBuffer, "%s%d%s\r", ATREF, sequence++, LAND);
	
	

	//update adc_read_last
 	adc_read_last = system_adc_read();	
	//send the buffer
 	espconn_sent(&user_udp_espconn, DeviceBuffer, os_strlen(DeviceBuffer));
}
 
 /******************************************************************************
	  * FunctionName : user_udp_sent_cb
	  * Description  : udp sent successfully
	  * Parameters  : arg -- Additional argument to pass to the callback function
	  * Returns	  : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_udp_sent_cb(void *arg)
{
	struct espconn *pespconn = arg;
 
	//os_printf("user_udp_send successfully !!!\n");
	 
	//disarm timer first
	os_timer_disarm(&test_timer);

	//re-arm timer to check ip
	os_timer_setfn(&test_timer, (os_timer_func_t *)user_udp_send, NULL); // only send next packet after prev packet sent successfully
	//os_timer_arm(&test_timer, 40, 0);
	os_timer_arm(&test_timer, 50, 0);
}


 /******************************************************************************
 * FunctionName : user_check_ip
 * Description  : check whether get ip addr or not
 * Parameters	: none
 * Returns	  : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_check_ip(void)
{
	struct ip_info ipconfig;

	//disarm timer first
	os_timer_disarm(&test_timer);

	//get ip info of ESP8266 station
	wifi_get_ip_info(STATION_IF, &ipconfig);

	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
	{
	  //os_printf("got ip !!! \r\n");

	  wifi_set_broadcast_if(STATIONAP_MODE); // send UDP broadcast from both station and soft-AP interface

	  user_udp_espconn.type = ESPCONN_UDP;
	  user_udp_espconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	  user_udp_espconn.proto.udp->local_port = 1112;  // set a available  port
	 
	  const char udp_remote_ip[4] = {255, 255, 255, 255}; 
	 
	  os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4); // ESP8266 udp remote IP
	 
	  user_udp_espconn.proto.udp->remote_port = 5556;  // ESP8266 udp remote port
	 
	  espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb); // register a udp packet receiving callback
	  espconn_regist_sentcb(&user_udp_espconn, user_udp_sent_cb); // register a udp packet sent callback
	 
	  espconn_create(&user_udp_espconn);	// create udp

	  user_udp_send();	// send udp data

	  os_timer_arm(&send_timer, 40, 0);
	}
	else
	{
		if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
			wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
			wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
		{
		 //os_printf("connect fail !!! \r\n");
		}
	  else
	  {	
		//os_printf("still waiting...\n");
		//re-arm timer to check ip
		os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
		os_timer_arm(&test_timer, 500, 0);
	}
	}
}


/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters	: none
 * Returns	  : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
	// Wifi configuration
	char ssid[32] = SSID;
	char password[64] = PASSWORD;
	struct station_config stationConf;

	//need not mac address
	stationConf.bssid_set = 0;
	
	//Set ap settings
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 64);
	wifi_station_set_config(&stationConf);

	//set a timer to check whether got ip from router succeed or not.
	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
	os_timer_arm(&test_timer, 100, 0);

	//set send command timer
	os_timer_disarm(&send_timer);
	os_timer_setfn(&send_timer, (os_timer_func_t *)user_udp_send, NULL);
}
