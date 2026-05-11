#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QRect>
#include <QPixmap>
#include <QSoundEffect>
#include "gameobjects.h"

enum GameState {
    Menu,       // 主菜单
    Guide,      // 游戏指南
    Options,    // 设置选项 (新加)
    Playing,    // 游戏中
    GameOver    // 游戏结束
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QTimer *timer;
    QList<QPointF> waypoints;
    QList<Enemy*> enemies;
    QList<TowerSlot*> towerSlots;
    QList<Laser> lasers;

    TowerSlot* selectedSlot;
    GameState state;

    int money;
    int baseHp;
    int spawnTimer;

    // --- 界面与多媒体 ---
    QPixmap menuBgImg;      // 菜单背景图
    QPixmap gameBgImg;      // 游戏内背景图

    QSoundEffect *clickSound;
    QSoundEffect *laserSound;

    // --- 菜单按钮区域 ---
    QRect startBtnRect;
    QRect guideBtnRect;
    QRect optionsBtnRect;   // 选项按钮
    QRect exitBtnRect;      // 退出按钮
    QRect backBtnRect;      // 返回按钮
};

#endif // MAINWINDOW_H