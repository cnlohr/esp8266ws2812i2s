#include <stdint.h>
#ifndef NO_CTYPES
#include <c_types.h>
#endif
#include "pattern.h"
#include <stdio.h>

float my_fmod(float arg1, float arg2) {
    int full = (int)(arg1/arg2);
    return arg1 - full*arg2;
}

uint64_t HSVtoHEX( float hue, float sat, float value )
{

    float pr,  pg, pb, avg;    pr=pg=pb=avg=0;
    short ora, og, ob;         ora=og=ob=0;

    float ro = my_fmod( hue * 6, 6. );
    ro = my_fmod( ro + 6 + 1, 6 ); //Hue was 60* off...

    //yellow->red
    if     ( ro < 1 ) { pr = 1;         pg = 1. - ro; }
    else if( ro < 2 ) { pr = 1;         pb = ro - 1.; }
    else if( ro < 3 ) { pr = 3. - ro;   pb = 1;       }
    else if( ro < 4 ) { pb = 1;         pg = ro - 3;  }
    else if( ro < 5 ) { pb = 5  - ro;   pg = 1;       }
    else              { pg = 1;         pr = ro - 5;  }

    //Actually, above math is backwards, oops!
    pr *= value;   pg *= value;   pb *= value;
    avg += pr;     avg += pg;     avg += pb;

    pr = pr * sat + avg * (1.-sat);
    pg = pg * sat + avg * (1.-sat);
    pb = pb * sat + avg * (1.-sat);

    ora = pr*255;  og = pb*255;   ob = pg*255;

    if( ora < 0 ) ora = 0;   if( ora > 255 ) ora = 255;
    if( og  < 0 ) og = 0;    if( og  > 255 ) og  = 255;
    if( ob  < 0 ) ob = 0;    if( ob > 255 )  ob  = 255;

    return (ob<<16) | (og<<8) | ora;
}

uint32_t hex_pattern( uint8_t pattern, uint16_t light, uint16_t lights, uint32_t frame, uint8_t clr[3] ) {
    uint32_t hex = 0;

    switch( pattern ) {
        //For button
        case PTRN_CONST: if( clr != NULL ) hex = clr[0] | clr[1]<<8 | clr[2]<<16; break;
        case 1: hex = (light == (frame % lights))?0xFFFFFF:0x000000; break;
        case 2: hex = HSVtoHEX( light*(1/12.) + frame*(1./48.), 1, 1.0 ); break;
        case 3: hex = ((int32_t)((frame+light)%lights)>(lights-2))?0xffffff:0; break; //The worm.
        case 4: hex = HSVtoHEX( light*.03 - frame*.04, 1,  (((((-light<<3)%256) - ((frame<<3)%256)+256)%256) ) /256.0*0.9-0.1); break; //Long, smooth, transitioning. 1.0

        //For wall art.
        case 5: hex = (((frame+light)%186)>160)?0xff8f8f:0; break; //The worm.
        case 6: hex = (((frame+light)%186)>130)?0x0000ff:0; break; //The red worm.
        case 7: hex = HSVtoHEX( light*.005 - frame*.001, 1,  ((((-light%256) - ((frame>>1)%256)+256)%256) ) /256.0*1.2-0.1); break;
        case 8: hex = HSVtoHEX( light*.00500 + ((int)(frame*0.42))*.17, 1, 0.40 ); break;//Fiesta

        //and my favorite:
        case 9: hex = HSVtoHEX( light*.001 - frame*.001, 1,  ((((light%256) - ((frame>>1)%256)+256)%256) ) /256.0*1.5-0.1); break; //Long, smooth, transitioning. 1.0 overloads.  Trying 0.9. If 0.9 works, should back off.

        case 10: hex = HSVtoHEX( light*0.005376344 - frame*.001, 1.3,  1.0); break; //Long, smooth, transitioning. full-brigth

        //Chaser
        case 12: hex = HSVtoHEX( light*.002 + frame*.002, 4, 1.0 );  break; //0.50 = overload. 0.45 = overheat? =0.40 = HOT
        case 13: hex = HSVtoHEX( frame*.001, 1.0,  1.0);  break; //Long, smooth, transitioning. full-brigth
        case PTRN_NONE: break;
        default: hex = HSVtoHEX( light*.05 + frame*.01, 1, 1.0 );
    }

    return hex;
}
