#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <linphone/core.h>
#include <bctoolbox/vfs.h>
//#include <ortp/port.h>
#include <QCoreApplication>
#include <QMainWindow>
#include <QDir>
#include <QThread>
#include <QListWidget>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QCloseEvent>
#include <mutex>
#include <QTextCodec>
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <string>
#ifdef _WIN32
#include <process.h>
#else
#include <pthread.h>
#endif
#define INT_TO_VOIDPTR(i) ((void*)(intptr_t)(i))
#define VOIDPTR_TO_INT(p) ((int)(intptr_t)(p))
struct PhoneS{
    QString myExtension ;
    QString myPassword ;
    QString myDomain ;
    QString myDisplay;
    bool isRecord=0;

    bool_t running=1;
    FILE* myfile;//write log
    QString linfile;
    QString account_path;
    QString exePath ;
    QString config_name;
    QString config_fact;
    QString server_addr;
    LinphoneCoreVTable vtable = { 0 };
    LinphoneCore *lc ;
    LinphoneProxyConfig* proxy_cfg;
    LinphoneAddress *from ;
    LinphoneAuthInfo *info;
    #ifdef _WIN32
    #else

    #endif
};
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void GetConfig();
    void ChangeLabel();//init show label
    void SetParam();//init phone param
    void mylinphone_delete_proxy_list(LinphoneCore *lc);
    //void Log(const char* content) ;
    void InitLin();
    PhoneS mp;
    #ifdef _WIN32
    #else
        pthread_t tida;
    #endif
    ~MainWindow();

private slots:
    void on_pBReg_clicked();

    void on_pBUnRef_clicked();

    void on_pBCallAudio_clicked();

    void on_pBHangUp_clicked();

    void on_pBCallvido_clicked();

    void on_pBTran_clicked();

    void on_pBHold_clicked();

    void on_pBResume_clicked();

    void on_pBCon_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

   // void on_pushButton_clicked();

    void on_pBSendDtmf_clicked();

    void on_pBRedirct_clicked();

   // void on_pushButton_clicked();

    void on_pBopenvideo_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
