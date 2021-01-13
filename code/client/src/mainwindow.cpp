#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include <Qdebug>
#include <stdio.h>
#include <QLayout>
#include <thread>
#include <QLabel>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <QDateTime>
#include <QProgressBar>
#include <QDir>
#include <QInputDialog>
#include <QHeaderView>

#import <sys/socket.h>
#import <netinet/in.h>
#import <netinet6/in6.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#include <netdb.h>
#import <SystemConfiguration/SCNetworkReachability.h>

#define MAXBUF 8191
//8279
int m_sockfd, sockfd, listenfd, connfd;
struct sockaddr_in addr;
char sentence[8192];
int len;
int p;
PORT info;
int PASV_mode = 1;
FILE *fp;
int port = 8279;
char m_sentence[8192];
char file_contents[8192];
char ip_address[100] = {"192.168.1.106"};
//char ip_address[20] = "166.111.80.237";

//WSADATA s;
int ClientSocket;
struct sockaddr_in ClientAddr;
QDirModel *model = new QDirModel();
mySortFilter *proxyModel = new mySortFilter();
char SendBuffer[8192];

QVBoxLayout *serverlayout = new QVBoxLayout;
QVBoxLayout *clientlayout = new QVBoxLayout;
QVBoxLayout *progresslayout = new QVBoxLayout;
QProgressBar *pProgressBar;
QPushButton *sv_upload;
QPushButton *sv_download;

QLabel *server_list[10000];
int filenum_server = 0;
int filenum_client = 0;
QLabel **client_list;
QWidget *tp_;
int if_thread = 0;
FILE *fout;
int n;
long long int read_size;
long long int write_size;
FILE *fbackup = fopen("backup.txt", "r+b");
int STOR_filesize = 0;

mySortFilter::mySortFilter()
{

}

