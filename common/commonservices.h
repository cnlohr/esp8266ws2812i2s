//Copyright 2015 <>< Charles Lohr Under the MIT/x11 License, NewBSD License or
// ColorChord License.  You Choose.

#ifndef _COMMON_H
#define _COMMON_H

#include "c_types.h"

#define FLASH_PROTECTION_BOUNDARY 524288
#define MAX_DEVICE_NAME 32
#define USERDATA_SIZE 256

//Returns nr bytes to return.  You must allocate retdata.
//It MUST be at least 1,300 bytes large and it MUST be 32-bit aligned.
//NOTE: It is SAFE to use pusrdata and retdata as the same buffer.
int ICACHE_FLASH_ATTR issue_command(char * retdata, int retsize, char *pusrdata, unsigned short len);

//Includes UDP Control + HTTP Interfaces
void ICACHE_FLASH_ATTR CSPreInit();
void ICACHE_FLASH_ATTR CSInit();
void ICACHE_FLASH_ATTR CSTick( int slowtick );
void ICACHE_FLASH_ATTR CSConnectionChange();

void ICACHE_FLASH_ATTR CSSettingsLoad(int force_reinit);
void ICACHE_FLASH_ATTR CSSettingsSave();

struct CommonSettings
{
	uint8_t settings_key; //Needs to be 0xAF
	char DeviceName[MAX_DEVICE_NAME];
	char DeviceDescription[MAX_DEVICE_NAME];
	char UserData[USERDATA_SIZE];
};

extern struct CommonSettings SETTINGS;


//You must provide:
//Critical should not lock interrupts, just disable services that have problems
//with double-interrupt faults.  I.e. turn off/on any really fast timer interrupts.
//These generally only get called when doing serious operations like reflashing.
void EnterCritical();
void ExitCritical();

//If we receive a command that's not F, E or W (Flash Echo Wifi)
int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len);

//Other utility stuff
extern int need_to_switch_opmode; //0 = no, 1 = will need to after a scan. 2 = do it now. 3 = switched, need to start dhcps.  4 = need to go back into station mode.


#endif

