import serial

s = serial.Serial(port = "/dev/ttyUSB0", baudrate = 115200)
if not s.isOpen():
    s.open()


s.write(str.encode("\n"))
s.write(str.encode("\n"))