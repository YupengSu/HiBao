/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "speech_recognizer.h"

#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096

/* Upload User words */
static int upload_userwords()
{
	char*			userwords	=	NULL;
	size_t			len			=	0;
	size_t			read_len	=	0;
	FILE*			fp			=	NULL;
	int				ret			=	-1;

	fp = fopen("userwords.txt", "rb");
	if (NULL == fp)										
	{
		printf("\nopen [userwords.txt] failed! \n");
		goto upload_exit;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp); 
	fseek(fp, 0, SEEK_SET);  					
	
	userwords = (char*)malloc(len + 1);
	if (NULL == userwords)
	{
		printf("\nout of memory! \n");
		goto upload_exit;
	}

	read_len = fread((void*)userwords, 1, len, fp); 
	if (read_len != len)
	{
		printf("\nread [userwords.txt] failed!\n");
		goto upload_exit;
	}
	userwords[len] = '\0';
	
	MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); //ÉÏ´«ÓÃ»§´Ê±í
	if (MSP_SUCCESS != ret)
	{
		printf("\nMSPUploadData failed ! errorCode: %d \n", ret);
		goto upload_exit;
	}
	
upload_exit:
	if (NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}	
	if (NULL != userwords)
	{
		free(userwords);
		userwords = NULL;
	}
	
	return ret;
}


static void show_result(char *string, char is_over)
{
	printf("\rResult: [ %s ]", string);
	if(is_over)
		putchar('\n');
}

static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

void on_result(const char *result, char is_last)
{
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		show_result(g_result, is_last);
		if(is_last){
            if (strlen(g_result) == 0){
				send_Chat("nulltoken");
			}
			else
			    send_Chat(g_result);
        }
	}
}
static size_t a_size = 0;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{	
	size_t realsize = size * nmemb;
	char *result = (char *)userdata;
	memcpy(result, ptr, realsize);
	result[realsize] = '\0';
	//printf("\n%s\n", result);
	return realsize;
}

static CURL *curl = NULL;
void send_Chat(const char *string)
{
	CURLcode res;
	char data[1024];
	size_t size = strlen(string);
	//prtntf("*ld",size);    
	sprintf(data,"text=%s", string);    
	printf("\n%s\n",data);    
	char result[1024];    
	curl = curl_easy_init();    
	if (curl) {        
		curl_easy_setopt(curl,CURLOPT_URL,"http://192.168.87.159:8999/chatgpt");        
		curl_easy_setopt(curl,CURLOPT_POSTFIELDS,data);        
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_callback);        
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,result);     

		printf("result: %s\n",result);
		res = curl_easy_perform(curl);        
		
		if (res != CURLE_OK)            
			fprintf(stderr,"curl_easy_perform() failed:%s\n",curl_easy_strerror(res));         
		
		    
	}    
	
	printf("Result:\n%s\n",result);     
	//解析JSON数据    
	cJSON *root = cJSON_Parse(result);    
	if(!root) {        
		printf("Error before:[%s]\n", cJSON_GetErrorPtr());
		return;
	}
	
	printf("Root:%s\n",root);

	cJSON *answer = cJSON_GetObjectItemCaseSensitive(root,"text");
	if (cJSON_IsString(answer) && (answer->valuestring != NULL)) {
		printf("Answer: %s\n", answer->valuestring);
	}

    a_size = strlen(answer->valuestring);
	printf("Length of answer: %d\n", a_size);
	cJSON_Delete(root);

	curl_easy_cleanup(curl);
}


void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);
	printf("Start Listening...\n");
}
void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
		printf("\nSpeaking done \n");
	else
		printf("\nRecognizer error %d\n", reason);
}

