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
print("begin successful")

#Toggle all LED light colors
#改变所以灯颜色
def change_color(color, wait_ms=20):
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms / 1000.0)
         
# 呼吸灯
#利用setBrightness()设置灯的亮度 值在0到255之间 包括 0和255
def breathing(color):
    strip.setBrightness(0)
    change_color(color)
    strip.show()
    brightness = 0
    increment = 1
    while True:
        brightness = brightness + increment
        strip.setBrightness(brightness)
        strip.show()
        time.sleep(0.005)
        if brightness <= 0 or brightness >= 255:
            increment = -increment
        
    
try:
    breathing(GREEN)
except:
    pass