//#include <math.h>
#include <string.h>   
#include <stdlib.h>
#include "driver/parse.h"

#define PI 3.14159265

char buffer[GPS_COUNT][BUFFLENGTH]; 
int bufIndex[] = {0, 0};

double GPSdata[GPS_COUNT][GPS_NUMVARS];

void process(char c, int uartNo){

	int i = 0;
	// load into array if not linefeed
	if(c != '\n' & c != '$')
		buffer[uartNo][bufIndex[uartNo]++] = c;
	// restart buffer if $ recieved
	else if(c == '$')
		clearbuffer(uartNo);
	// otherwise process the buffer!
	else if(c == '\n'){
		char * token;
		char * dummy;
		//extract the checksum
		token = strtok(buffer[uartNo], "*");
		token = strtok(NULL, "\n");

		//if there is no checksum, abort!
		if(token == NULL){
			clearbuffer(uartNo);
			return;
		}
	
		//convert checksum to int
		int checksum = strtol(token, &dummy, 16);

		//check the checksum
		unsigned char runningCS = 0;
		for(i = 0; i < strlen(buffer[uartNo]); i++)
			runningCS = runningCS^buffer[uartNo][i];

		//if wrong checksum, abort!
		if(runningCS != checksum){
			clearbuffer(uartNo);
			return;
		}

		//extract the type
		token = strtok(buffer[uartNo], ",");
		if(token == NULL){
			clearbuffer(uartNo);
			return;
		}

		int savePos = 0;
		if(!strcmp(token, "GPRMC")){
			savePos = RMC_START;
		}else{
			clearbuffer(uartNo);
			return;
		}	
		//else if(!strcmp(token, "PGRMZ")){
		//	savePos = RMZ_START;
		//}

		//until we reach the checksum token
		while(1){
			token = strtok( NULL, ",");	
		
			//token == null marks completion
			if(token == NULL){
				clearbuffer(uartNo);
				return;
			}
			//save into the array!
			GPSdata[uartNo][savePos++] = stof(token);
			//GPSdata[uartNo][savePos++] = 1;

			//make special arrangements for char inputs
			if(!strcmp(token, "N"))
				GPSdata[uartNo][savePos-1] = F_NORTH;
			else if(!strcmp(token, "E"))
				GPSdata[uartNo][savePos-1] = F_EAST;
			else if(!strcmp(token, "W"))
				GPSdata[uartNo][savePos-1] = F_WEST;
			else if(!strcmp(token, "S"))
				GPSdata[uartNo][savePos-1] = F_SOUTH;
			//special case: V indicates invalid input
			else if(!strcmp(token, "V")){
				GPSdata[uartNo][savePos-1] = F_WARNING;
				clearbuffer(uartNo);
				return;
			}
			else if(!strcmp(token, "A"))
				GPSdata[uartNo][savePos-1] = F_NOWARN;
				
		}
	}
}

void clearbuffer(int uartNo){
	int i;
	bufIndex[uartNo] = 0;
	for(i = 0; i < BUFFLENGTH; i++)
		buffer[uartNo][i] = '0';
}

double getGPS(int uartNo, int value){
	return GPSdata[uartNo][value];
}

double stof(const char* s){
  double rez = 0, fact = 1;
  int point_seen;
  if (*s == '-'){
    s++;
    fact = -1;
  };
  for (point_seen = 0; *s; s++){
    if (*s == '.'){
      point_seen = 1; 
      continue;
    };
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) fact /= 10.0f;
      rez = rez * 10.0f + (double)d;
    };
  };
  return rez * fact;
};