mySortFilter::~mySortFilter()
{

}
bool mySortFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(!sourceModel()) return false;

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QDirModel *model = static_cast<QDirModel*>(sourceModel());
    QString str = model->fileName(index);

    if (model->fileInfo(index).isDir()) return true;

    else if (model->fileInfo(index).isFile()) return true;

    return false;
}
void send_message(int socket,char* content){
    memset(sentence,0,sizeof(sentence));
    strcpy(sentence,content);
    len = strlen(sentence) + 1;
    sentence[len - 1] = '\r';
    sentence[len] = '\n';
    sentence[len + 1] = '\0';
    len = strlen(sentence);

    p = 0;
    while (p < len) {
        int n = send(sockfd, sentence, len,0);
        qDebug() <<"have sent" << sentence;
        if (n < 0) {
            ("Error write(): %s(%d)\n", strerror(errno), errno);
        }
        else {
            p += n;
            break;
        }
    }
    sentence[p - 2] = '\0';
    qDebug() << sentence;
    if (!strncmp(sentence, "PORT", 4)) {
        int _port[10];
        divide_port(sentence, _port, 5);
        PASV_mode = 0;
        info.port = _port[4] * 256 + _port[5];
        //info.port = 9000;
        //35,40;
        printf("port = %d\r\n", info.port);
    }
    else if (!strncmp(sentence, "STOR", 4)) {
            Upload_Thread *b = new Upload_Thread;
            b->start();
            if_thread = 1;
            qDebug() << "multithread start";
    }
    else if (!strncmp(sentence, "RETR", 4)) {
              Download_Thread *a = new Download_Thread;
              a->start();
              if_thread = 1;
              qDebug() << "multithread start";
    }
    else if (!strncmp(sentence, "LIST", 4)) {
            for(int i = 0; i < filenum_server; i++)
                delete(server_list[i]);

            filenum_server = 0;
            m_sockfd = connect_client(info.ip_address, info.port);
            while (1) {
                memset(sentence, 0, sizeof(sentence));
                int n = recv(m_sockfd, sentence, 8192,0);
                if (n == 0) {
                    memset(sentence, 0, sizeof(sentence));
                    break;
                }
                    QLabel *label = new QLabel(sentence);
                    server_list[filenum_server] = label;
                    filenum_server++;
                    serverlayout->addWidget(label);
                    qDebug()<<strlen(sentence)<<" "<<sentence;

            }
            qDebug() << "-1";
            int n = recv(sockfd, m_sentence, 8191, 0);
            m_sentence[n - 2] = '\0';
            qDebug()<< "LIST"<<m_sentence;
            close(m_sockfd);
    }
    p = 0;
    if(if_thread == 0){
        while (1) {
            int n = recv(sockfd, sentence, 8191,0);
            if (n < 0) {
                printf("Error read(): %s(%d)\n", strerror(errno), errno);
            }
            else {
                p += n;
                break;
            }
        }
        qDebug() << "END" <<sentence << endl;
    }

    sentence[p - 2] = '\0';

    if (!strncmp(sentence, "227", 3)) {
        memset(info.ip_address, 0, sizeof(info.ip_address));
        char ip[100];
        int _port[10];
        get_info(sentence, 26);
        divide_port(sentence, _port, 1);
        for (int i = 0; i < 3; i++) {
            sprintf(ip, "%d", _port[i]);
            strcat(info.ip_address, ip);
            strcat(info.ip_address, ".");
            memset(ip, 0, sizeof(ip));
        }

        sprintf(ip, "%d", _port[3]);
        strcat(info.ip_address, ip);
        PASV_mode = 1;
        info.port = _port[4] * 256 + _port[5];
        //info.port = 9000;
        qDebug() << info.port;
    }
    else if (!strncmp(sentence, "221", 3)) {
        close(sockfd);
    }
}
void reset_clientlist(){

}
void Download_Thread::progress_slots(int n){
    if (n == -1){
        //reset_clientlist();
        sv_download->setVisible(true);
        sv_upload->setVisible(true);
    }
    else{
        pProgressBar->setMinimum(0);  // 最小值
        pProgressBar->setMaximum(n);  // 最大值
    }
}
void Download_Thread::run(){
    connect(this, SIGNAL(progress_signals(int)), this, SLOT(progress_slots(int)));
    m_sockfd = connect_client(info.ip_address, info.port);
    int n = recv(sockfd, m_sentence, 8191,0);
    qDebug() <<"down" <<m_sentence;
    char temp[1000] = {""};
    m_sentence[n - 2] = '\0';
    get_info(sentence, 5);
    strcat(temp,sentence);
   //fp = fopen(temp, "wb");
    emit progress_signals(0);
    while (1) {
        n = recv(m_sockfd, file_contents, 8192,0);
        qDebug() << n;
        if (n == 0) {
            break;
        }
        fwrite(file_contents, 1, n, fp);
        read_size += n;
        rewind(fbackup);
        fprintf(fbackup, "%lld", read_size);
    }
    fclose(fp);
    fclose(fbackup);
    close(m_sockfd);
    recv(sockfd, sentence, 8191,0);

    qDebug() << "DT" <<sentence;
    emit progress_signals(-1);
    emit progress_signals(100);
    fbackup = fopen("backup.txt", "w");
    fputc('0', fbackup);
     fclose(fbackup);
    if_thread = 0;
}

void Upload_Thread::progress_slots(int n,int m){
    if (n == -1){
        qDebug() << "yes";
        sv_download->setVisible(true);
        sv_upload->setVisible(true);
    }
    else{
        pProgressBar->setMinimum(0);  // 最小值
        pProgressBar->setMaximum(m);  // 最大值
        pProgressBar->setValue(n);
    }
}


