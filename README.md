# ESPDrone

## File Breakdown
1. user_main.c : initializes GPIO, UARTs, GPSs, etc. Is started at boot. 
2. qwifi.c : establishes WiFi connection to QC. Controls the state machine defined in Flystates below. Sends commands to the QC every 50mS.
3. /driver/parse.c : code to store and parse GPRMC strings. Called character-wise by the UART and SoftSerial interrupts.
4. /driver/distance_calculation.c : finds distance between two GPS reads
5. /driver/qmath.c : fills gaps in ESP SDK support for trig functions. 

## Installation: 

1. Get Espressif SDK from http://bbs.espressif.com/viewtopic.php?f=46&t=1702

2. install using autotool, etc. (refer to its readme)
3. navigate to extracted folder
4. run *git clone https://github.com/jcloiacon/ESPDrone*

5. on windows, run *gen_misc.bat*
on linux, run *./gen_misc.sh*

6. run flash with ESP8266 attached
