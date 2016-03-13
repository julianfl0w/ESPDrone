# ESPDrone

## Installation: 

1. Get Espressif SDK from http://bbs.espressif.com/viewtopic.php?f=46&t=1702

2. install 
3. navigate to extracted folder
4. git clone https://github.com/jcloiacon/ESPDrone

5. on windows
run generate.bat

on linux 
run generate.sh

6. run flash with ESP8266 attached

## Specific installation (from SDK)
STEP 1: Copy driver and include folders to your project sub-folder, such as app folder. Unused drivers can be removed in your project.

STEP 2: Modify Makefile in app folder.
    1). Search SUBDIRS, add driver as subdir:
        SUBDIRS=    \
	    user    \
	    driver

    2). Search COMPONENTS_eagle.app.v6, add libdriver.a:
        COMPONENTS_eagle.app.v6 = \
	   user/libuser.a  \
	   driver/libdriver.a
