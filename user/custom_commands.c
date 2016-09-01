//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include <esp82xxutil.h>
#include <commonservices.h>
#include "ws2812_i2s.h"
#include "vars.h"
#include "pattern.h"

int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len)
{
	char * buffend = buffer;

	switch( pusrdata[1] )	{
    	case 'C': case 'c': { //Custom command test
    		buffend += ets_sprintf( buffend, "CC" );
    		return buffend-buffer;
    	} break;

    	case 'l': case 'L': { //LEDs
    		int i, it = 0;
    		buffend += ets_sprintf( buffend, "CL:%d:", UsrCfg->nled );
    		uint16_t toledsvals = UsrCfg->nled*3;
    		if( toledsvals > 600 ) toledsvals = 600;
    		for( i = 0; i < toledsvals; i++ )
    		{
    			uint8_t samp = last_leds[it++];
    			*(buffend++) = tohex1( samp>>4 );
    			*(buffend++) = tohex1( samp&0x0f );
    		}
    		return buffend-buffer;
    	} break;

        case 'T': case 't': { // set led-ranges over Web-UI
            #ifdef DEBUG
                printf("Color Data: ");
                int it;
                for(it=3; it<len; ++it)
                    printf("%x ", pusrdata[it]);
                printf("\n");
            #endif
            ws2812_push(pusrdata+3,len-3);
            ets_memcpy( last_leds, pusrdata+3, len );
            UsrCfg->nled = (len-1) / 3;
            UsrCfg->ptrn = PTRN_NONE;
            frame = 0;
        } break;


        case 'P': case 'p': { // set pattern to be repeated
            if( len<5 ) break;
            frame = 0;
            UsrCfg->ptrn = (uint8_t)pusrdata[2];
            uint8_t c1 = (uint8_t)pusrdata[3];
            uint8_t c2 = (uint8_t)pusrdata[4];
            UsrCfg->nled = (uint16_t)(c1<<8) + (uint16_t)c2;
            if( len>7 ) ets_memcpy(UsrCfg->clr, (uint8_t*)(pusrdata+5), 3);
            debug( "len: %u; ptrn: %i, nled: %i, clr: %02x%02x%02x\n", len, UsrCfg->ptrn, UsrCfg->nled, (int)pusrdata[5], (int)pusrdata[6], (int)pusrdata[7] );
            CSSettingsSave();
            return buffend-buffer;
        } break;

	}
	return -1;
}
