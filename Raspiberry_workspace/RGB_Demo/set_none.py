import time
import random
from rpi_ws281x import PixelStrip, Color
from include import *

# Create NeoPixel object with appropriate configuration.
#创建对象
strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)

# Intialize the library (must be called once before other functions).
#初始化
strip.begin()
print("Set none!")


#Toggle all LED light colors
#改变所以灯颜色
def change_color(color, wait_ms=20):
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms / 1000.0)

change_color(Color(0,0,0))