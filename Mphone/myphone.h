#pragma once

#include  <iostream>
#include <fstream>
#include <string>

//#include "stdafx.h"
#include <io.h>
#include <direct.h>
using namespace std;
#include "Resource.h"
#include <process.h>

#include "linphone/core.h"
#include <bctoolbox/vfs.h>
class MyPhone {
public:
	LinphoneCoreVTable vtable = { 0 };
	LinphoneCore *lc ;
	LinphoneProxyConfig* proxy_cfg;
	LinphoneAddress *from ;
	LinphoneAuthInfo *info;
	char* account_path = new char[1024];
	char* myExtension = new char[1024];
	char* myPassword = new char[1024];
	char* myDomain = new char[1024];
	char* myDisplay=new char[1024];
	//HANDLE m_hThread;
	bool_t running;
	FILE* myfile;//write log
	const char* mypath = new char[1024];
	char* file = new char[1024];//log path
	char* logfile = new char[1024];
	char* logfilepath = new char[1024];
	char *config_name = new char[1024];//config path
	const char* server_addr = new char[1024];
	char* error = new char[1024];//error log path
	void TheExePath();
	//void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg);
	//void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message);
	//void call_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf);
	void MyWrite(const char* content);
	void MyWriteError(const char* content);
	void isFileExist(const char* content);
	void isDirExist(const char* content);
	void UnRegister();
	void Register();
	void Init1();
	void Init2();
	void DelProxy();
public:
	MyPhone() {
		running = TRUE;
		TheExePath();
	}
	~MyPhone() {
		/*delete[]file;
		file = NULL;
		delete[]mypath;
		mypath = NULL;
		delete[]logfile;
		logfile = NULL;
		delete[]config_name;
		config_name = NULL;
		delete[]server_addr;
		server_addr = NULL;
		delete[]error;
		error = NULL;
		delete[]account_path;
		account_path = NULL;
		delete[]myExtension;
		myExtension = NULL;
		delete[]myPassword;
		myPassword = NULL;
		delete[]myDomain;
		myDomain = NULL;
		delete[]myDisplay;
		myDisplay = NULL;
		delete[] logfilepath;
		logfilepath = NULL;*/
		fclose(myfile);
	}
};

#define INT_TO_VOIDPTR(i) ((void*)(intptr_t)(i))
#define VOIDPTR_TO_INT(p) ((int)(intptr_t)(p))













