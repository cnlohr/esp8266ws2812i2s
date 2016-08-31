#ifndef _VARS_H
#define _VARS_H

#include "commonservices.h"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

typedef struct {
    uint8_t ptrn;    ///< Pattern to display on connected leds
    uint16_t nled;   ///< Number of connected LEDs
    uint8_t clr[3];  ///< Color data (used for PATTERN_CONST)
    // ... add more here ...
} __attribute__((__packed__)) usr_conf_t;
BUILD_BUG_ON(sizeof(usr_conf_t>USERDATA_SIZE)) // Produce invalid code if struct is too large ("best" way of compile time size checking)

extern usr_conf_t * UsrCfg;   ///< User data preserved between esp boots

extern uint8_t pattern;       ///< ID of the pattern, see `pattern.c`
extern uint32_t frame;        ///< Temporal position in pattern period
extern uint8_t last_leds[];   ///< Current LED color data
extern int last_led_count;    ///< Number of connected LEDs

#endif
