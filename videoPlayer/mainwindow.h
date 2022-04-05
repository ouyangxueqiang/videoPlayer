#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QSqlQueryModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QMediaPlayer::State state;
    QSqlQueryModel *model;
    QSqlQueryModel *model2;

private slots:
   void onPlay();
   void onPlayerStateChanged(QMediaPlayer::State state);
   void onPlayerDurationChanged(qint64 duration);
   void onPlayerPositionChanged(qint64 position);
   void onSliderMoved(int value);
   void sliderClicked();
   void onBtnAddClicked();
   void onItemDBCliked(const QModelIndex &index);
   void onBtnScreenClicked();
   bool eventFilter(QObject *target, QEvent *e);

   void on_volumnSlider_valueChanged(int value);
   void on_btnQuick_clicked();
   void on_btnSlow_clicked();
};

#endif // MAINWINDOW_H

