#include "mainwindow.h"
#include "ui_mainwindow.h"
Ui::MainWindow *globalui;
MainWindow *globalmp;
LinphoneCall *holdcall;
mutex mtx;

void MyLog(const char* content,...);
void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg);
void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message);
void call_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf);//dtmf receive
void lockLog(const char* content,...);//log -lock
void Codecable(int sel_index,int etype);//set audio codec
void vCodecable(int sel_index,int etype);//set video codec
#ifdef _WIN32
void ThreadProc(void* param);
#else
void* ThreadProc(void*);
#endif
#ifdef HAVE_X11_XLIB_H
#include <X11/Xlib.h>
#endif
void MainWindow::mylinphone_delete_proxy_list(LinphoneCore *lc)
{
    const bctbx_list_t *proxies;
    int n;
    int def=linphone_core_get_default_proxy(lc,nullptr);

    proxies=linphone_core_get_proxy_config_list(lc);
    for(n=0;proxies!=nullptr;proxies=bctbx_list_next(proxies),n++){
        if (n==def)
          lockLog(" Proxy %i - this is the default one",n);
        else
            lockLog(" Proxy %i  ",n);
        linphone_core_remove_proxy_config(lc,(LinphoneProxyConfig*)proxies->data);
    }
    if ( ! n ) lockLog("No proxies defined");
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    try {
        if(ui->label_Reg->text().contains("Ok")){
            LinphoneProxyConfig* proxy_cfg = linphone_core_get_default_proxy_config(mp.lc); /* get default proxy config*/
            linphone_proxy_config_edit(proxy_cfg); /*start editing proxy configuration*/
            linphone_proxy_config_enable_register(proxy_cfg, FALSE); /*de-activate registration for this proxy config*/
            linphone_proxy_config_done(proxy_cfg); /*initiate REGISTER with expire = 0*/
            QThread::usleep (50000);
        }
       linphone_core_terminate_all_calls(mp.lc);
        mylinphone_delete_proxy_list(mp.lc);
//        LinphoneProxyConfig* proxy_cfg = linphone_core_get_default_proxy_config(mp.lc);
//        linphone_core_remove_proxy_config(mp.lc, proxy_cfg);
        mp.running=0;
        linphone_core_unref(mp.lc);
        //linphone_core_destroy(mp.lc);
        #ifdef _WIN32
        #else
        if(mp.myfile!=nullptr)
        {
          fclose(mp.myfile);
        }
        #endif
    }catch (...){
        lockLog("closeEvent");
        }
    delete ui;
}
//set default label
void MainWindow::ChangeLabel(){
    ui->label_ext->setText(mp.myExtension);
    ui->label_domain->setText(mp.myDomain);
    globalui=ui;
    InitLin();
}
//set param
void MainWindow::SetParam(){
    GetConfig();
    globalmp=this;
}
//read config
void MainWindow::GetConfig(){
    try {
  mp.exePath=QCoreApplication::applicationDirPath();
  mp.account_path = mp.exePath+"/Account.ini";
  mp.config_name=mp.exePath+"/linphonerc";
  mp.config_fact=mp.exePath+"/linphonerc-factory";
  QDateTime time =QDateTime::currentDateTime();
  QString date =time.toString("yyyy-MM-dd");
  mp.linfile=mp.exePath+"/log/"+date+".txt";
  //read extension
  QSettings *configIniRead = new QSettings(QString(mp.account_path), QSettings::IniFormat);
   //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
   mp.myExtension = configIniRead->value("/auth_info/ext").toString();
   mp.myPassword = configIniRead->value("/auth_info/password").toString();
   mp.myDomain = configIniRead->value("/auth_info/domain").toString();
   mp.myDisplay = configIniRead->value("/auth_info/display").toString();
   QString isr = configIniRead->value("/auth_info/isrecord").toString();
   if('0'==isr)
       mp.isRecord=FALSE;
   else
       mp.isRecord=TRUE;
    configIniRead=new QSettings(QString(mp.config_name), QSettings::IniFormat);
    configIniRead->remove("/proxy_0");
    configIniRead->remove("/proxy_1");
   //读入完成后删除指针
   delete configIniRead;
   lockLog(" Mphone starting");
    } catch (...) {
         lockLog("GetConfig error");
    }
}
//write log
void MyLog(const char* content,...) {
    try
    {
        QDateTime time =QDateTime::currentDateTime();
        QString date =time.toString("yyyy-MM-dd");
        QString date_time =time.toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString path=QCoreApplication::applicationDirPath()+"/log/";
        QDir dir(path);
        if(!dir.exists())
         {
           dir.mkdir(path);
         }
         QFile file(path+date+".txt");
         file.open(QIODevice::WriteOnly | QIODevice::Append);
         QTextStream text_stream(&file);
         text_stream<<date_time<<"  " << QString(content) << "\r\n";
         file.flush();
         file.close();
    }
    catch (...)
    {

    }
}
void lockLog(const char* content,...){
    try {
        char *res;
        va_list args;
        va_start(args,content);
        res=ortp_strdup_vprintf(content,args);
        va_end(args);
        mtx.lock();
        MyLog(res);
        mtx.unlock();
    }  catch (...) {
        MyLog("writelog error");
    }
}
void LinLog(const char *domain,BctbxLogLevel lev,const char*fmt,va_list args){
    try {
        char * content =bctbx_strdup_vprintf(fmt,args);
        if(QString(content).remove(QRegExp("\\s"))==""){
            return;
        }else{
            QDateTime time =QDateTime::currentDateTime();
            QString date =time.toString("yyyy-MM-dd");
            QString date_time =time.toString("yyyy-MM-dd hh:mm:ss.zzz");
            QString path=QCoreApplication::applicationDirPath()+"/log/";
            QDir dir(path);
            if(!dir.exists())
             {
               dir.mkdir(path);
             }
            QFile file(path+date+".txt");
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            QTextStream text_stream(&file);
            text_stream<<date_time<<"  " <<domain<<":"<< QString(content) << "\r\n";
            file.flush();
            file.close();
           // lockLog(domain,content);
        }
    }  catch (...) {
        MyLog("LinLog error");
    }
}
//init linphone
void MainWindow::InitLin(){
    try {
#ifdef _WIN32
        linphone_core_set_log_level(ORTP_ERROR/*ORTP_DEBUG*/);
        linphone_core_set_log_handler(LinLog);
#else
        mp.myfile=fopen(mp.linfile.toUtf8().data(),"at+");
        if(mp.myfile==nullptr){
            fclose(mp.myfile);
            lockLog("open line file is error");
        }else{
            linphone_core_set_log_file(mp.myfile);
            linphone_core_set_log_level(/*ORTP_DEBUG*/ORTP_ERROR/*ORTP_ERROR*/);
        }
#endif
        mp.vtable.registration_state_changed = registration_state_changed;
        mp.vtable.call_state_changed = call_state_changed;
        mp.vtable.dtmf_received = call_dtmf_received;
        const char* tem= mp.config_name.toLatin1().data();
        QFileInfo fileInfo(mp.config_name);
        if(!fileInfo.isFile())
        {
            QFile file(mp.config_name);
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            QTextStream text_stream(&file);
            file.flush();
            file.close();
        }
        QFileInfo fileInfo2(mp.config_fact);
        if(!fileInfo2.isFile())
        {
            QFile file(mp.config_fact);
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            QTextStream text_stream(&file);
            file.flush();
            file.close();
        }
        mp.lc = linphone_core_new(&(mp.vtable),mp.config_name.toLatin1().data(), mp.config_fact.toLatin1().data(), nullptr);
        linphone_core_set_user_agent(mp.lc, "myphone", "4.3.0");
        LCSipTransports trans = { -1,-1,-1,-1 };
        linphone_core_set_sip_transports(mp.lc, &trans);
       #ifdef _WIN32
       _beginthread(ThreadProc, 0, &mp);
       #else
       if(0==pthread_create(&tida,NULL,ThreadProc,&mp)){
          lockLog(" Receive Thread is Started");
       }else{
       lockLog(" Receive Thread is Started Failure");
       }
       #endif
       linphone_core_enable_video_capture(mp.lc, TRUE);
       linphone_core_enable_video_display(mp.lc, FALSE);
       linphone_core_enable_video_preview(mp.lc, FALSE);


       const bctbx_list_t *node=NULL;
       PayloadType *pt;
       int index=0;
       //--------------------------VIDEO code--------------------------------------------
        node=linphone_core_get_video_codecs(mp.lc);
        lockLog(" ----------------------VIDEO code----------------------");
        for(;node!=NULL;node=bctbx_list_next(node)){
            pt=(PayloadType*)(node->data);
            lockLog(" %2d: %s (%d) %s", index, pt->mime_type, pt->clock_rate,
                linphone_core_payload_type_enabled(mp.lc,pt) ? "enabled" : "disabled");
            index++;
        }
       //--------------------------AUDIO code--------------------------------------------
        lockLog(" ----------------------AUDIO code----------------------");
         node=linphone_core_get_audio_codecs(mp.lc);
         for(;node!=NULL;node=bctbx_list_next(node)){
             pt=(PayloadType*)(node->data);
             lockLog(" %2d: %s (%d) %s", index, pt->mime_type, pt->clock_rate,
                 linphone_core_payload_type_enabled(mp.lc,pt) ? "enabled" : "disabled");
             index++;
         }
         lockLog(" sip port = %i audio rtp port = %i video rtp port = %i",linphone_core_get_sip_port(mp.lc),linphone_core_get_audio_port(mp.lc),linphone_core_get_video_port(mp.lc));
       if(linphone_core_get_use_info_for_dtmf(mp.lc))
       {
        lockLog(" enable info dtmf");
       }
       if(linphone_core_get_use_rfc2833_for_dtmf(mp.lc))
       {
        lockLog(" enable rfc2833 dtmf");
       }
        // vCodecable(1,1);
         //  Codecable(0,0);
    } catch (...) {
        lockLog("InitLin error");
    }
}
void Codecable(int sel_index,int etype){
    const bctbx_list_t *node=NULL;
    PayloadType *pt;
    int index=0;
    node=linphone_core_get_audio_codecs(globalmp->mp.lc);
    for(;node!=NULL;node=bctbx_list_next(node)){
        if(index==sel_index||sel_index==-1){
            pt=(PayloadType*)(node->data);
            if(etype==0){
            linphone_core_enable_payload_type(globalmp->mp.lc,pt,FALSE);
            }else{
                linphone_core_enable_payload_type(globalmp->mp.lc,pt,TRUE);
            }
            lockLog("%2d: %s (%d) %s", index, pt->mime_type, pt->clock_rate,
                linphone_core_payload_type_enabled(globalmp->mp.lc,pt) ? "enabled" : "disabled");
        }
        index++;
    }
}
void vCodecable(int sel_index,int etype){
    const bctbx_list_t *node=NULL;
    PayloadType *pt;
    int index=0;
    node=linphone_core_get_video_codecs(globalmp->mp.lc);
    for(;node!=NULL;node=bctbx_list_next(node)){
        if(index==sel_index||sel_index==-1){
            pt=(PayloadType*)(node->data);
            if(etype==0){
            linphone_core_enable_payload_type(globalmp->mp.lc,pt,FALSE);
            }else{
                linphone_core_enable_payload_type(globalmp->mp.lc,pt,TRUE);
            }
            lockLog("%2d: %s (%d) %s", index, pt->mime_type, pt->clock_rate,
                linphone_core_payload_type_enabled(globalmp->mp.lc,pt) ? "enabled" : "disabled");
        }
        index++;
    }
}
#ifdef _WIN32
void ThreadProc(void* param)
#else
void* ThreadProc(void* param)
#endif
{
   try
   {
        PhoneS * temlc=(PhoneS *)param;
       while (temlc->running) {
           linphone_core_iterate(temlc->lc); /* first iterate initiates registration */
           QThread::usleep (50000);
       }
   }
   catch (...)
   {
       lockLog("ThreadProc error");
   }
}
void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
    try
        {
        char *from=linphone_call_get_remote_address_as_string(call);
        const PayloadType *opt=NULL;
            switch (cstate) {
            case LinphoneCallOutgoingRinging:
                lockLog(" It is now ringing remotely %s",from);
                globalui->label_Call->setText("CallOutgoingRinging");
                break;
            case LinphoneCallOutgoingEarlyMedia:
                lockLog(" It is now CallOutgoingEarlyMedia %s",from);
                globalui->label_Call->setText("CallOutgoingEarlyMedia");
                break;
            case LinphoneCallConnected:
                lockLog(" We are connected %s",from);
                globalui->label_Call->setText("CallConnected");
                if(linphone_call_get_dir(call)==LinphoneCallOutgoing||linphone_call_get_dir(call)==LinphoneCallIncoming){
                    if(globalmp->mp.isRecord){
                       if(!linphone_call_is_recording(call)){
                           linphone_call_start_recording(call);
                       }
                    }
                }
                break;
            case LinphoneCallStreamsRunning:
                lockLog(" Media streams established %s",from);
                globalui->label_Call->setText("CallStreamsRunning");
                opt= linphone_call_params_get_used_audio_codec(linphone_call_get_current_params(call));
                lockLog(" mime_type:%s clock_rate:%d",opt->mime_type,opt->clock_rate);
                if(linphone_core_video_enabled(globalmp->mp.lc)){
                     opt=linphone_call_params_get_used_video_codec(linphone_call_get_current_params(call));
                     if(opt !=0x0){
                         try {
                         lockLog(" mime_type:%s clock_rate:%d",opt->mime_type,opt->clock_rate);
                     } catch (...) {

                     }
                    }
                }
                if(linphone_core_is_rtp_muted(lc)){
                    lockLog(" linphone_core_is_rtp_muted is true");
                }
                if(linphone_core_get_rtp_no_xmit_on_audio_mute(lc)){
                    lockLog(" linphone_core_get_rtp_no_xmit_on_audio_mute is true");
                }
                if(linphone_core_is_mic_muted(lc)){
                    lockLog(" linphone_core_is_mic_muted is true");
                }
                break;
            case LinphoneCallEnd:
                lockLog(" Call is terminated %s",from);
                if(globalmp->mp.isRecord){
                    if(linphone_call_is_recording(call)){
                      linphone_call_stop_recording(call);
                    }
                }
                globalui->label_Call->setText("CallEnd");
                int i;i=0;
                while (i<globalui->listWidget->count()) {
                    if(QString::compare(globalui->listWidget->item(i)->text(),QString(from))){
                        globalui->listWidget->takeItem(i);
                    }
                    i++;
                }
                linphone_core_enable_video_display(globalmp->mp.lc, FALSE);
                linphone_core_enable_video_preview(globalmp->mp.lc, FALSE);
                if(linphone_call_get_dir(call)==LinphoneCallIncoming){

                }
                break;
            case LinphoneCallError:
                lockLog(" Call failure %s",from);
                globalui->label_Call->setText("CallError");
                break;
            case LinphoneCallOutgoingInit:
                lockLog(" CallOutgoingInit %s",from);
                globalui->label_Call->setText("CallOutgoingInit");
                break;
            case LinphoneCallOutgoingProgress:
                lockLog(" CallOutgoingProgress %s",from);
                globalui->label_Call->setText("CallOutgoingProgress");
                break;
            case LinphoneCallIdle:
                lockLog(" Call IDLE %s",from);
                globalui->label_Call->setText("CallIdle");
                break;
            case LinphoneCallUpdating:
                lockLog(" Call Updating %s",from);
                globalui->label_Call->setText("CallUpdating");
                break;
            case LinphoneCallIncomingReceived://处理呼入电话
                lockLog(" Call Incoming %s",from);
                globalui->label_Call->setText("CallIncomingReceived");
                linphone_call_enable_camera(call,TRUE);
                QListWidgetItem* item;
                item=new QListWidgetItem(QString(from));
                globalui->listWidget->addItem(item);
                break;
            case LinphoneCallPausing:
                lockLog(" Call Pushing %s",from);
                globalui->label_Call->setText("CallPushing");
                break;
            case LinphoneCallPaused:
                lockLog(" Call PushPaused %s",from);
                globalui->label_Call->setText("CallPushPaused");
                break;
            case LinphoneCallIncomingEarlyMedia:
                lockLog(" Call Incoming Early Media %s",from);
                globalui->label_Call->setText("CallIncomingEarlyMedia");
                break;
            case LinphoneCallPausedByRemote:
                lockLog(" CallPausedByRemote %s",from);
                globalui->label_Call->setText("CallPausedByRemote");
                break;
            case LinphoneCallUpdatedByRemote:
                lockLog(" allUpdatedByRemote %s",from);
                globalui->label_Call->setText("CallUpdatedByRemote");
                const LinphoneCallParams *cp;
                cp=linphone_call_get_current_params(call);
                if(!linphone_call_camera_enabled(call)&&linphone_call_params_video_enabled(cp)){
                    lockLog("Far end requests to share video. Type camera on  if you agree");
                }
                break;
            case LinphoneCallReleased:
                lockLog(" CallReleased %s",from);
                break;
            case LinphoneCallEarlyUpdatedByRemote:
                lockLog(" CallEarlyUpdatedByRemote %s",from);
                globalui->label_Call->setText("CallEarlyUpdatedByRemote");
                break;
            case LinphoneCallResuming:
                lockLog(" CallResuming %s",from);
                globalui->label_Call->setText("CallResuming");
                break;
            case LinphoneCallRefered:
                lockLog(" CallRefered %s",from);
                globalui->label_Call->setText("CallRefered");
                break;
            case LinphoneCallEarlyUpdating:
                lockLog(" CallEarlyUpdating %s",from);
                globalui->label_Call->setText("CallEarlyUpdating");
                break;
            }
        }
        catch (...)
        {
            lockLog("call_state_changed error");
        }
}
void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message){
    try
        {
         char tem[1024];
       //  lockLog("----------------------registerstatechange----------------------");
         //lockLog(message);
            if (strcmp("LinphoneRegistrationOk", linphone_registration_state_to_string(cstate)) == 0)
            {
                globalui->label_Reg->setText("State:RegistrationOk");
                sprintf(tem, " %s  %s %s", globalmp->mp.myExtension.toLatin1().data(), " State:RegistrationOk",message);
                lockLog(tem);
            }
            if (strcmp("LinphoneRegistrationCleared", linphone_registration_state_to_string(cstate)) == 0)
            {
                globalui->label_Reg->setText("State:RegistrationCleared");
                sprintf(tem, " %s  %s %s",  globalmp->mp.myExtension.toLatin1().data(), " State:RegistrationCleared",message);
                lockLog(tem);
            }
            if (strcmp("LinphoneRegistrationProgress", linphone_registration_state_to_string(cstate)) == 0)
            {
                globalui->label_Reg->setText("State:RegistrationProgress");
                sprintf(tem, " %s  %s %s", globalmp->mp.myExtension.toLatin1().data(), " State:RegistrationProgress",message);
                lockLog(tem);
            }
            if (strcmp("LinphoneRegistrationFailed", linphone_registration_state_to_string(cstate)) == 0)
            {
                globalui->label_Reg->setText("State:RegistrationFailed");
                sprintf(tem, " %s  %s %s", globalmp->mp.myExtension.toLatin1().data(), " State:RegistrationFailed",message);
                lockLog(tem);
            }
        }
        catch (...)
        {
            lockLog("registration_state_changed error");
        }
}
void call_dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf){
    try
        {
        char *from=linphone_call_get_remote_address_as_string(call);
        lockLog("Receiving tone %c from %s",dtmf,from);
        //* 42 #35
        switch (dtmf) {
        case 42:
            globalui->label_dtmf->setText(QString("receive tone * from ")+from);
            break;
        case 35:
            globalui->label_dtmf->setText(QString("receive tone # from ")+from);
            break;
        default:globalui->label_dtmf->setText("receive tone "+QString(dtmf)+" from "+from);
            break;
        }
        ms_free(from);
        }
        catch (...)
        {
            lockLog("call_dtmf_received error");
        }
}
void MainWindow::on_pBReg_clicked()
{
    try
        {
            char* tem = new char[1024];;
            sprintf(tem, "sip:%s@%s", mp.myExtension.toUtf8().data(), mp.myDomain.toUtf8().data());
            mp.proxy_cfg=linphone_core_create_proxy_config(mp.lc);
            mp.from=linphone_address_new(tem);
            mp.info=linphone_auth_info_new(mp.myExtension.toUtf8().data(),mp.myExtension.toUtf8().data(),mp.myPassword.toUtf8().data(),nullptr,nullptr,mp.myDomain.toUtf8().data());
            linphone_proxy_config_set_identity_address(mp.proxy_cfg,mp.from);
            mp.server_addr=linphone_address_get_domain(mp.from);
            linphone_proxy_config_set_server_addr(mp.proxy_cfg,mp.server_addr.toUtf8().data());
            linphone_proxy_config_enable_register(mp.proxy_cfg,TRUE);
            linphone_proxy_config_enable_publish(mp.proxy_cfg,TRUE);
            linphone_core_add_auth_info(mp.lc,mp.info);
            linphone_address_unref(mp.from);
            linphone_core_add_proxy_config(mp.lc,mp.proxy_cfg);
            linphone_core_set_default_proxy_config(mp.lc,mp.proxy_cfg);
        }
        catch (...)
        {
            lockLog("Register error");
        }
}
void MainWindow::on_pBUnRef_clicked()
{
    try {
        LinphoneProxyConfig *cfg = linphone_core_get_default_proxy_config(mp.lc);
            if (cfg && linphone_proxy_config_get_state(cfg) == LinphoneRegistrationOk) {
                linphone_proxy_config_edit(cfg);
                linphone_proxy_config_enable_register(cfg,FALSE);
                linphone_proxy_config_done(cfg);
            }else{

            }
     }
     catch (...)
     {
         lockLog("on_pBUnRef_clicked error");
     }
}
void MainWindow::on_pBCallAudio_clicked()
{
    try {
        char* called=new char[5+ui->leCallTo->text().count()+mp.myDomain.count()];
        char* callto=new char[5+ui->leCallTo->text().count()+mp.myDomain.count()];
        sprintf(called, "sip:%s@%s", ui->leCallTo->text().toUtf8().data(), mp.myDomain.toUtf8().data());
        const char* to = called;
        LinphoneCall *call;
        LinphoneCallParams *cp=linphone_core_create_call_params (mp.lc, NULL);

        //record
        if(mp.isRecord){
            QString recpath=mp.exePath+"/record/"+QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")+"_"+mp.myExtension+"_"+ui->leCallTo->text()+"_out.mkv";
            //write record path to database --undone
            QDir dir(mp.exePath+"/record");
            if(!dir.exists())
             {
               dir.mkdir(mp.exePath+"/record");
             }
            QByteArray ba=recpath.toLatin1();
            linphone_call_params_set_record_file(cp,ba.data());
        }
        if ( linphone_core_in_call(mp.lc) )
            {
                lockLog("Terminate or hold on the current call first.");
                return;
            }
        linphone_call_params_enable_video (cp,FALSE);
        linphone_call_params_enable_early_media_sending(cp,TRUE);
        if(NULL==(call=linphone_core_invite_with_params(mp.lc,called,cp))){
            lockLog("Error from invite.",called);
        }else{

        }
        linphone_call_params_unref(cp);
    }   catch (...)
    {
        lockLog("on_pBCallAudio_clicked error");
    }
}
void MainWindow::on_pBHangUp_clicked()
{
    try
    {
        linphone_core_terminate_all_calls(mp.lc);
    }
    catch (...)
    {
        lockLog("on_pBHangUp_clicked error");
    }
}
void MainWindow::on_pBCallvido_clicked()
{
    try {
        char* called=new char[5+ui->leCallTo->text().count()+mp.myDomain.count()];
        char* callto=new char[5+ui->leCallTo->text().count()+mp.myDomain.count()];
        sprintf(called, "sip:%s@%s", ui->leCallTo->text().toUtf8().data(), mp.myDomain.toUtf8().data());
        const char* to = called;
        LinphoneCall *call;
        LinphoneCallParams *cp=linphone_core_create_call_params (mp.lc, NULL);

        //record
        if(mp.isRecord){
            QString recpath=mp.exePath+"/record/"+QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")+"_"+mp.myExtension+"_"+ui->leCallTo->text()+"out.mkv";
            //write record path to database --undone
            QDir dir(mp.exePath+"/record");
            if(!dir.exists())
             {
               dir.mkdir(mp.exePath+"/record");
             }
            QByteArray ba=recpath.toLatin1();
            linphone_call_params_set_record_file(cp,ba.data());
        }
        if ( linphone_core_in_call(mp.lc) )
            {
                lockLog("Terminate or hold on the current call first.");
                return;
            }
        linphone_call_params_enable_video (cp,TRUE);
        linphone_call_params_enable_early_media_sending(cp,TRUE);
        linphone_core_enable_video_display(mp.lc, TRUE);
        linphone_core_enable_video_preview(mp.lc, TRUE);
        if(NULL==(call=linphone_core_invite_with_params(mp.lc,called,cp))){
            lockLog("Error from invite.",called);
        }else{

        }
        linphone_call_params_unref(cp);
         if(linphone_call_camera_enabled(call)){
             lockLog(" Camera is allowed for");
         }else{lockLog(" Camera is dis-allow for current call.");}
    }
    catch (...)
    {
        lockLog("on_pBCallvido_clicked error");
    }
}
void MainWindow::on_pBHold_clicked()
{
    try {
        if(linphone_core_in_call(mp.lc))
        {
            if(-1==linphone_call_pause(linphone_core_get_current_call(mp.lc))){
                lockLog(" pause is error");
            }else{
                lockLog(" pause is success");
            }
        }
    }   catch (...)
    {
        lockLog("on_pBHold_clicked error");
    }
}
void MainWindow::on_pBResume_clicked()
{
    try {
        const bctbx_list_t *calls = linphone_core_get_calls(mp.lc);
        if(calls)
        {
         LinphoneCall *call;
         const bctbx_list_t *elem;
         elem = linphone_core_get_calls(mp.lc);
         if (elem == nullptr) {
             lockLog("(empty)\n");
         } else {
             for (; elem != nullptr; elem = elem->next) {
                 char *tmp=new char[sizeof (linphone_call_get_remote_address_as_string(call))];
                 call = (LinphoneCall *)elem->data;
                 if(linphone_call_resume(call)==-1){
                   lockLog("There was a problem to resume the call check the remote address you gave %s",tmp);
                 }
                 ms_free(tmp);
             }
         }
        }else
        {
            lockLog("No active call.\n");
        }
    }   catch (...)
    {
        lockLog("on_pBResume_clicked error");
    }
}
void MainWindow::on_pBTran_clicked()
{
    try {
         LinphoneCall * call=linphone_core_get_current_call(mp.lc);
         if(nullptr==call){
             lockLog("No active call.");
         }
         QString url="sip:"+ui->leCallTo->text()+"@"+mp.myDomain;
         QByteArray ba=url.toLatin1();
         char *tem=ba.data();
         //sprintf(tem,"sip:%s@%s",ba.data(),mp.myDomain.toUtf8().data());
         linphone_call_transfer(call,tem);
         lockLog(" transfer %s to %s",linphone_call_get_remote_address_as_string(call),tem);
    }catch (...)
    {
        lockLog("on_pBTran_clicked error");
    }
}
void MainWindow::on_pBCon_clicked()
{
    try {
    LinphoneCall *call=linphone_core_get_current_call(mp.lc);
    LinphoneCall *callpause;
    const bctbx_list_t *elem;
    elem = linphone_core_get_calls(mp.lc);
    if (elem == nullptr) {
        lockLog("(empty)\n");
    } else {
            for (; elem != nullptr; elem = elem->next) {
//                char *tmp=new char[sizeof (linphone_call_get_remote_address_as_string(call))];
                callpause = (LinphoneCall *)elem->data;
                if(linphone_call_get_state(callpause)==LinphoneCallPaused){
                     linphone_call_transfer_to_another(call,callpause);
                     lockLog(" Contwocall %s to %s",linphone_call_get_remote_address_as_string(call),linphone_call_get_remote_address_as_string(callpause));
                     break;
                }
                //ms_free(tmp);
            }
        }
//        if(call && linphone_call_get_state(call) !=LinphoneCallEnd){
//           linphone_core_terminate_call(mp.lc,call);
//        }
    }   catch (...)
    {
        lockLog("on_pBCon_clicked error");
    }
}
void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    try {
        QByteArray ba=item->text().toLatin1();
        char * tem=ba.data();
        lockLog(" tem %s",tem);
        const bctbx_list_t * elem=linphone_core_get_calls(mp.lc);
        if(elem==NULL){
            lockLog(" incoming call null");
        }else{
            for (;elem!=NULL ;elem=elem->next ) {
                LinphoneCall* callnow;
               callnow=(LinphoneCall*)elem->data;
                char* extcom=linphone_call_get_remote_address_as_string(callnow);
               // if(strcmp(extcom,tem)==0){
                if(QString::compare(extcom,tem,Qt::CaseInsensitive)==0){                    
                    LinphoneCallParams* params;
                    params=linphone_core_create_call_params(mp.lc,callnow);
                    //record
                    if(mp.isRecord){
                        QString remoteext=extcom;
                        int istart=remoteext.indexOf("sip:");     //01234567890123456789
                        int iend =remoteext.indexOf("@",istart+4);//sip:1000@192.168.0.51
                        remoteext=remoteext.mid(istart+4,iend-istart-4);
                        QString recpath=mp.exePath+"/record/"+QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")+"_"+remoteext+"_"+mp.myExtension+"_in.mkv";
                        //write record path to database --undone
                        QByteArray ba=recpath.toLatin1();
                        linphone_call_params_set_record_file(params,ba.data());
                    }
                    linphone_call_enable_camera(callnow,TRUE);
                    linphone_call_params_enable_early_media_sending(params,TRUE);
                    linphone_call_params_enable_video(params,TRUE);
                    linphone_call_accept_early_media_with_params(callnow,params);
                    linphone_call_params_unref(params);
                    linphone_core_accept_call(mp.lc,callnow);
                }
                lockLog(" remote_address %s",extcom);
            }
        }
    }  catch (...)
    {
        lockLog("on_listWidget_itemDoubleClicked error");
    }
}

