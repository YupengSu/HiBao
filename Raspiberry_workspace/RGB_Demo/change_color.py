import colorsys
import time
import random
from rpi_ws281x import PixelStrip, Color
from include import *

# 示例起始颜色和结束颜色
RED = (255, 0, 0)  # 红色
GREEN = (0, 255, 0)  # 红色
BULE = (0, 0, 255)    # 蓝色

# 渐变的总时长和步数
duration = 5  # 总时长为5秒
steps = 50   # 分50步渐变

# Create NeoPixel object with appropriate configuration.
#创建对象
strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)

# Intialize the library (must be called once before other functions).
#初始化
strip.begin()
print("begin successful")

#Toggle all LED light colors
#改变所以灯颜色
def change_color(color, wait_ms=40):
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms / 1000.0)

def cycle_light_effect(start_color, end_color, duration, steps):
    
    start_hls = colorsys.rgb_to_hls(*start_color)
    end_hls = colorsys.rgb_to_hls(*end_color)

    h_increment = (end_hls[0] - start_hls[0]) / steps

    interval = duration / steps

    for i in range(steps):
        # 计算当前步数的H值
        current_h = start_hls[0] + h_increment * i

        # 将HLS值转换回RGB
        current_rgb = colorsys.hls_to_rgb(current_h, start_hls[1], start_hls[2])
        current_rgb = [round(channel) for channel in current_rgb]

        # 在这里执行设置灯光颜色的操作
        # 这里使用print模拟设置灯光颜色
        print(current_rgb)
        change_color(Color(current_rgb[0], current_rgb[1], current_rgb[2]))

try:
    while True:
        cycle_light_effect(RED, BULE, duration, steps)
        cycle_light_effect(BULE, RED, duration, steps)
except:
    pass