from config.chatgpt_config import config_dict
from src.openai_request import OpenAI_Request
from tools.cfg_wrapper import load_config
from tools.context import ContextHandler
from tools.tokennizer import Tokennizer

from flask import Flask, request, jsonify
import requests
import time

# load config
config = load_config(config_dict)
keys = config.Acess_config.authorization
model_name = config.Model_config.model_name
request_address = config.Model_config.request_address

# load context
context_manage_config = config.Context_manage_config
del_config = context_manage_config.del_config
max_context = context_manage_config.max_context
context_handler = ContextHandler(max_context=max_context, context_del_config=del_config)

# load tokenizer
tokenizer = Tokennizer(model_name)

app = Flask(__name__)


user_dictionary = {
    "UNKNOWN": "你是一名优秀的英语语言工作者，你的任务是尽可能按照人类的口吻帮助我修改润色我的段落，你只用回复润色后的段落即可",
    "SYP": "你是老师，你的学生苏宇鹏在与你聊天，他是一个聪明小孩，请回答30字以内的句子，且你只能说中文，不要换行。",
    "YJQ": "你是一名大学生，你的朋友杨嘉琪在与你聊天，她是一个聪明小孩，请以幽默的语言回答她，回答30字以内的句子，不要换行。",
    "PGQ": "你是学习微电子科学的学生，你的教授余老师在与你聊天，他是一个优秀和蔼的教授，请以尊敬的语言回答他，并请求他给你一个好的成绩，请回答30字以内的句子，不要换行。"
}

label_to_name = ['UNKNOWN', "PGQ", "YJQ", "SYP"]


def chat_test(input_s, keys, model_name, request_address, context_handler, tokenizer, log_time=True, context_max=3200):
    requestor = OpenAI_Request(keys, model_name, request_address)
    # if input_s == "clear":
    #     context_handler.clear()
    #     print('start a new session')
    #     continue
    # else:
    inputs_length = tokenizer.num_tokens_from_string(input_s)
    context_handler.append_cur_to_context(input_s, inputs_length)

    st_time = time.time()
    res = requestor.post_request(context_handler.context)
    ed_time = time.time()
    response = ""
    if res.status_code == 200:

        response = res.json()['choices'][0]['message']['content']
        # cut \n for show
        response = response.lstrip("\n")

        completion_length = res.json()['usage']['completion_tokens']
        total_length = res.json()['usage']['total_tokens']

        context_handler.append_cur_to_context(response, completion_length, tag=1)

        if total_length > context_max:
            context_handler.cut_context(total_length, tokenizer)
        print(response)

    else:
        status_code = res.status_code
        reason = res.reason
        des = res.text
        raise print(f'visit error :\n status code: {status_code}\n reason: {reason}\n err description: {des}\n '
                    f'please check whether your account  can access OpenAI API normally')

    if log_time:
        print(f'time cost : {ed_time - st_time}')

    return response


def search_dictionary(input_string, dictionary):
    for key in dictionary:
        if key in input_string:
            return dictionary[key], key
    return -1, "no match"


@app.route('/chatgpt', methods=['POST'])
def chatgpt():
    input_text = request.form['text']
    print("input:" + input_text)
    # result, k = search_dictionary(input_text, my_dictionary)
    # print(result)
    # if result != -1:
    #     requests.post("http://192.168.66.139:5000/getmessage",
    #                   data={
    #                       "text": "好的，好的。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。"})
    #     requests.post("http://192.168.66.139:5000/action", data={"text": result})
    #     return jsonify({'text': k}), 200
    # else:
    if "再见" in input_text or "拜拜" in input_text:
        requests.post("http://192.168.47.77:8999/getmessage", data={"text": "下次再见吧！"})
        return jsonify({'text': "Finished!"}), 200
    else:
        response = chat_test(input_text, keys, model_name, request_address, context_handler, tokenizer)
        print(f"\nresponse : {response}")
        requests.post("http://192.168.47.77:8999/getmessage", data={"text": response})
        return jsonify({'text': response}), 200


@app.route('/changeUser', methods=['POST'])
def changeUser():
    input_text = request.form['text']
    name = label_to_name[int(input_text)]
    print("User:" + name)

    context_handler.clear()

    USER_INFORM = user_dictionary.get(name)
    USER_LENGTH = tokenizer.num_tokens_from_string(USER_INFORM)
    context_handler.append_cur_to_context(USER_INFORM, USER_LENGTH, tag=2)

    return jsonify({'text': "Change User Successfully"}), 200


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8999)

'''
if __name__ == '__main__':
    n = 0
    for key in user_dictionary:
        print(str(n)+" "+key)
        n += 1
    input_r = input('mode select: ')
    name = label_to_name[int(input_r)]
    USER_INFORM = user_dictionary.get(name)
    USER_LENGTH = tokenizer.num_tokens_from_string(USER_INFORM)
    context_handler.append_cur_to_context(USER_INFORM, USER_LENGTH, tag=2)
    while 1:
        input_s = input('\nuser input: ')
        if input_s == "clear":
            context_handler.clear()
            print('start a new session')
            continue
        else:
            chat_test(input_s,keys,model_name,request_address,context_handler,tokenizer)
'''
