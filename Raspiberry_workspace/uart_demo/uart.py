# -*- coding: utf-8 -*
import serial
import time
from Speech import Speech
import os
import requests

speech = Speech()

STATE_INFRARED = 0
STATE_RECONGNIZE = 1
STATE_SPEAK = 2

STATE = STATE_INFRARED

ser_T1 = serial.Serial("/dev/ttyAMA0",115200)
ser_T2 = serial.Serial("/dev/ttyUSB0",115200)
ser_P = serial.Serial("/dev/ttyAMA2",9600)

if not ser_T1.isOpen():
    print("open failed")
else:
    print("open success: ")
    print(ser_T1)

if not ser_T2.isOpen():
    print("open failed")
else:
    print("open success: ")
    print(ser_T2)

if not ser_P.isOpen():
    print("open failed")
else:
    print("open success: ")
    print(ser_P)

count_p = [0,0,0]

count_t = [0,0,0,0]

User_dict = {
    "0": "陌生人",
    "1": "彭冠旗",
    "2": "杨嘉琪",
    "3": "苏宇鹏"
}

try:
    os.system("sudo pkill -9 -f green_breath.py")
    os.system("sudo pkill -9 -f blue_speak.py")
    os.system("sudo pkill -9 -f change_color.py")
    os.system("sudo pkill -9 -f red_spark.py")
    os.system("sudo python3 ~/HiBao_Project/RGB_Demo/set_none.py")
    os.popen("bash ~/HiBao_Project/RGB_Demo/change_color.sh","r")
    while True:
        if STATE == STATE_INFRARED:
            rec0 = ser_P.read(1)
            if(rec0==b'\xaa'):
                recv = ser_P.read(7)
                s = str(recv[1])
                print(s)
                count_p[int(s)-1] += 1
                if (10 in count_p):
                    index = count_p.index(10)
                    if index == 0:
                        STATE = STATE_RECONGNIZE

                        speech.SetReader(speech.Reader_Type["Reader_XuXiaoBao"])
                        speech.SetVolume(10)
                        speech.Speech_text("主人，我看到你了！",speech.EncodingFormat_Type["GB2312"])
                        while speech.GetChipStatus() != speech.ChipStatus_Type['ChipStatus_Idle']:
                            time.sleep(0.1) 

                        # open the recognition
                        ser_T2.write(str.encode("cd userdata/\n"))
                        time.sleep(0.5)
                        ser_T2.write(str.encode("./myapp 1\n"))

                        print("Open Successful!")
                    elif index == 1:
                        # find Hibao
                        os.system("sudo pkill -9 -f green_breath.py")
                        os.system("sudo pkill -9 -f blue_speak.py")
                        os.system("sudo pkill -9 -f change_color.py")
                        os.system("sudo pkill -9 -f red_spark.py")
                        os.system("sudo python3 ~/HiBao_Project/RGB_Demo/set_none.py")
                        os.popen("bash ~/HiBao_Project/RGB_Demo/red_spark.sh")
                        
                        speech.SetReader(speech.Reader_Type["Reader_XuXiaoBao"])
                        speech.SetVolume(10)
                        speech.Speech_text("主人，我在这里！",speech.EncodingFormat_Type["GB2312"])
                        while speech.GetChipStatus() != speech.ChipStatus_Type['ChipStatus_Idle']:
                            time.sleep(0.1) 
                        time.sleep(1)
                    elif index == 2:
                        # welcome back
                        speech.SetReader(speech.Reader_Type["Reader_XuXiaoBao"])
                        speech.SetVolume(10)
                        speech.Speech_text("主人，欢迎回家！",speech.EncodingFormat_Type["GB2312"])
                        while speech.GetChipStatus() != speech.ChipStatus_Type['ChipStatus_Idle']:
                            time.sleep(0.1) 
                    else:
                        pass
                    count_p = [0,0,0]
                else:
                    pass
            else:
                pass

        elif STATE == STATE_RECONGNIZE:
            reco = ser_T1.read(1)
            if(reco==b'\xaa'):
                recv = ser_T1.read(10)
                print(recv)
                s = str(recv[4])
                print(s)
                if s != '0':
                    count_t[int(s)] += 1
                if (10 in count_t):
                    index = count_t.index(10)

                    ser_T2.write(str.encode("\n"))
                    ser_T2.write(str.encode("\n"))
                    ser_T2.write(str.encode("cd .. \n"))

                    print("Close Successful!")

                    s = str(index).encode(encoding='UTF-8')
                    for i in range(20):
                        ser_P.write(s)
                    
                    print(requests.post("http://192.168.87.159:8999/changeUser", data={"text" : str(index)}).json()["text"])

                    speech.SetReader(speech.Reader_Type["Reader_XuXiaoBao"])
                    speech.SetVolume(10)
                    speech.Speech_text("你好呀" + User_dict[str(index)] + "，我可以和你聊天！",speech.EncodingFormat_Type["GB2312"])
                    while speech.GetChipStatus() != speech.ChipStatus_Type['ChipStatus_Idle']:
                        time.sleep(0.1)

                    count_t = [0,0,0,0]
                    STATE = STATE_SPEAK
                print(s)
        elif STATE == STATE_SPEAK:
            os.system('bash ~/HiBao_Project/02_iat_open.sh')
            STATE = STATE_INFRARED
            os.system("sudo pkill -9 -f green_breath.py")
            os.system("sudo pkill -9 -f blue_speak.py")
            os.system("sudo pkill -9 -f change_color.py")
            os.system("sudo pkill -9 -f red_spark.py")
            os.system("sudo python3 ~/HiBao_Project/RGB_Demo/set_none.py")
            os.popen("bash ~/HiBao_Project/RGB_Demo/change_color.sh","w")
            time.sleep(20)
        time.sleep(0.01) 
except KeyboardInterrupt:
    if ser_P != None:
        ser_P.close()
    if ser_T1 != None:
        ser_T1.close()
    if ser_T2 != None:
        ser_T2.close()