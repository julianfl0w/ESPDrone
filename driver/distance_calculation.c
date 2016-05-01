#include "driver/distance_calculation.h"

double ICACHE_FLASH_ATTR deg2rad(double deg)
{
	return (deg * PI / 180);
}

double ICACHE_FLASH_ATTR rad2deg(double rad)
{
	return (rad * 180 / PI);
}
double ICACHE_FLASH_ATTR calculate_distance(double QC_lat, double QC_long, double GV_lat, double GV_long)
{
	double theta, dist;
	theta = QC_long - GV_long;
	dist = qsin(deg2rad(QC_lat)) * qsin(deg2rad(GV_lat)) + qcos(deg2rad(QC_lat)) * qcos(deg2rad(GV_lat)) * qcos(deg2rad(theta));
	dist = qarccos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515 * 1.609344 * 1000;
	return dist;
}

double ICACHE_FLASH_ATTR calculate_xdistance(double QC_lat, double GV_lat)
{
	double xdist;
	latMid = deg2rad((QC_lat + GV_lat)/2);
	m_per_deg_lat = 111132.954 -559.822 * qcos(2 * latMid) + 1.175 * qcos(4 * latMid);
	xdist = (QC_lat - GV_lat) * m_per_deg_lat;
	return xdist;
}

double ICACHE_FLASH_ATTR calculate_ydistance(double QC_long, double GV_long)
{
	if(latMid == 0)
	{
		return 0;
	}
	else
	{
		double ydist;
		m_per_deg_long = 111412.84*qcos(latMid) - 93.5*qcos(3 * latMid) - 0.118 * qcos(5 * latMid);
		ydist = (QC_long - GV_long) * m_per_deg_long;
		return ydist;
	}	
}
double* ICACHE_FLASH_ATTR calculate_xy_distance(double QC_lat, double QC_long, double GV_lat, double GV_long)
{
	double m_per_deg_lat, m_per_deg_long, deltaLat, deltaLon;
	double xdist, ydist;
	static double result[2];
	latMid = deg2rad((QC_lat + GV_lat)/2);
	deltaLon = QC_long - GV_long;
	deltaLat = QC_lat - GV_lat;

	m_per_deg_lat = 111132.954 -559.822 * qcos(2 * latMid) + 1.175 * qcos(4 * latMid);
	m_per_deg_long = 111412.84*qcos(latMid) - 93.5*qcos(3 * latMid) - 0.118 * qcos(5 * latMid);

	//printf("M per degree lat = %lf. M per degree long = %lf.\n", m_per_deg_lat, m_per_deg_long);

	result[0] = deltaLat * m_per_deg_lat;
	result[1] = deltaLon * m_per_deg_long;
	
	//printf("xdistance = %lf meters\nydistance = %lf meters\n", xdist, ydist);
	return result;
}

double ICACHE_FLASH_ATTR NMEAtoDecimalDegrees(double NMEA)
{
	int b;
	double c;

	b = NMEA/100;
	c = (NMEA/100 - b)*100;
	c /= 60;
	c += b;
	return c;
}


