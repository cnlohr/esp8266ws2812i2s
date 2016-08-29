# WebUI 

## Range

Somehow include magic bit-sequence, i.e. `#000001` to preserve color in `ws2812_push()`.

## Pattern

The `pattern` and `last_led_count` is stored in `SETTINGS.UserData` of `commonservices.h`. It takes up the first three bytes.

