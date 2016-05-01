#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <driver/qmath.h>
#include <c_types.h>
#define R 6373	// Radius of the Earth in kilometers (km)
#define PI 3.14159265359 // PI, obviously...
#define LatKM 110.574
#define LongKM 113.320

static double latMid = 0;
static double m_per_deg_lat = 0;
static double m_per_deg_long = 0;

double ICACHE_FLASH_ATTR deg2rad(double deg);

double ICACHE_FLASH_ATTR rad2deg(double rad);

double ICACHE_FLASH_ATTR calculate_distance(double QC_lat, double QC_long, double GV_lat, double GV_long);

double ICACHE_FLASH_ATTR calculate_xdistance(double QC_lat, double GV_lat);

double ICACHE_FLASH_ATTR calculate_ydistance(double QC_long, double GV_long);

double* ICACHE_FLASH_ATTR calculate_xy_distance(double QC_lat, double QC_long, double GV_lat, double GV_long);

double ICACHE_FLASH_ATTR NMEAtoDecimalDegrees(double NMEA);
