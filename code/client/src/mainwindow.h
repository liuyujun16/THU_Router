#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QObject>
#include <QProgressBar>
#include <QTreeView>
#include <QDirModel>
#include <QtGui>
#include <QDialog>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void resizeEvent(QResizeEvent *event);
    void on_upload_clicked();

    void on_download_clicked();

    void on_refresh_clicked();

    void on_Login_btn_clicked();
    
    void on_treeView_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
};

class mySortFilter : public QSortFilterProxyModel
{
public:
    mySortFilter();
    ~mySortFilter();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

};

class Download_Thread:public QThread
   {
       Q_OBJECT
signals:
    void progress_signals(int n);
public slots:
    void progress_slots(int n);
public:
   void run();
private:
   //volatile bool stopped;
};
class Upload_Thread:public QThread
   {
       Q_OBJECT
signals:
    void progress_signals(int n,int m);
public slots:
    void progress_slots(int n,int m);
public:
   void run();
private:
       //volatile bool stopped;
};


#endif // MAINWINDOW_H
