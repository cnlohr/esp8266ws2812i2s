#ESP8266 I2S WS2812 Driver 

Provides more reliable WS2812 driving... And a nifty websockets-based interface for the ESP8266.  
If you want to watch a video about this project, click the video link below.

[![Youtube: Using I2S on the ESP8266 to drive WS2812Bs](http://img.youtube.com/vi/6zqGwxqJQnw/0.jpg)](http://www.youtube.com/watch?v=6zqGwxqJQnw)

This project is based off of the Espressif I2S interface for an [mp3 player](https://github.com/espressif/esp8266_mp3_decoder/)

If you want more information about the build environment, etc.  You should check out the [regular WS2812 driver](https://github.com/cnlohr/ws2812esp8266).

This project is still jankey and needs some cleanup.  
The way it currently works is to continuously send WS2812 frames and when new data comes in on port 7777, it just updates the frames.

I wanted to make it so it would stop doing all DMA proesses so it could save bus contention, 
but I couldn't figure out how to make that happen.  
If I tried disabling and re-enabling,
it seems to have caused the DMA to just freak out and turn off until chip reboot.

The WS2812 output is synthesized from the input buffer. 
Any time a 0 is to be transmitted, we need a .3us pulse high and .9us low.  
If a 1, then we send a .9us high with a .3us low.  The way we do this is we look at nibbles, i.e.:

0101 would convert into:
1000 1110 1000 1110 < bitstream that actually gets shipped out.

Unfortunately this does mean it takes up 4x the space in RAM to DMA the data... But, it is very fast and efficient.

## Hardware Connection

Unfortunately the I2S Output pin is the same as RX1 (pin 25), which means if you are programming via the UART, it'll need to be unplugged any time you're testing.  The positive side of this is that it is a pin that is exposed on most ESP8266 breakout boards.

## Memory layout:

We use a bit of a modified take on the old memory layout since, while we support OTA upgrades, we do so in a different way than the normal way. Not that HTTP is limited on parts with <1M, and OTA flashing is not supported at all on parts with <512kB.

| Address | Size  | Name / Description            |
| ------- |:-----:| ----------------------------- |
| 00000h  | 64k   | 0x00000.bin, IRAM Code        | 
| 10000h  | 172k  | Normally unused, HTTP may be here if signature found. Only used if you have < 1M part. |
| 3A000h  | 16k   | Device Configuration          |
| 3E000h  | 8k    | May be used by ESP SDK.       |
| 40000h  | 240k  | 0x40000.bin, Cached code.     |
| 7C000h  | 8k    | May be used by ESP SDK.       |
| 7E000h  | 8k    | May be WiFi configuration     |
| 80000h  | 512k  | Scratchpad (Temp only!)       |
| 100000h | 1M+   | HTTP data, 1M on W25Q16.  HTTP Used here if signature found.       |

