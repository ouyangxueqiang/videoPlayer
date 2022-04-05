#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMediaPlayer>
#include <QFileDialog>
#include <qdebug.h>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /*----------------------数据库--------------------*/
    qDebug()<<QSqlDatabase::drivers();  //打印看看（已经存在的）驱动是什么
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");  //说明使用的是SQLLITE这个数据库
    db.setDatabaseName("videoplayer.db");  //创建db文件
    db.open();
    QSqlQuery query;
    if(!query.exec("create table if not exists playerlist(id integer primary key autoincrement, name text, url text)")){
        qDebug()<<"Error 1 (create error)"<<query.lastError().text();
        return;
    }
    //db.close();

    ui->setupUi(this);

    model = new QSqlQueryModel;
    model2 = new QSqlQueryModel;
    model->setQuery("select * from playerlist");
    model2->setQuery("select name from playerlist");
    ui->tableView->setModel(model2);

    player =new QMediaPlayer;
    player->setMedia(QMediaContent(QUrl::fromLocalFile("E:\\movie\\.mp4")));
    player->setVideoOutput(ui->widget);
    player->play();

    //安装事件过滤器
    ui->widget->installEventFilter(this);

    ui->volumnSlider->setValue(100);

    //视频播放or暂停状态切换，不断获取，存在state这个成员变量里
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(onPlayerStateChanged(QMediaPlayer::State)));
    //视频长度改变，打印出来
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(onPlayerDurationChanged(qint64)));
    //进度条位置改变，打印出来
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(onPlayerPositionChanged(qint64)));
    //拖动进度条，改变视频的进度
    connect(ui->horizontalSlider,SIGNAL(sliderMoved(int)),this,SLOT(onSliderMoved(int)));
    //按下播放按钮，改变按钮的状态；因为视频的状态被player这个播放器不断获取,所以onPlayerStateChanged一直被调用，相应的图片也会改变
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(onPlay()));
    //自定义的信号costomSliderClicked()
    connect(ui->horizontalSlider,SIGNAL(costomSliderClicked()),this,SLOT(sliderClicked()));
    //监听按钮，选择播放源
    connect(ui->btnAdd,SIGNAL(clicked()),this,SLOT(onBtnAddClicked()));
    //监听播放列表的双击
    connect(ui->tableView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onItemDBCliked(QModelIndex)));
    //监听按钮，全屏播放
    connect(ui->btnScreen,SIGNAL(clicked()),this,SLOT(onBtnScreenClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * 切换视频状态（播放->暂停,暂停->播放）
*/
void MainWindow::onPlay(){
    if(player->state()== QMediaPlayer::PlayingState){
        player->pause();
    }else{
        player->play();
    }
}

/*
 * 跟踪视频状态（播放or暂停）
*/
void MainWindow::onPlayerStateChanged(QMediaPlayer::State state){
   this->state = state;
   if(state == QMediaPlayer::PlayingState){
       //ui->pushButton->setStyleSheet("border-image: url(:/pic/exo.jpg)");
   }else{
       //ui->pushButton->setStyleSheet("border-image: url(:/pic/exo.jpg)");
   }
}

/*
 * 获取视频长度
*/
void MainWindow::onPlayerDurationChanged(qint64 duration){
    //qDebug()<<duration;
    qint64 dur= duration / 1000;
    qint64 hours=dur / 3600;
    qint64 minutes=(dur - hours * 3600) / 60;
    qint64 seconds=(dur - hours * 3600 - minutes * 60);
    QString strTime = QString("%1:%2:%3").arg(hours ).arg(minutes ).arg(seconds );
    ui->horizontalSlider->setMaximum(duration);  //最大值
    ui->timelabel->setText("/"+strTime);
}

/*
 * 获取视频进度
*/
void MainWindow::onPlayerPositionChanged(qint64 position){
    //qDebug()<<position;
    qint64 dur= position / 1000;
    qint64 hours=dur / 3600;
    qint64 minutes=(dur - hours * 3600) / 60;
    qint64 seconds=(dur - hours * 3600 - minutes * 60);
    QString strTime = QString("%1:%2:%3").arg(hours ).arg(minutes ).arg(seconds );
    ui->horizontalSlider->setValue(position);  //进度条进度跟着变
    ui->currentTimeLabel->setText(strTime);
}

/*
 * 根据进度条的value，改变视频的进度
*/
void MainWindow::onSliderMoved(int value){
    player->setPosition(value);  //视频跟着进度条变
}

/*
* slider点击事件发生后，改变视频的进度
*/
void MainWindow::sliderClicked(){
    player->setPosition(ui->horizontalSlider->value());
}

/*
 * 点击右下角按钮，选择本地文件
*/
void MainWindow::onBtnAddClicked()
{
    QString curPath=QDir::homePath();    //系统当前目录
    QString dlgTitle="选择文件"; //对话框标题
    QString filter="mp4文件(*.mp4);;mp3文件(*.mp3);;所有文件(*.*)"; //文件过滤器
    QString aFile=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);

    if (aFile.isEmpty())
      return;

    QFileInfo   fileInfo(aFile);
    //ui->LabCurMedia->setText(fileInfo.fileName());
    // qDebug()<<aFile;
    player->setMedia(QUrl::fromLocalFile(aFile)); //用aFile这个绝对路径实现的播放
    player->play();
    /*--------选的哪个就存入数据库（先不考虑重复的问题）------*/
    QSqlQuery query;
    /*----获取视频文件的名字---*/
    int index=-1, lastIndex;
    do{
       lastIndex = index;
       index = aFile.indexOf('/',index+1);
    }while(index!=-1);
    QString name = aFile.right(aFile.length()-lastIndex-1);
    /*---------------------*/
    QString cmd = QString("insert into playerlist(name,url) values('%1','%2')").arg(name).arg(aFile);
    if(!query.exec(cmd)){
          qDebug()<<"Error 2 (insert error)"<<query.lastError().text();
          return;
    }
    /*-------刷新播放列表--------*/
    model->setQuery("select * from playerlist");
    model2->setQuery("select name from playerlist");
    ui->tableView->setModel(model2);
}


/*
 * 双击播放列表，播放相应视频
*/
void MainWindow::onItemDBCliked(const QModelIndex &index){
    //qDebug()<<index.row();  //打印用户点击的第几行
    QSqlRecord record = model->record(index.row());  //得到在数据库表中是第几条记录
    //测试，并且URL中的中文能够正常输出
    //qDebug()<<record.value("id").toInt()<<record.value("name").toString()<<record.value(2).toString();
    //播放相应视频，这里应该用绝对路径（否则选择播放源的功能就不能在整个文件管理器进行了）
    player->setMedia(QMediaContent(QUrl::fromLocalFile(record.value(2).toString())));
    player->play();
}

void MainWindow::onBtnScreenClicked(){
    ui->widget->setWindowFlags(Qt::Window);
    ui->widget->showFullScreen();
}

//esc退出全屏
bool MainWindow::eventFilter(QObject *target, QEvent *e)
{
    if (target==ui->widget)
    {
      if(e->type() == QEvent::KeyPress)
      {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);    //获取键盘输入事件对象
        if(keyEvent->key() == Qt::Key_Escape)
        {
            ui->widget->setWindowFlags(Qt::SubWindow);
            ui->widget->showNormal();
            ui->widget->setGeometry(20,70,711,391);
          }
        }
}
    return QMainWindow::eventFilter(target, e);
}

void MainWindow::on_volumnSlider_valueChanged(int value)
{
    player->setVolume(value);
}

void MainWindow::on_btnQuick_clicked()
{
    ui->horizontalSlider->sliderMoved(ui->horizontalSlider->value()+2000);
}

void MainWindow::on_btnSlow_clicked()
{
    ui->horizontalSlider->sliderMoved(ui->horizontalSlider->value()-2000);
}
