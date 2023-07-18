import smbus
import time


class Speech:
    def __init__(self):
        self.i2c_addr = 0x30
        self.date_head = 0xfd
        self.bus = smbus.SMBus(1)

        self.EncodingFormat_Type = {
            'GB2312': 0x00,
            'GBK': 0X01,
            'BIG5': 0x02,
            'UNICODE': 0x03
        }
        self.ChipStatus_Type = {
            'ChipStatus_InitSuccessful': 0x4A,  # 初始化成功回传
            'ChipStatus_CorrectCommand': 0x41,  # 收到正确的命令帧回传
            'ChipStatus_ErrorCommand': 0x45,  # 收到不能识别命令帧回传
            'ChipStatus_Busy': 0x4E,  # 芯片忙碌状态回传
            'ChipStatus_Idle': 0x4F  # 芯片空闲状态回传
        }
        self.Style_Type = {
            'Style_Single': 0,  # 为 0，一字一顿的风格
            'Style_Continue': 1  # 为 1，正常合成
        }  # 合成风格设置 [f?]
        self.Language_Type = {
            'Language_Auto': 0,  # 为 0，自动判断语种
            'Language_Chinese': 1,  # 为 1，阿拉伯数字、度量单位、特殊符号等合成为中文
            'Language_English': 2  # 为 1，阿拉伯数字、度量单位、特殊符号等合成为中文
        }  # 合成语种设置 [g?]
        self.Articulation_Type = {
            'Articulation_Auto': 0,  # 为 0，自动判断单词发音方式
            'Articulation_Letter': 1,  # 为 1，字母发音方式
            'Articulation_Word': 2  # 为 2，单词发音方式
        }  # 设置单词的发音方式 [h?]
        self.Spell_Type = {
            'Spell_Disable': 0,  # 为 0，不识别汉语拼音
            'Spell_Enable': 1  # 为 1，将“拼音＋1 位数字（声调）”识别为汉语拼音，例如： hao3
        }  # 设置对汉语拼音的识别 [i?]
        self.Reader_Type = {
            'Reader_XiaoYan': 3,  # 为 3，设置发音人为小燕(女声, 推荐发音人)
            'Reader_XuJiu': 51,  # 为 51，设置发音人为许久(男声, 推荐发音人)
            'Reader_XuDuo': 52,  # 为 52，设置发音人为许多(男声)
            'Reader_XiaoPing': 53,  # 为 53，设置发音人为小萍(女声
            'Reader_DonaldDuck': 54,  # 为 54，设置发音人为唐老鸭(效果器)
            'Reader_XuXiaoBao': 55  # 为 55，设置发音人为许小宝(女童声)                
        }  # 选择发音人 [m?]
        self.NumberHandle_Type = {
            'NumberHandle_Auto': 0,  # 为 0，自动判断
            'NumberHandle_Number': 1,  # 为 1，数字作号码处理
            'NumberHandle_Value': 2  # 为 2，数字作数值处理
        }  # 设置数字处理策略 [n?]
        self.ZeroPronunciation_Type = {
            'ZeroPronunciation_Zero': 0,  # 为 0，读成“zero
            'ZeroPronunciation_O': 1  # 为 1，读成“欧”音
        }  # 数字“0”在读 作英文、号码时 的读法 [o?]
        self.NamePronunciation_Type = {
            'NamePronunciation_Auto': 0,  # 为 0，自动判断姓氏读音
            'NamePronunciation_Constraint': 1  # 为 1，强制使用姓氏读音规则
        }  # 设置姓名读音 策略 [r?]
        self.OnePronunciation_Type = {
            'OnePronunciation_Yao': 0,  # 为 0，合成号码“1”时读成幺
            'OnePronunciation_Yi': 1  # 为 1，合成号码“1”时读成一
        }  # 设置号码中“1”的读法 [y?]
        self.Rhythm_Type = {
            'Rhythm_Diasble': 0,  # 为 0，“ *”和“#”读出符号
            'Rhythm_Enable': 1  # 为 1，处理成韵律，“*”用于断词，“#”用于停顿
        }  # 是否使用韵律 标记“*”和“#” [z?]

    def I2C_WriteBytes(self, str_):
        for ch in str_:
            try:
                self.bus.write_byte(self.i2c_addr, ch)
                time.sleep(0.01)
            except:
                print("write I2C error")

    def Speech_text(self, str_, encoding_format):
        str_ = str_.encode('gb2312')
        size = len(str_) + 2
        DataHead = self.date_head
        Length_HH = size >> 8
        Length_LL = size & 0x00ff
        Command = 0x01
        EncodingFormat = encoding_format

        Date_Pack = [DataHead, Length_HH, Length_LL, Command, EncodingFormat]

        self.I2C_WriteBytes(Date_Pack)

        self.I2C_WriteBytes(str_)
        print("Speech successfully")

    def SetBase(self, str_):
        str_ = str_.encode('gb2312')
        size = len(str_) + 2

        DataHead = self.date_head
        Length_HH = size >> 8
        Length_LL = size & 0x00ff
        Command = 0x01
        EncodingFormat = 0x00

        Date_Pack = [DataHead, Length_HH, Length_LL, Command, EncodingFormat]

        self.I2C_WriteBytes(Date_Pack)

        self.I2C_WriteBytes(str_)

    def TextCtrl(self, ch, num):
        if num != -1:
            str_T = '[' + ch + str(num) + ']'
            self.SetBase(str_T)
        else:
            str_T = '[' + ch + ']'
            self.SetBase(str_T)

    def GetChipStatus(self):
        AskState = [0xfd, 0x00, 0x01, 0x21]
        try:
            self.I2C_WriteBytes(AskState)
            time.sleep(0.05)
        except:
            print("I2CRead_Write error")

        try:
            Read_result = self.bus.read_byte(self.i2c_addr)
            return Read_result
        except:
            print("I2CRead error")

    def SetStyle(self, num):
        self.TextCtrl('f', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetLanguage(self, num):
        self.TextCtrl('g', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetArticulation(self, num):
        self.TextCtrl('h', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetSpell(self, num):
        self.TextCtrl('i', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetReader(self, num):
        self.TextCtrl('m', num)
        print("SetReader successful")
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetNumberHandle(self, num):
        self.TextCtrl('n', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetZeroPronunciation(self, num):
        self.TextCtrl('o', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetNamePronunciation(self, num):
        self.TextCtrl('r', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    # 设置语速 [s?] ? 为语速值，取值：0～10
    def SetSpeed(self, speed):
        self.TextCtrl('s', speed)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    # 设置语调 [t?] ? 为语调值，取值：0～10
    def SetIntonation(self, intonation):
        self.TextCtrl('t', intonation)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    # 设置音量 [v?] ? 为音量值，取值：0～10
    def SetVolume(self, volume):
        self.TextCtrl('v', volume)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetOnePronunciation(self, num):
        self.TextCtrl('y', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    def SetRhythm(self, num):
        self.TextCtrl('z', num)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)

    # 恢复默认的合成参数 [d] 所有设置（除发音人设置、语种设置外）恢复为默认值
    def SetRestoreDefault(self):
        self.TextCtrl('d', -1)
        while self.GetChipStatus() != self.ChipStatus_Type['ChipStatus_Idle']:
            time.sleep(0.002)
