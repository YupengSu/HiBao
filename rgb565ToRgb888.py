from PIL import Image
import numpy as np
from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtGui import QPixmap, QImage
import serial
import matplotlib.pyplot as plt
import time
import cv2
import threading

imshape = (240,320,3)
imdata = np.zeros(imshape)

def rgb565Torgb(data: list):
    global imdata
    for h in range(0, 240):
        for w in range(0, 320):
            i = h * 320 + w
            pixel = (data[2 * i] << 8) | data[2 * i + 1]
            imdata[h,w,2] = (pixel >> 11) / 31.0 # R
            imdata[h,w,1] = ((pixel >> 5) & 0x3f) / 63.0 # G
            imdata[h,w,0] = (pixel & 0x1f) / 31.0 # B
    return 0

f1 = open( 'E:/VIDEO011.RGB', 'r')
read = f1.read()
f1.close()

rgb565Torgb(read)

cv2.imshow("Demo", imdata) 
cv2.waitKey(0)