void Upload_Thread::run(){
    connect(this, SIGNAL(progress_signals(int,int)), this, SLOT(progress_slots(int,int)));
    m_sockfd = connect_client(info.ip_address, info.port);
    char temp[1000] = {""};
    int n = recv(sockfd, m_sentence, 8191,0);
    m_sentence[n - 2] = '\0';
    qDebug()<<"filename = "<<m_sentence;
    get_info(sentence, 5);
    strcat(temp,sentence);
    //fp = fopen(temp, "rb");
    //fseek(fp, 0, SEEK_END);
    int now_size = 0;
    int file_size = ftell(fp);
    //fp = fopen(temp, "rb");
    while (!feof(fp)) {
        len = fread(file_contents, 1, sizeof(file_contents), fp);
        n = send(m_sockfd, file_contents, len, 0);
        write_size = write_size + len;
        rewind(fbackup);
        fprintf(fbackup, "%lld", write_size);
        emit progress_signals(write_size,STOR_filesize);
    }
    emit progress_signals(-1,-1);
    emit progress_signals(0,100);
    close(m_sockfd);
    recv(sockfd, sentence, 8191,0);
    qDebug()<<"upload"<<sentence;
    fclose(fp);
    fclose(fbackup);

    fbackup = fopen("backup.txt", "w");
    fputc('0', fbackup);
    fclose(fbackup);
    if_thread = 0;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tp_ = ui->clientcontents;
    sv_download = ui->download;
    sv_upload = ui->upload;
    this->setCentralWidget(ui->stackedWidget);
    ui->stackedWidget->setCurrentWidget(ui->Login);
    ui->RETR->setStyleSheet("#RETR{background-color:#408080}");
    pProgressBar = new QProgressBar(this);
    pProgressBar->setOrientation(Qt::Horizontal);  // 水平方向
//    pProgressBar->setStyleSheet("QProgressBar{border: 1px solid grey;border-radius: 5px;text-align: center;}"
//                             "QProgressBar::chunk{background-color: #808040;width: 10px;margin: 0.5px;}");
    pProgressBar->setMinimum(0);  // 最小值
    pProgressBar->setMaximum(100);  // 最大值
    pProgressBar->setValue(0);  // 当前进度
    pProgressBar->setGeometry(300,30,500,50);
    pProgressBar->setVisible(false);

//    QDirModel *model = new QDirModel();
//    mySortFilter *proxyModel = new mySortFilter();
    model = new QDirModel();
    proxyModel = new mySortFilter();
    proxyModel->setSourceModel(model);
    ui->treeView->setModel(proxyModel);
    qDebug() << QCoreApplication::applicationDirPath();
    ui->treeView->setRootIndex(proxyModel->mapFromSource(model->index(QCoreApplication::applicationDirPath())));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::resizeEvent(QResizeEvent *event)//设置部件大小及坐标
{
    ui->scrollArea->setGeometry(50,100,550,400);
    ui->scrollArea_2->setGeometry(620,100,550,400);
    ui->label->setGeometry(50,520,60,20);
    ui->treeView->setGeometry(0,0,550,400);
    ui->label_2->setGeometry(620,520,60,20);
    ui->clientinfo->setGeometry(620,80,150,20);
    ui->inputfilename->setGeometry(110,520,160,20);
    ui->inputfilename_2->setGeometry(680,520,160,20);
    ui->download->setGeometry(270,520,80,20);
    ui->upload->setGeometry(840,520,80,20);
    ui->serverinfo->setGeometry(50,80,150,20);
    ui->refresh->setGeometry(200,80,80,20);
    pProgressBar->setGeometry(300,30,500,50);
}

void MainWindow::on_upload_clicked()
{
    char* filename_download;
    char temp[1000] = {"STOR "};
    char send_type[1000] = {"TYPE I"};
    char send_REST[1000];

    send_message(sockfd,send_type);
    memset(send_REST,0,sizeof(send_REST));
    fbackup =  fopen("backup.txt", "r+b");
    filename_download = ui->inputfilename_2->text().toLatin1().data();

    fp = fopen(filename_download, "rb");
    fseek(fp, 0, SEEK_END);
    STOR_filesize = ftell(fp);
    rewind(fp);

    if (fgetc(fbackup) == '0'){
        write_size = 0;
        qDebug() << "up_0";
    }
    else {
        rewind(fbackup);
        fscanf(fbackup, "%lld", &write_size);
        sprintf(send_REST, "REST %lld\r\n", write_size);
        fseek(fp,write_size,SEEK_SET);
        send_message(sockfd,send_REST);
    }
    pProgressBar->setMaximum(100);  // 最大值
    pProgressBar->setValue(0);

    filename_download = ui->inputfilename_2->text().toLatin1().data();
    strcat(temp,filename_download);
    send_message(sockfd,"PASV");
    send_message(sockfd,temp);
    ui->download->setVisible(false);
    ui->upload->setVisible(false);
}

void MainWindow::on_download_clicked()
{
    char* filename_download;
    char temp[1000] = {"RETR "};
    char send_REST[1000];
    char send_type[1000] = {"TYPE I"};

    send_message(sockfd,send_type);

    memset(sentence,0,sizeof(sentence));
    memset(send_REST,0,sizeof(send_REST));

    pProgressBar->setMaximum(100);  // 最大值
    pProgressBar->setValue(0);
    filename_download = ui->inputfilename->text().toLatin1().data();

    if (fgetc(fbackup) == '0') {
        qDebug() << "filename_download " << filename_download;
        fp = fopen(filename_download, "wb");
        read_size = 0;
     }
    else {
        qDebug() << "filename_download " << filename_download;
        fp = fopen(filename_download, "r+b");
        rewind(fbackup);
        fscanf(fbackup, "%lld", &read_size);
        sprintf(send_REST, "REST %lld", read_size);
        fseek(fp, read_size, SEEK_SET);
        qDebug() << send_REST;
        send_message(sockfd,send_REST);
     }
    strcat(temp,filename_download);
    send_message(sockfd,"PASV");
    send_message(sockfd,temp);
    ui->download->setVisible(false);
    ui->upload->setVisible(false);
}

void MainWindow::on_refresh_clicked()
{
    send_message(sockfd,"PASV");
    send_message(sockfd,"LIST");

    model = new QDirModel();
    proxyModel->setSourceModel(model);
    ui->treeView->setModel(proxyModel);
    qDebug() << QCoreApplication::applicationDirPath();
    ui->treeView->setRootIndex(proxyModel->mapFromSource(model->index(QCoreApplication::applicationDirPath())));
}



void MainWindow::on_Login_btn_clicked()
{
        char* _username;
        char* _password;
        char* ip_char;
        char* port_char;

        char ubuf[1000] = {"USER "};
        char pbuf[1000] = {"PASS "};
        ip_char = ui->ip_input->text().toLatin1().data();
        port_char = ui->port_input->text().toLatin1().data();
        memset(ip_address,0,sizeof(ip_address));
        strcpy(ip_address,ip_char);
        port = atoi(port_char);



        pProgressBar->setVisible(true);

        sockfd = connect_client(ip_address, port);
        p = recv(sockfd, sentence, MAXBUF,0);
        _username = ui->username->text().toLatin1().data();
        strcat(ubuf,_username);
        send_message(sockfd,ubuf);

        _password = ui->password->text().toLatin1().data();
        strcat(pbuf,_password);
        send_message(sockfd,pbuf);
        qDebug() << ubuf;
        qDebug() << pbuf;

        ui->stackedWidget->setCurrentWidget(ui->RETR);

        serverlayout->setContentsMargins(0,0,0,1);
        clientlayout->setContentsMargins(0,0,0,1);
        ui->servercontents->setLayout(serverlayout);
        ui->clientcontents->setLayout(clientlayout);

        send_message(sockfd,"PASV");
        send_message(sockfd,"LIST");
        qDebug() << pbuf;
       // reset_clientlist();
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    qDebug() << index.row();
    qDebug() << index.data().toByteArray();
    ui->inputfilename_2->setText(index.data().toByteArray());
}