/*send dtmf*/
void MainWindow::on_pBSendDtmf_clicked()
{
    try {
        QByteArray ba=ui->leCallTo->text().toLatin1();
        char *tem=ba.data();
        linphone_call_send_dtmfs(linphone_core_get_current_call(mp.lc), tem);
        char* tmp;
        lockLog("send tone %s from %s",tem,linphone_call_get_remote_address_as_string(linphone_core_get_current_call(mp.lc)));
        globalui->label_dtmf->setText(QString("send tone ")+tem+QString(" from ")+linphone_call_get_remote_address_as_string(linphone_core_get_current_call(mp.lc)));

//        while ( isdigit(*tem) || *tem == '#' || *tem == '*' )
//            {
//                if (linphone_core_get_current_call(mp.lc))
//                {
//                    //linphone_call_send_dtmf(linphone_core_get_current_call(mp.lc), *tem);
//                    linphone_core_send_dtmf(mp.lc,*tem);
//                }
//                linphone_core_play_dtmf (mp.lc,*tem,100);
//                ms_sleep(1); // be nice
//                ++tem;
//            }
    }catch (...){
        lockLog("on_pushButton_2_clicked error");
    }
}
//redirect
void MainWindow::on_pBRedirct_clicked()
{
//    try {
//        LinphoneCall * call = linphone_core_get_current_call(mp.lc);
//        QString url="sip:"+ui->leCallTo->text()+mp.myDomain;
//        QByteArray ba=url.toLatin1();
//        char *tem=ba.data();
//        //sprintf(tem,"sip:%s@%s",ba.data(),mp.myDomain.toUtf8().data());
////        const bctbx_list_t *elem;
////        if((elem=linphone_core_get_calls)==NULL){
////            lockLog(" No active calls.");
////        }

//        if ( call != NULL ) {
//            if (linphone_call_get_dir(call)==LinphoneCallIncoming) {
//                if (linphone_call_redirect(call,tem) != 0) {
//                     lockLog(" Could not redirect call %s.",tem);
//                 }else{
//                    lockLog(" redirect call %s to %s",linphone_call_get_remote_address_as_string(call),tem);
//                }
//            }  else{
//                lockLog("no incomingcall redirect call %s.",tem);
//            }
//        }
//    } catch (...){
//        lockLog("on_pushButton_3_clicked error");
//    }
}
//openvideo
void MainWindow::on_pBopenvideo_clicked()
{
  const LinphoneCallParams *cp=linphone_call_get_current_params(linphone_core_get_current_call(mp.lc));
  LinphoneCallParams *cp2=linphone_call_params_copy(cp);
  linphone_call_params_enable_video(cp2,TRUE);
  linphone_call_update(linphone_core_get_current_call(mp.lc),cp2);
  linphone_call_params_unref(cp2);
  linphone_core_enable_video_display(mp.lc, TRUE);
  linphone_core_enable_video_preview(mp.lc, TRUE);
}
