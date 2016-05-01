#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <driver/qmath.h>

#define R 6373	// Radius of the Earth in kilometers (km)
#define PI 3.14159265359 // PI, obviously...
#define LatKM 110.574
#define LongKM 113.320

static double latMid = 0;
static double m_per_deg_lat = 0;
static double m_per_deg_long = 0;

double deg2rad(double deg);

double rad2deg(double rad);

double calculate_distance(double QC_lat, double QC_long, double GV_lat, double GV_long);

double calculate_xdistance(double QC_lat, double GV_lat);

double calculate_ydistance(double QC_long, double GV_long);

double* calculate_xy_distance(double QC_lat, double QC_long, double GV_lat, double GV_long);

double NMEAtoDecimalDegrees(double NMEA);
