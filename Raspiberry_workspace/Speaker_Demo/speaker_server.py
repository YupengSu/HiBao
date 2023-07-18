from flask import Flask, request, jsonify
import sys
import os
sys.path.append("..")
from Speech import Speech
import time
import smbus

speech = Speech()

speech.SetReader(speech.Reader_Type["Reader_XuXiaoBao"])
speech.SetVolume(10)
speech.Speech_text("对话服务端部署成功",speech.EncodingFormat_Type["GB2312"])

while speech.GetChipStatus() != speech.ChipStatus_Type['ChipStatus_Idle']:
    time.sleep(0.1)

app = Flask(__name__)

@app.route('/getmessage', methods=['POST'])
def chatgpt():
    input = request.form['text']
    print(input)
    
    response = {'message': 'successfully'}
    os.system("sudo pkill -9 -f green_breath.py")
    os.system("sudo pkill -9 -f red_spark.py")
    os.system("sudo pkill -9 -f change_color.py")
    os.system("sudo python3 ~/HiBao_Project/RGB_Demo/set_none.py")
    os.popen("~/HiBao_Project/RGB_Demo/blue_speak.sh")

    speech.SetReader(speech.Reader_Type["Reader_XuXiaoBao"])
    speech.SetVolume(10)
    speech.Speech_text(input,speech.EncodingFormat_Type["GB2312"])
    while speech.GetChipStatus() != speech.ChipStatus_Type['ChipStatus_Idle']:
        time.sleep(0.1) 
    if input == "下次再见吧！":
        os.system("sudo pkill -9 -f iat")
    return jsonify(response), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8999)