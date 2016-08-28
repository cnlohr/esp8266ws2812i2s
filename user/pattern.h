#ifndef _PATTERN_H
#define _PATTERN_H

uint64_t HSVtoHEX( float hue, float sat, float value );

uint32_t hex_pattern( uint8_t pattern, uint16_t light, uint16_t lights, uint32_t frame );

#endif
