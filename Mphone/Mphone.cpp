

#include "resource.h"
#include "myphone.h"
#include <Windows.h>
#include <thread>



MyPhone mp;
HWND hwndDialog;
LPSTR state = new char[100];
void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg);
void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message);
void call_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf);
void PhoneInit();
void linphonec_set_caller(const char *caller);
void linphonec_call_identify(LinphoneCall* call);
void ThreadProc(void* param);
LinphoneCall* holdCall = NULL;
LinphoneCall* nowCall = NULL; static void *window_id = NULL;
bool_t linphonec_camera_enabled = TRUE;

static char callee_name[256] = { 0 };
static char caller_name[256] = { 0 };

INT_PTR CALLBACK Proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	try
	{
		hwndDialog = hwnd;
		switch (umsg)
		{
		case WM_COMMAND:
			if (LOWORD(wparam) == IDC_BUTTON_INIT)//init phone
			{
				try
				{
					PhoneInit();
					SetDlgItemText(hwnd, IDC_STATIC_EXT, mp.myExtension);
					SetDlgItemText(hwnd, IDC_STATIC_DOMAIN, mp.myDomain);
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonInit");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_RELOAD) {
				try
				{
					mp.myExtension = new char[1024]; mp.myPassword = new char[1024]; mp.myDomain = new char[1024]; mp.myDisplay = new char[1024];
					GetPrivateProfileString(("auth_info"), ("ext"), ("9999"), mp.myExtension, MAX_PATH, mp.account_path);
					GetPrivateProfileString(("auth_info"), ("password"), (" "), mp.myPassword, MAX_PATH, mp.account_path);
					GetPrivateProfileString(("auth_info"), ("domain"), ("192.168.0.98"), mp.myDomain, MAX_PATH, mp.account_path);
					GetPrivateProfileString(("auth_info"), ("display"), ("server"), mp.myDisplay, MAX_PATH, mp.account_path);
					state = new char[100];
					GetDlgItemTextA(hwndDialog, IDC_STATIC_REF, state, 100);
					if (strstr(state, "Ok") != NULL) {
						mp.UnRegister();
					}
					delete[] state;
					SetDlgItemText(hwnd, IDC_STATIC_EXT, mp.myExtension);
					SetDlgItemText(hwnd, IDC_STATIC_DOMAIN, mp.myDomain);
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonRELOAD");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_REG)//register phone
			{
				try
				{
					mp.Register();

				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonReg");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_UNREG)//unregister phone
			{
				try
				{
					mp.UnRegister();
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonUnreg");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_CALLVIDO)// call vido 
			{
				LinphoneCallParams *cp = linphone_core_create_call_params(mp.lc, NULL);

				linphone_call_params_enable_video(cp, TRUE);
				linphone_call_params_enable_early_media_sending(cp, TRUE);
				if (NULL == linphone_core_invite_with_params(mp.lc, "sip:6002@192.168.0.98", cp)) {

				}
				else {
					snprintf(callee_name, sizeof(callee_name), "%s", "--sip:6002@192.168.0.98 --early-media");
				}
				linphone_call_params_unref(cp);
			}
			else if (LOWORD(wparam) == IDC_BUTTON_CALL)//call to
			{
				try
				{
					LPSTR callto = new char[1024];
					LPSTR called = new char[1024];//sip:2001@192.168.0.98
					GetDlgItemTextA(hwnd, IDC_EDIT_CALLNUM, callto, 100);
					sprintf(called, "sip:%s@%s", callto, mp.myDomain);
					const char* to = called;

					LinphoneCallParams *cp = linphone_core_create_call_params(mp.lc, NULL);
					linphone_call_params_enable_video(cp, FALSE);
					linphone_call_params_enable_early_media_sending(cp, TRUE);
					nowCall = linphone_core_invite(mp.lc, to);
					linphone_call_ref(nowCall);
					if (nowCall == NULL)
					{
						callto = new char[1024];
						sprintf(callto, "***Could not place call to %s", to);
						//callto.AppendFormat("***Could not place call to %s", to);
						mp.MyWrite(callto);
					}
					else {
						callto = new char[1024];
						sprintf(callto, "***Call to %s is in progerss...", to);
						//callto.AppendFormat("***Call to %s is in progerss...", to);
						mp.MyWrite(callto);
					}
					SetDlgItemText(hwnd, IDC_STATIC_MY, callto);
					delete[]callto;
					delete[]called;
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonCall");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_HANGUP)//hangup call
			{
				try
				{
					//nowCall = linphone_core_get_current_call(mp.lc);
					if (nowCall && linphone_call_get_state(nowCall) != LinphoneCallEnd) {
						linphone_core_terminate_call(mp.lc, nowCall);
					}
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonHANGUP");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_TRAN) //transfer call
			{
				try
				{
					nowCall = linphone_core_get_current_call(mp.lc);
					linphone_core_transfer_call(mp.lc, nowCall, "3000");
					mp.MyWrite("***transfer");
					SetDlgItemText(hwnd, IDC_STATIC_MY, "transfer");
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonTransfer");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_HOLD)//hold call
			{
				try
				{
					holdCall = nowCall;// linphone_core_get_current_call(mp.lc);
					//linphone_core_pause_call(mp.lc, holdCall);
					linphone_call_pause(holdCall);
					mp.MyWrite("***hold");
					SetDlgItemText(hwnd, IDC_STATIC_MY, "hold");
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonHold");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_RESUME) //resume call
			{
				try
				{
					/*const bctbx_list_t *	elem = linphone_core_get_calls(mp.lc);
					LinphoneCall* hold;
					if (elem == NULL) {
						mp.MyWrite("****(empty)\n");
					}
					else {
						for (; elem != NULL; elem = elem->next) {

							hold = (LinphoneCall*)elem->data;
						}
					}*/
					//linphone_call_resume(hold);
					//linphone_core_resume_call(mp.lc, holdCall);
					linphone_call_resume(holdCall);
					mp.MyWrite("***resume");
					SetDlgItemText(hwnd, IDC_STATIC_MY, "resume");
					nowCall = holdCall;
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonResume");
				}
			}
			else if (LOWORD(wparam) == IDC_LIST_INCOM) //双击接听
			{
				try
				{
					switch (HIWORD(wparam))
					{
					case LBN_DBLCLK:
					{
						HWND hwndList = GetDlgItem(hwnd, IDC_LIST_INCOM);
						// Get selected index.
						int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);

						// display item's text 
						TCHAR buff[MAX_PATH];
						SendMessage(hwndList, LB_GETTEXT, lbItem, (LPARAM)buff);
						const bctbx_list_t *	elem = linphone_core_get_calls(mp.lc);
						char* extcom = new char[100];
						if (elem == NULL) {
							mp.MyWrite("****(empty)\n");
						}
						else {
							for (; elem != NULL; elem = elem->next) {
								nowCall = (LinphoneCall*)elem->data;
								extcom = linphone_call_get_remote_address_as_string(nowCall);
								if (strcmp(extcom, buff) == 0)
								{
									linphone_core_accept_call(mp.lc, nowCall);
									mp.MyWrite(strcat(extcom, "pickup"));

									SetDlgItemText(hwnd, IDC_STATIC_MY, extcom);
								}
							}
						}
						break;
					}
					}
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonResume");
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_CONTWOCALL)//connect two call
			{
				try
				{
					//nowCall = linphone_core_get_current_call(mp.lc);
					linphone_core_transfer_call_to_another(mp.lc, nowCall, holdCall);
					if (nowCall && linphone_call_get_state(nowCall) != LinphoneCallEnd) {
						linphone_core_terminate_call(mp.lc, nowCall);
					}
					mp.MyWrite("***Contwocall");
					SetDlgItemText(hwnd, IDC_STATIC_MY, "contwocall");
				}
				catch (...)
				{
					mp.MyWriteError("OnBnClickedButtonContwocall");
				}
			}
			else if (LOWORD(wparam) == IDCANCEL)//close
			{
				try
				{
					int iRet = MessageBox(hwnd, "确定要关闭程序", "关闭", MB_ICONQUESTION | MB_YESNO);
					if (iRet == IDYES)
					{
						nowCall = linphone_core_get_current_call(mp.lc);
						if (nowCall && linphone_call_get_state(nowCall) != LinphoneCallEnd) {
							mp.MyWrite("Terminating the call...\n");
							linphone_core_terminate_call(mp.lc, nowCall);
						}
						state = new char[100];
						GetDlgItemTextA(hwndDialog, IDC_STATIC_REF, state, 100);
						if (strstr(state, "Ok") == NULL) {
							mp.UnRegister();
						}
						delete[] state;
						mp.running = FALSE;
						ms_usleep(50000);
						linphone_core_destroy(mp.lc);
						CloseHandle(0);
						EndDialog(hwnd, IDCANCEL);
					}
				}
				catch (...)
				{
					mp.MyWriteError("IDCANCEL");
				}
			}
			break;
		case WM_CLOSE://close windows
			try
			{
				int iRet = MessageBox(hwnd, "确定要关闭程序", "关闭", MB_ICONQUESTION | MB_YESNO);
				if (iRet == IDYES)
				{
					nowCall = linphone_core_get_current_call(mp.lc);
					if (nowCall && linphone_call_get_state(nowCall) != LinphoneCallEnd) {
						mp.MyWrite("Terminating the call...\n");
						linphone_core_terminate_call(mp.lc, nowCall);
					}
					state = new char[100];
					GetDlgItemTextA(hwndDialog, IDC_STATIC_REF, state, 100);
					if (strstr(state, "Ok") != NULL) {
						mp.UnRegister();
					}
					delete[]state;
					mp.running = FALSE;
					ms_usleep(50000);
					linphone_core_destroy(mp.lc);
					CloseHandle(0);
					EndDialog(hwnd, IDCANCEL);
				}
			}
			catch (...)
			{
				mp.MyWriteError("WM_CLOSE");
			}
			break;
		}
		return 0;
	}
	catch (...)
	{

	}
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nCmdShow) {

	DialogBoxA(hInstance, (LPCTSTR)IDD_DIALOG_MAIN, NULL, Proc);
	return 0;
}



void ThreadProc(void* param)
{
	try
	{
		while (mp.running) {
			linphone_core_iterate(mp.lc); /* first iterate initiates registration */
			ms_usleep(50000);
		}
	}
	catch (...)
	{
		mp.MyWriteError("PhoneInit");
	}
}
void PhoneInit() {
	try
	{
		mp.Init1();
		mp.vtable.registration_state_changed = registration_state_changed;
		mp.vtable.call_state_changed = call_state_changed;
		mp.vtable.dtmf_received = call_dtmf_received;
		mp.Init2();
		LCSipTransports trans = { -1,-1,-1,-1 };
		linphone_core_set_sip_transports(mp.lc, &trans);
		_beginthread(ThreadProc, 0, NULL);
	}
	catch (...)
	{
		mp.MyWriteError("PhoneInit");
	}
}
void linphonec_call_identify(LinphoneCall* call) {
	static int callid = 1;
	linphone_call_set_user_data(call, INT_TO_VOIDPTR(callid));
	callid++;
}
void linphonec_set_caller(const char *caller) {
	snprintf(caller_name, sizeof(caller_name) - 1, "%s", caller);
}
void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg) {
	try
	{
		//char* from = new char[50];
		char *from = linphone_call_get_remote_address_as_string(call);
		int id = VOIDPTR_TO_INT(linphone_call_get_user_data(call));
		switch (cstate) {
		case LinphoneCallOutgoingRinging:
			mp.MyWrite("***It is now ringing remotely !");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallOutgoingRinging");
			break;
		case LinphoneCallOutgoingEarlyMedia:
			mp.MyWrite("***Receiving some early media");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallOutgoingEarlyMedia");
			break;
		case LinphoneCallConnected:
			mp.MyWrite("***We are connected !");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallConnected");
			break;
		case LinphoneCallStreamsRunning:
			mp.MyWrite("***Media streams established !");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallStreamsRunning");
			break;
		case LinphoneCallEnd:
			mp.MyWrite("***Call is terminated.");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallEnd");
			break;
		case LinphoneCallError:
			mp.MyWrite("***Call failure !");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallError");
			break;
		case LinphoneCallOutgoingInit:mp.MyWrite("***CallOutgoingInit");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallOutgoingInit"); break;
		case LinphoneCallOutgoingProgress:mp.MyWrite("***CallOutgoingProgress");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallOutgoingProgress"); break;
		case LinphoneCallIdle:mp.MyWrite("***Call IDLE !");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallIdle"); break;
		case LinphoneCallUpdating:mp.MyWrite("***Call Updating");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallUpdating"); break;
		case LinphoneCallIncomingReceived://处理呼入电话
		{
			mp.MyWrite("***Call Incoming");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallIncomingReceived");
			from = linphone_call_get_remote_address_as_string(call);
			linphonec_call_identify(call);
			linphone_call_enable_camera(call, linphonec_camera_enabled);
			id = VOIDPTR_TO_INT(linphone_call_get_user_data(call));
			linphonec_set_caller(from);
			//real_early_media_sending
			LinphoneCallParams* callparams = linphone_core_create_call_params(lc, call);
			//linphonec_out("Sending early media using real hardware\n");
			linphone_call_params_enable_early_media_sending(callparams, TRUE);
			linphone_call_params_enable_video(callparams, TRUE);
			linphone_call_accept_early_media_with_params(call, callparams);
			linphone_call_params_unref(callparams);
			SendDlgItemMessage(hwndDialog, IDC_LIST_INCOM, LB_ADDSTRING, 0, (LPARAM)from); }
			break;
			/*case LinphoneCallPushIncomingReceived:
				mp.MyWrite("***Call Push Incoming");
				SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallPushIncomingReceived"); break;*/
		case LinphoneCallIncomingEarlyMedia:mp.MyWrite("***Call Incoming Early Media");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallIncomingEarlyMedia"); break;
		case LinphoneCallPausedByRemote:mp.MyWrite("***CallPausedByRemote");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallPausedByRemote"); break;
		case LinphoneCallUpdatedByRemote:mp.MyWrite("***allUpdatedByRemote");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallUpdatedByRemote"); break;
		case LinphoneCallReleased:mp.MyWrite("***CallReleased");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallReleased"); break;
		case LinphoneCallPausing:
			mp.MyWrite("***CallPausing");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallPausing");
			break;
		case LinphoneCallPaused:
			mp.MyWrite("***CallPaused");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallPaused"); break;
		case LinphoneCallResuming:
			mp.MyWrite("***CallResuming");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallResuming"); break;
		case LinphoneCallRefered:
			mp.MyWrite("***CallRefered");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallRefered"); break;
		case LinphoneCallEarlyUpdatedByRemote:mp.MyWrite("***CallEarlyUpdatedByRemote");
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, "CallEarlyUpdatedByRemote"); break;
		default:
			char * tem = new char[1024];
			sprintf(tem, "***Unhandled notification %d", cstate);
			mp.MyWrite(tem);
			SetDlgItemTextA(hwndDialog, IDC_STATIC_CALL, tem);
			delete[]tem;
			break;
		}
	}
	catch (...)
	{
		mp.MyWriteError("call_state_changed");
	}
}
void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message) {
	try
	{
		char tem[1024];
		/*sprintf(tem, "***New registration state %s for user id %s at proxy %s", linphone_registration_state_to_string(cstate), linphone_proxy_config_get_identity(cfg), linphone_proxy_config_get_addr(cfg));
		mp.MyWrite(tem);*/
		if (strcmp("LinphoneRegistrationOk", linphone_registration_state_to_string(cstate)) == 0)
		{
			SetDlgItemTextA(hwndDialog, IDC_STATIC_REF, "State:RegistrationOk");
			sprintf(tem, "***%s  %s", mp.myExtension, " State:RegistrationOk");
			mp.MyWrite(tem);
		}
		if (strcmp("LinphoneRegistrationCleared", linphone_registration_state_to_string(cstate)) == 0)
		{
			SetDlgItemTextA(hwndDialog, IDC_STATIC_REF, "State:RegistrationCleared");
			sprintf(tem, "***%s  %s", mp.myExtension, " State:RegistrationCleared");
			mp.MyWrite(tem);
		}
		if (strcmp("LinphoneRegistrationProgress", linphone_registration_state_to_string(cstate)) == 0)
		{
			SetDlgItemTextA(hwndDialog, IDC_STATIC_REF, "State:RegistrationProgress");
			sprintf(tem, "***%s  %s", mp.myExtension, " State:RegistrationProgress");
			mp.MyWrite(tem);
		}
		//delete tem;
	}
	catch (...)
	{
		mp.MyWriteError("registration_state_changed");
	}
}
void call_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf) {
	try
	{
		char *from = linphone_call_get_remote_address_as_string(call);
		fprintf(stdout, "Receiving tone %c from %s", dtmf, from);
		char * tem = { 0 };
		sprintf(tem, "***Receiving tone %c from %s", dtmf, from);
		SetDlgItemTextA(hwndDialog, IDC_STATIC_DTMF, tem);
		mp.MyWrite(tem);
		//delete tem;
		fflush(stdout);
		ms_free(from);
	}
	catch (...)
	{
		mp.MyWriteError("call_dtmf_received");
	}
}

void MyPhone::TheExePath() {
	try
	{

		char szFilePath[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
		(strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串  
		mypath = szFilePath;
		file = new char[strlen(mypath) + 20];
		config_name = new char[strlen(mypath) + 20];
		error = new char[strlen(mypath) + 20];
		logfile = new char[strlen(mypath) + 20];
		sprintf(config_name, "%s\\linphone.cfg", mypath);
		time_t timep;
		time(&timep);
		char tmp[64];
		strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
		sprintf(file, "%s\\log\\%slog.txt", mypath, tmp);
		sprintf(logfile, "%s\\log", mypath);
		sprintf(logfilepath, "%s\\log\\%slog.txt", mypath, tmp);
		sprintf(error, "%s\\log\\%serror.txt", mypath, tmp);

		sprintf(account_path, "%s\\Account.ini", mypath);
		//account_path.Format("%s", mypath);
		/*account_path = _T(mypath);*/
		//account_path.Append(_T("\\Account.ini"));
		if (!file)
		{
			MyWriteError("Log Path Is Error");
		}
		if (!config_name) {
			MyWriteError("Config File Path Is Error");
		}
		if (!account_path)
		{
			MyWriteError("Account File Path Is Error");
		}
		isDirExist(logfile);
		isFileExist(logfilepath);
		isFileExist(file);
		//isFileExist(error);
		GetPrivateProfileString(("auth_info"), ("ext"), ("9999"), myExtension, MAX_PATH, account_path);
		GetPrivateProfileString(("auth_info"), ("password"), (" "), myPassword, MAX_PATH, account_path);
		GetPrivateProfileString(("auth_info"), ("domain"), ("192.168.0.98"), myDomain, MAX_PATH, account_path);
		GetPrivateProfileString(("auth_info"), ("display"), ("server"), myDisplay, MAX_PATH, account_path);

		//WritePrivateProfileString("proxy_0", NULL, NULL, account_path);
		DelProxy();
		/*fstream fout(config_name, ios::out | ios::trunc);
		fout << "" << endl;
		fout.close();*/
	}
	catch (...)
	{
		MyWriteError("TheExePath");
	}
}
void MyPhone::DelProxy() {
	try
	{
		char ac_Result[100];
		int iret = GetPrivateProfileString("proxy_0", NULL, "", ac_Result, 90, account_path);
		if (iret > 0)
		{
			WritePrivateProfileString("proxy_0", "reg_proxy", NULL, account_path);
			WritePrivateProfileString("proxy_0", "reg_identity", NULL, account_path);
			WritePrivateProfileString("proxy_0", "quality_reporting_enabled", NULL, account_path);
			WritePrivateProfileString("proxy_0", "quality_reporting_interval", NULL, account_path);
			WritePrivateProfileString("proxy_0", "reg_expires", NULL, account_path);
			WritePrivateProfileString("proxy_0", "reg_sendregister", NULL, account_path);
			WritePrivateProfileString("proxy_0", "publish", NULL, account_path);
			WritePrivateProfileString("proxy_0", "avpf", NULL, account_path);
			WritePrivateProfileString("proxy_0", "avpf_rr_interval", NULL, account_path);
			WritePrivateProfileString("proxy_0", "dial_escape_plus", NULL, account_path);
			WritePrivateProfileString("proxy_0", "privacy", NULL, account_path);
			WritePrivateProfileString("proxy_0", "push_notification_allowed", NULL, account_path);
			WritePrivateProfileString("proxy_0", "idkey", NULL, account_path);
			WritePrivateProfileString("proxy_0", "publish_expires", NULL, account_path);
			WritePrivateProfileString("proxy_0", NULL, NULL, account_path);
		}
	}
	catch (...)
	{
		MyWriteError("DelProxy");
	}
}
void MyPhone::MyWrite(const char* content) {
	try
	{
		time_t timep;
		time(&timep);
		char tmp[64];
		strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
		char temh[64];
		strftime(temh, sizeof(temh), "%Y-%m-%d %H:%M:%S", localtime(&timep));
		//file = new char[strlen(logfile) + 20];
		//sprintf(file, "%s\\%slog.txt", logfile, tmp);
		isDirExist(logfile);
		isFileExist(logfilepath);
		isFileExist(file);
		isFileExist(error);
		ofstream ofs;
		ofs.open(logfilepath, ios::app);
		ofs << temh << content << endl;
		ofs.close();
	}
	catch (...)
	{
		MyWriteError("MyWrite");
	}
}
void MyPhone::MyWriteError(const char* content) {
	try
	{
		time_t timep;
		time(&timep);
		char tmp[64];
		strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
		char temh[64];
		strftime(temh, sizeof(temh), "%Y-%m-%d %H:%M:%S", localtime(&timep));
		/*file = new char[strlen(mypath) + 20];
		sprintf(file, "%s\\log\\%serrorlog.txt", mypath, tmp);*/
		isDirExist(logfile);
		isFileExist(error);
		ofstream ofs;
		ofs.open(error, ios::app);
		ofs << temh << content << endl;
		ofs.close();
	}
	catch (...)
	{
	}
}
void MyPhone::isFileExist(const char* content) {
	try
	{
		const char* fname = content;
		fstream fs;
		fs.open(fname, ios::in);
		if (!fs)
		{
			ofstream fout(fname);
			if (fout)
			{
				fout.close();
			}
		}
		else
		{
			return;
		}
	}
	catch (...)
	{
		MyWriteError("isFileExist");
	}
}
void MyPhone::isDirExist(const char* content) {
	try
	{
		const char* dir = content;
		if (_access(dir, 0) == -1)
		{
			_mkdir(dir);
		}
		else
		{
			return;
		}
	}
	catch (...)
	{
		MyWriteError("isDirExist");
	}
}
void MyPhone::UnRegister() {
	try
	{
		LinphoneProxyConfig* proxy_cfg = linphone_core_get_default_proxy_config(lc); /* get default proxy config*/
		linphone_proxy_config_edit(proxy_cfg); /*start editing proxy configuration*/
		linphone_proxy_config_enable_register(proxy_cfg, FALSE); /*de-activate registration for this proxy config*/
		linphone_proxy_config_done(proxy_cfg); /*initiate REGISTER with expire = 0*/
		ms_usleep(50000);
		linphone_core_clear_proxy_config(lc);
		//linphone_core_remove_proxy_config(lc, proxy_cfg);

	}
	catch (...)
	{
		MyWriteError("UnRegister");
	}
}
void func(const char *domain, BctbxLogLevel lev, const char *fmt, va_list args) {

	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&timep));
	char temh[64];
	strftime(temh, sizeof(temh), "%Y-%m-%d %H:%M:%S", localtime(&timep));
	char *message = bctbx_strdup_vprintf(fmt, args);
	//file = new char[strlen(logfile) + 20];
	//sprintf(file, "%s\\%slog.txt", logfile, tmp);

	ofstream ofs;
	ofs.open(mp.logfilepath, ios::app);
	ofs << temh << domain << "   " << "   " << message << endl;
	ofs.close();
	bctbx_free(message); return;
}
void MyPhone::Init1() {
	try
	{
		//fclose(myfile);
		myfile = _fsopen(file, "at+", _SH_DENYNO /*_SH_DENYRW*/);
		if (myfile == nullptr)
		{
			MyWriteError("open file is error");
			fclose(myfile);
		}
		else
		{
			/*linphone_core_enable_logs(myfile);*/
			//linphone_core_set_log_file(myfile);
			linphone_core_set_log_level(ORTP_ERROR);
			linphone_core_set_log_handler(func);
		}
	}
	catch (...)
	{
		MyWriteError("Init1");
	}
}
void MyPhone::Init2() {
	try
	{
		lc = linphone_core_new(&vtable, NULL, NULL /*config_name*/, NULL);
		linphone_core_set_user_agent(lc, "myphone", "4.5.0");
		/*linphone_core_set_zrtp_secrets_file(lc, config_name);
		linphone_core_set_user_certificates_path(lc, config_name);
		linphone_core_enable_video_capture(lc, TRUE);
		linphone_core_enable_video_display(lc, TRUE);
		linphone_core_enable_video_preview(lc, TRUE);*/
	}
	catch (...)
	{
		MyWriteError("Init2");
	}
}
void MyPhone::Register() {
	try
	{
		proxy_cfg = linphone_core_create_proxy_config(lc);
		/*parse identity*/
		char* tem = new char[1024];;
		sprintf(tem, "sip:%s@%s", mp.myExtension, mp.myDomain);
		from = linphone_address_new(tem);
		info = linphone_auth_info_new(myExtension, myExtension /*mydemo.myDisplay*/, myPassword, NULL, NULL, myDomain);//添加auth
		linphone_core_add_auth_info(lc, info);
		linphone_auth_info_unref(info);
		linphone_proxy_config_set_identity_address(proxy_cfg, from); /*set identity with user name and domain*/
		linphone_proxy_config_set_server_addr(proxy_cfg, tem); /* we assume domain = proxy server address*/
		server_addr = linphone_address_get_domain(from); /*extract domain address from identity*/
		linphone_proxy_config_enable_register(proxy_cfg, TRUE); /*activate registration for this proxy config*/
		linphone_proxy_config_enable_publish(proxy_cfg, FALSE);
		linphone_address_unref(from); /*release resource*/
		linphone_core_add_proxy_config(lc, proxy_cfg); /*add proxy config to linphone core*/
		linphone_core_set_default_proxy_config(lc, proxy_cfg); /*set to default proxy*/
		delete[]tem;
	}
	catch (...)
	{
		MyWriteError("Register");
	}
}