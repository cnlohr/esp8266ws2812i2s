//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "espconn.h"
#include "esp82xxutil.h"
#include "ws2812_i2s.h"
#include "commonservices.h"
#include <mdns.h>
#include "vars.h"
#include "pattern.h"

#define procTaskPrio        0
#define procTaskQueueLen    1

static volatile os_timer_t some_timer;
static volatile os_timer_t pattern_timer;
static struct espconn *pUdpServer;
usr_conf_t * UsrCfg = (usr_conf_t*)(SETTINGS.UserData);
uint8_t last_leds[512*3] = {0};
uint32_t frame = 0;


//int ICACHE_FLASH_ATTR StartMDNS();


void ICACHE_FLASH_ATTR user_rf_pre_init(void) {/*nothing.*/}


char * ICACHE_FLASH_ATTR strcat( char * dest, char * src ) { return strcat(dest, src ); }



//Tasks that happen all the time.
os_event_t    procTaskQueue[procTaskQueueLen];
static void ICACHE_FLASH_ATTR procTask(os_event_t *events)
{
	CSTick( 0 );
	system_os_post(procTaskPrio, 0, 0 );
}


//Display pattern on connected LEDs
static void ICACHE_FLASH_ATTR patternTimer(void *arg)
{
    if(UsrCfg->ptrn == PTRN_NONE) return;

    int it;
    for(it=0; it<UsrCfg->nled; ++it) {
        uint32_t hex = hex_pattern( UsrCfg->ptrn, it, UsrCfg->nled, frame, UsrCfg->clr );
        last_leds[3*it+0] = (hex>>8);
        last_leds[3*it+1] = (hex);
        last_leds[3*it+2] = (hex>>16);
    }
    frame++;
    debug("Frame: %i", (int)frame);
    ws2812_push( (char*)last_leds, 3*UsrCfg->nled);
}


//Timer event.
static void ICACHE_FLASH_ATTR myTimer(void *arg)
{
	CSTick( 1 );
}


//Called when new packet comes in.
static void ICACHE_FLASH_ATTR
udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
    UsrCfg->ptrn = PTRN_NONE;
	struct espconn *pespconn = (struct espconn *)arg;

	uart0_sendStr("X");

	ws2812_push( pusrdata+3, len-3 );

	len -= 3;
	if( len > sizeof(last_leds) + 3 )
		len = sizeof(last_leds) + 3;
	ets_memcpy( last_leds, pusrdata+3, len );
	UsrCfg->nled = len / 3;
}


void ICACHE_FLASH_ATTR charrx( uint8_t c ) {/*Called from UART.*/}

void  ICACHE_FLASH_ATTR umcall( void );

void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	uart0_sendStr("\r\nesp82XX Web-GUI\r\n" VERSSTR "\b");

	umcall();
}

void  ICACHE_FLASH_ATTR umcall( void )
{
//Uncomment this to force a system restore.
//	system_restore();

	CSSettingsLoad( 0 );
    CSPreInit();

    pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	ets_memset( pUdpServer, 0, sizeof( struct espconn ) );
	espconn_create( pUdpServer );
	pUdpServer->type = ESPCONN_UDP;
	pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	pUdpServer->proto.udp->local_port = COM_PORT;
	espconn_regist_recvcb(pUdpServer, udpserver_recv);

	if( espconn_create( pUdpServer ) )
		while(1)
            uart0_sendStr( "\r\nFAULT\r\n" );

	CSInit();

	//Set GPIO16 for INput
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
		(READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1);     // mux configuration for XPD_DCDC and rtc_gpio0 connection

	WRITE_PERI_REG(RTC_GPIO_CONF,
		(READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0); //mux configuration for out enable

	WRITE_PERI_REG(RTC_GPIO_ENABLE,
		READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);       //out disable

/*
	SetServiceName( "ws2812" );
	AddMDNSName( "esp82xx" );
	AddMDNSName( "ws2812" );
	AddMDNSService( "_http._tcp", "An ESP8266 Webserver", WEB_PORT );
	AddMDNSService( "_ws2812._udp", "WS2812 Driver", COM_PORT );
	AddMDNSService( "_esp82xx._udp", "ESP8266 Backend", BACKEND_PORT );
*/
	//Add a process
	system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);

	//Timer example
	os_timer_disarm(&some_timer);
	os_timer_setfn(&some_timer, (os_timer_func_t *)myTimer, NULL);
	os_timer_arm(&some_timer, 100, 1);

	//Pattern Timer example
	os_timer_disarm(&pattern_timer);
	os_timer_setfn(&pattern_timer, (os_timer_func_t *)patternTimer, NULL);
	os_timer_arm(&pattern_timer, 20, 1); //~50 Hz

	ws2812_init();

	printf( "Boot Ok.\n" );

//	wifi_set_sleep_type(LIGHT_SLEEP_T);
//	wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);

	system_os_post(procTaskPrio, 0, 0 );
}


//There is no code in this project that will cause reboots if interrupts are disabled.
void EnterCritical() {}
void ExitCritical() {}

//For SDK 2.0.0 only.
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

