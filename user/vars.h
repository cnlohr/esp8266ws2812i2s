#ifndef _VARS_H
#define _VARS_H

#ifndef WS_SLEEP
    #define WS_SLEEP 14000
#endif
#define PATTERN_NONE 255

extern uint8_t pattern;
extern uint32_t frame;
extern uint32_t ws_sleep;

extern uint8_t last_leds[];
extern int last_led_count;

#endif