from rpi_ws281x import Color

LED_COUNT = 14        # LED灯的个数
LED_PIN = 18        # DI端接GPIO18

# 以下可以不用改
LED_FREQ_HZ = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA = 10          # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 100  # Set to 0 for darkest and 255 for brightest
LED_INVERT = False    # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53

# color setting
RED = Color(255,0,0)
ORANGE = Color(255,165,0)
YELLOW = Color(255,255,0)
GREEN = Color(0,255,0)
CYAN = Color(0,255,255)
BULE = Color(0,0,255)
PURPLE = Color(225,0,225)
WHITE = Color(255,255,255)
BLACK = Color(0,0,0)
GRAY = Color(128,128,128)
BROWN = Color(165,42,42)


