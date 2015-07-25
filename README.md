#ESP8266 I2S WS2812 Driver 

(For more reliable WS2812 driving...)

This project is based off of the I2S interface for the mp3 player found here:
https://github.com/espressif/esp8266_mp3_decoder/

If you want more information about the build environment, etc.  You should 
check out the regular WS2812 driver, found here: https://github.com/cnlohr/ws2812esp8266

This project is still jankey and needs some cleanup.  The way it currently works is to
continuously send WS2812 frames and when new data comes in on port 7777, it just updates the frames.

I wanted to make it so it would stop doing all DMA proesses so it could save bus contention, but I couldn't figure out how to make that happen.  If I tried disabling and re-enabling, it seems to have caused the DMA to just freak out and turn off until chip reboot.

The WS2812 output is synthesized from the input buffer.   Any time a 0 is to be transmitted, we need a .3us pulse high and .9us low.  If a 1, then we send a .9us high with a .3us low.  The way we do this is we look at nibbles, i.e.:

0101 would convert into:
1000 1110 1000 1110 < bitstream that actually gets shipped out.

Unfortunately this does mean it takes up 4x the space in RAM to DMA the data... But, it is very fast and efficient.

## Hardware Connection

Unfortunately the I2S Output pin is the same as RX1 (pin 25), which means if you are programming via the UART, it'll need to be unplugged any time you're testing.  The positive side of this is that it is a pin that is exposed on most ESP8266 breakout boards.

