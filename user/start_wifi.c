/******************************************************************************
     * Copyright 2013-2014 Espressif Systems
     *
*******************************************************************************/
#include "start_wifi.h"  
#include "gpio.h"
LOCAL os_timer_t test_timer;
LOCAL os_timer_t send_timer;
LOCAL struct espconn user_udp_espconn;
LOCAL struct espconn user_udp_espconn_rx;
int sequence = 0;
int flystate = 0;
uint8_t initNavData = 1;
int adc_read_last = 1000;
const char *ESP8266_MSG = "I'm ESP8266 ";
const char *ATREF = "AT*REF=";
const char *ATPCMD = "AT*PCMD=";
const char *TAKEOFF = ",290718208";
const char *LAND = ",290717696";
  
#define FLYSTATE_GROUNDED  0
#define FLYSTATE_MANUAL1   1
#define FLYSTATE_AUTOMATIC 2
#define FLYSTATE_MANUAL2   3
#define FLYSTATECOUNT 4

/*---------------------------------------------------------------------------*/
LOCAL struct espconn ptrespconn;

 /******************************************************************************
  * FunctionName : user_udp_recv_cb
  * Description  : Processing the received udp packet
  * Parameters   : arg -- Additional argument to pass to the callback function
  *                pusrdata -- The received data (or NULL when the connection has been closed!)
  *                length -- The length of received data
  * Returns      : none
 *******************************************************************************/
 LOCAL void ICACHE_FLASH_ATTR
 user_udp_recv_cb(void *arg, char *pusrdata, unsigned short length)
 {
     
     os_printf("recv udp data: %s\n", pusrdata);

 }


uint8_t forward, left, right;
 /******************************************************************************
      * FunctionName : user_udp_send
      * Description  : udp send data
      * Parameters  : none
      * Returns      : none
 *******************************************************************************/

//this function is called every 50ms. it sends the appropriate command to the quadcoper
 LOCAL void ICACHE_FLASH_ATTR
 user_udp_send(void)
 {
     char DeviceBuffer[40] = {0};
     //char hwaddr[6];
     struct ip_info ipconfig;

     const char udp_remote_ip[4] = { 255, 255, 255, 255}; 
     os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4); // ESP8266 udp remote IP need to be set everytime we call espconn_sent
     user_udp_espconn.proto.udp->remote_port = 5556;  // ESP8266 udp remote port need to be set everytime we call espconn_sent

     //wifi_get_macaddr(STATION_IF, hwaddr);
 	
	//detect adc button push, and increase state
	if(system_adc_read() < 100 & adc_read_last > 100){
		if(flystate == 0)
			os_sprintf(DeviceBuffer, "%s%d%s\r", ATREF, sequence++, TAKEOFF);
		flystate = (flystate + 1)%FLYSTATECOUNT;

	//initialise navigational data if necessary
	}else if(initNavData){
		initNavData = 0;
		os_sprintf(DeviceBuffer, "AT*CONFIG=\"general:navdata_demo\",\"TRUE\"\\r");

	//operate manual control
	}else if(flystate == FLYSTATE_MANUAL1 | flystate = FLYSTATE_MANUAL2){
	    forward = !GPIO_INPUT_GET(5);
        left = !GPIO_INPUT_GET(0);
        right = !GPIO_INPUT_GET(4);
		
		os_sprintf(DeviceBuffer, "%s%d,%d,%d,%d,%d,%d\r", ATPCMD, sequence++, (left || right || forward), 0, -forward * 1090519040,
			0, -left * 1090519040 + right * 1090519040);
		//os_sprintf(DeviceBuffer, "%s" MACSTR "!" , ESP8266_MSG, MAC2STR(hwaddr));

	//send ftrim if grounded
	}else if(flystate == FLYSTATE_GROUNDED){
		os_sprintf(DeviceBuffer, "AT*FTRIM=%d\r", sequence++);
		os_printf("sending %s\n", DeviceBuffer);

	//otherwse perform pathing with reference to GPS data
	}else if(flystate == FLYSTATE_AUTOMATIC){
		//INSERT AUTOPATHING HERE
	}

	//update adc_read_last
 	adc_read_last = system_adc_read();	
	//send the buffer
 	espconn_sent(&user_udp_espconn, DeviceBuffer, os_strlen(DeviceBuffer));
}
 
 /******************************************************************************
      * FunctionName : user_udp_sent_cb
      * Description  : udp sent successfully
      * Parameters  : arg -- Additional argument to pass to the callback function
      * Returns      : none
 *******************************************************************************/
  LOCAL void ICACHE_FLASH_ATTR
  user_udp_sent_cb(void *arg)
  {
      struct espconn *pespconn = arg;
 
      os_printf("user_udp_send successfully !!!\n");
     
      //disarm timer first
       os_timer_disarm(&test_timer);

      //re-arm timer to check ip
      os_timer_setfn(&test_timer, (os_timer_func_t *)user_udp_send, NULL); // only send next packet after prev packet sent successfully
      os_timer_arm(&test_timer, 50, 0);
  }


 /******************************************************************************
 * FunctionName : user_check_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
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
      os_printf("got ip !!! \r\n");

      wifi_set_broadcast_if(STATIONAP_MODE); // send UDP broadcast from both station and soft-AP interface

      user_udp_espconn.type = ESPCONN_UDP;
      user_udp_espconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
      user_udp_espconn.proto.udp->local_port = 5554;  // set a available  port
     
      const char udp_remote_ip[4] = {255, 255, 255, 255}; 
     
      os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4); // ESP8266 udp remote IP
     
      user_udp_espconn.proto.udp->remote_port = 5556;  // ESP8266 udp remote port
     
      espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb); // register a udp packet receiving callback
      espconn_regist_sentcb(&user_udp_espconn, user_udp_sent_cb); // register a udp packet sent callback
     
      espconn_create(&user_udp_espconn);   // create udp

      user_udp_send();   // send udp data

      os_timer_arm(&send_timer, 50, 0);
    }
   else
   {
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
        {
         os_printf("connect fail !!! \r\n");
        }
      else
      {	
		os_printf("still waiting...\n");
           //re-arm timer to check ip
            os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
            os_timer_arm(&test_timer, 500, 0);
        }
    }
}


/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
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
