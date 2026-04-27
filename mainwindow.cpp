#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "enemy.h"  // 最关键的一行，绝不能漏
#include <QPainter>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(400, 600);

    playerX = 180;

    gameObjects.push_back(new EnemySquare(100, -50));
    gameObjects.push_back(new EnemySquare(250, -200));
    gameObjects.push_back(new EnemySquare(50, -400));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
        for(Shape* s : gameObjects) {
            s->move();
            if(s->y > 600) {
                s->y = -30;
                s->x = QRandomGenerator::global()->bounded(0, 370);
            }
        }
        update();
    });
    timer->start(30);
}

MainWindow::~MainWindow()
{
    delete ui;
    for(Shape* s : gameObjects) {
        delete s;
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(Qt::blue);
    painter.drawEllipse(playerX, 500, 40, 40);

    for(Shape* s : gameObjects) {
        s->draw(painter);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A || event->key() == Qt::Key_Left) {
        playerX -= 25;
    } else if (event->key() == Qt::Key_D || event->key() == Qt::Key_Right) {
        playerX += 25;
    }

    if(playerX < 0) playerX = 0;
    if(playerX > 360) playerX = 360;

    update();
}