#include <mem.h>

#define GPS_GRND  0
#define GPS_AIR   1
#define GPS_COUNT 2

#define RMC_START   0
#define RMC_TIME    0
#define RMC_WARNING 1
#define RMC_LAT     2
#define RMC_LATD    3
#define RMC_LONG    4
#define RMC_LONGD   5
#define RMC_SPEED   6
#define RMC_CMG     7
#define RMC_DATE    8
#define RMC_MAGVAR  9
#define RMC_MAGVARD 10
#define RMZ_START   11
#define RMZ_ALT     11
#define RMZ_UNIT    12
#define RMZ_TYPE    13
#define GPS_NUMVARS 14

#define F_NORTH   0
#define F_SOUTH   1
#define F_EAST    2
#define F_WEST    3
#define F_NOWARN  0
#define F_WARNING 1

#define BUFFLENGTH 85

void process(char c, int uartNo);
void clearbuffer(int uartNo);
double getGPS(int uartNo, int value);
double stof(const char* s);