/* demo send audio data from a file */
static void demo_file(const char* audio_file, const char* session_begin_params)
{
	int	errcode = 0;
	FILE*	f_pcm = NULL;
	char*	p_pcm = NULL;
	unsigned long	pcm_count = 0;
	unsigned long	pcm_size = 0;
	unsigned long	read_size = 0;
	struct speech_rec iat;
	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	if (NULL == audio_file)
		goto iat_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm)
	{
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}

	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm);
	fseek(f_pcm, 0, SEEK_SET);

	p_pcm = (char *)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}

	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm);
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}

	errcode = sr_init(&iat, session_begin_params, SR_USER, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed : %d\n", errcode);
		goto iat_exit;
	}

	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("\nsr_start_listening failed! error code:%d\n", errcode);
		goto iat_exit;
	}

	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; /* 200ms audio */
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		ret = sr_write_audio_data(&iat, &p_pcm[pcm_count], len);

		if (0 != ret)
		{
			printf("\nwrite audio data failed! error code:%d\n", ret);
			goto iat_exit;
		}

		pcm_count += (long)len;
		pcm_size -= (long)len;		
	}

	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("\nsr_stop_listening failed! error code:%d \n", errcode);
		goto iat_exit;
	}

iat_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{
		free(p_pcm);
		p_pcm = NULL;
	}

	sr_stop_listening(&iat);
	sr_uninit(&iat);
}

/* demo recognize the audio from microphone */
static void demo_mic(const char* session_begin_params)
{
	int errcode;
	int i = 0;

	struct speech_rec iat;

	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	errcode = sr_init(&iat, session_begin_params, SR_MIC, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed\n");
		return;
	}
	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("start listen failed %d\n", errcode);
	}
	/* demo 15 seconds recording */
	while(i++ < 15)
		sleep(1);
	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("stop listening failed %d\n", errcode);
	}

	sr_uninit(&iat);
}


/* main thread: start/stop record ; query the result of recgonization.
 * record thread: record callback(data write)
 * helper thread: ui(keystroke detection)
 */
int main(int argc, char* argv[])
{	
	curl_global_init(CURL_GLOBAL_ALL);

	int ret = MSP_SUCCESS;
	int upload_on =	1; /* whether upload the user word */
	/* login params, please do keep the appid correct */
	const char* login_params = "appid = cdd053ec, work_dir = .";
	int aud_src = 0; /* from mic or file */

	/*
	* See "iFlytek MSC Reference Manual"
	*/
	const char* session_begin_params =
		"sub = iat, domain = iat, language = zh_cn, "
		"accent = mandarin, sample_rate = 16000, "
		"result_type = plain, result_encoding = utf8";

	/* Login first. the 1st arg is username, the 2nd arg is password
	 * just set them as NULL. the 3rd arg is login paramertes 
	 * */
	ret = MSPLogin(NULL, NULL, login_params);
	if (MSP_SUCCESS != ret)	{
		printf("MSPLogin failed , Error code %d.\n",ret);
		goto exit; // login fail, exit the program
	}

	// printf("Want to upload the user words ? \n0: No.\n1: Yes\n");
	// scanf("%d", &upload_on);
	if (0)
	{
		printf("Uploading the user words ...\n");
		ret = upload_userwords();
		if (MSP_SUCCESS != ret)
			goto exit;	
		printf("Uploaded successfully\n");
	}

	// baprintf("Where the audio comes from?\n" "0: From a audio file.\n1: From microphone.\n");
	// scanf("%d", &aud_src);
	if(1) {
		printf("Demo recognizing the speech from microphone\n");
		printf("Speak in 15 seconds\n");
		while(1) {
			system("sudo pkill -9 -f blue_speak.py");
			system("sudo pkill -9 -f change_color.py");
			system("sudo pkill -9 -f green_breath.sh");
			system("sudo python3 ~/HiBao_Project/RGB_Demo/set_none.py");
			popen("~/HiBao_Project/RGB_Demo/green_breath.sh","w"); 
			demo_mic(session_begin_params);
			for (int i = 0; i < 15; i++) {
				printf("%d\n", i);
				sleep(1);
			}
			printf("test: %d\n", a_size);
            if(a_size > 0){
				int k = a_size / 9 - 10;
				if(k > 0){
					while(k){
						sleep(1);
						k--;
						printf("%d\n",k);
						}	
					}	
			}
			a_size = 0;
		}
		
		printf("TIME passed\n");
	} else {
		printf("Demo recgonizing the speech from a recorded audio file\n");
		demo_file("wav/iflytek02.wav", session_begin_params); 
	}
exit:
	curl_global_cleanup();
	MSPLogout(); // Logout...

	return 0;
}
