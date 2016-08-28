//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include <esp82xxutil.h>
#include <commonservices.h>
#include "ws2812_i2s.h"
#include "vars.h"

int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len)
{
	char * buffend = buffer;

	switch( pusrdata[1] )
	{
	case 'C': case 'c': //Custom command test
	{
		buffend += ets_sprintf( buffend, "CC" );
		return buffend-buffer;
	}

	case 'l': case 'L': //LEDs
	{
		int i, it = 0;
		buffend += ets_sprintf( buffend, "CL:%d:", last_led_count );
		uint16_t toledsvals = last_led_count*3;
		if( toledsvals > 600 ) toledsvals = 600;
		for( i = 0; i < toledsvals; i++ )
		{
			uint8_t samp = last_leds[it++];
			*(buffend++) = tohex1( samp>>4 );
			*(buffend++) = tohex1( samp&0x0f );
		}
		return buffend-buffer;
	}

    case 'T': case 't': { // set led-ranges over Web-UI
        printf("Color Data: ");
        int it;
        for(it=3; it<len; ++it)
            printf("%x ", pusrdata[it]);
        printf("\n");
        ws2812_push(pusrdata+3,len-3);
        ets_memcpy( last_leds, pusrdata+3, len );
        last_led_count = (len-1) / 3;
        pattern = PATTERN_NONE;
        frame = 0;
    } break;


    case 'P': case 'p': { // set pattern to be repeated
        if( len < 5 || pusrdata[3] > '9' || pusrdata[3]<'0' ) break;
        pusrdata+=3;
        int tmp_pattern = safe_atoi(pusrdata);
        unsigned it;
        for(it=0; it<len-3 && *pusrdata !='\t'; ++it);
        pusrdata+=it;
        last_led_count = safe_atoi(pusrdata);
        printf("Pattern: %i; NumLeds: %i", tmp_pattern, last_led_count);
        frame = 0;
    } break;

	}
	return -1;
}
