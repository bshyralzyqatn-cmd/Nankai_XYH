#include "mainwindow.h"
#include <QRandomGenerator>
#include <QApplication>
#include <QPainter>
#include <QWidget>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), selectedSlot(nullptr)
{
    // 1. 基础窗口设置
    this->setFixedSize(800, 600);
    this->setWindowTitle("【南开大学26C++】细胞防线：生机觉醒");

    state = Menu; // 初始状态

    // 2. 加载图片素材 (建议放在 build 文件夹根目录下)
    // 加上冒号和路径，告诉 Qt 去资源文件里拿图片
    menuBgImg.load("://images/menu_bg.png");
    gameBgImg.load("background.png");

    // 3. 初始化音效 (增加指针检查防止闪退)
    clickSound = new QSoundEffect(this);
    clickSound->setSource(QUrl::fromLocalFile("click.wav"));
    clickSound->setVolume(0.8f);

    laserSound = new QSoundEffect(this);
    laserSound->setSource(QUrl::fromLocalFile("laser.wav"));
    laserSound->setVolume(0.4f);

    // 4. 初始化菜单按钮坐标 (居中对齐)
    int btnWidth = 220;
    int btnHeight = 50;
    int centerX = (800 - btnWidth) / 2;

    startBtnRect   = QRect(centerX, 300, btnWidth, btnHeight);
    guideBtnRect   = QRect(centerX, 370, btnWidth, btnHeight);
    optionsBtnRect = QRect(centerX, 440, btnWidth, btnHeight);
    exitBtnRect    = QRect(centerX, 510, btnWidth, btnHeight);

    backBtnRect    = QRect(centerX, 500, btnWidth, btnHeight);

    // 5. 游戏初始数据
    money = 500;
    baseHp = 10;
    spawnTimer = 0;

    // 设置敌人路径
    waypoints << QPointF(0, 100) << QPointF(600, 100) << QPointF(600, 400) << QPointF(200, 400) << QPointF(200, 600);

    // 设置防御塔基座位置
    towerSlots << new TowerSlot(300, 160) << new TowerSlot(500, 160) << new TowerSlot(670, 250)
               << new TowerSlot(400, 330) << new TowerSlot(300, 480) << new TowerSlot(100, 480);

    // 6. 游戏主循环定时器 (30ms刷新一次)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
        if (state == Playing) {
            // 刷怪逻辑
            if (++spawnTimer >= 40) {
                spawnTimer = 0;
                if (QRandomGenerator::global()->bounded(100) < 65)
                    enemies.append(new Bacteria(0, 100));
                else
                    enemies.append(new Virus(0, 100));
            }

            // 塔开火逻辑
            lasers.clear();
            for (TowerSlot* s : towerSlots) {
                if (s->builtTower) s->builtTower->fire(enemies, lasers);
            }
            if (lasers.size() > 0 && laserSound) laserSound->play();

            // 敌人移动与清理
            for (int i = enemies.size() - 1; i >= 0; i--) {
                Enemy* e = enemies[i];
                if (e->hp <= 0) {
                    money += e->reward;
                    delete e;
                    enemies.removeAt(i);
                } else if (e->move(waypoints)) {
                    baseHp--;
                    delete e;
                    enemies.removeAt(i);
                }
            }

            // 失败判定
            if (baseHp <= 0) state = GameOver;
        }
        update(); // 触发重绘
    });
    timer->start(30);
}

MainWindow::~MainWindow() {
    qDeleteAll(enemies);
    qDeleteAll(towerSlots);
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 状态 1: 主菜单
    if (state == Menu) {
        if (!menuBgImg.isNull()) {
            painter.drawPixmap(0, 0, this->width(), this->height(), menuBgImg);
        } else {
            painter.fillRect(rect(), QColor(20, 25, 35));
        }

        painter.setPen(QColor(0, 0, 0, 180));
        painter.setFont(QFont("Microsoft YaHei", 40, QFont::Bold));
        painter.drawText(rect().adjusted(2, 82, 2, -300), Qt::AlignCenter, "细胞防线：生机觉醒");
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(rect().adjusted(0, 80, 0, -300), Qt::AlignCenter, "细胞防线：生机觉醒");

        painter.setPen(QColor(150, 200, 255));
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(rect().adjusted(0, 160, 0, -300), Qt::AlignCenter, "Micro Immune TD - 26C++ Edition");

        QColor btnColor(20, 40, 60, 200);
        QColor textColor(220, 230, 250);
        painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));

        painter.setBrush(btnColor); painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(startBtnRect, 8, 8);
        painter.setPen(textColor); painter.drawText(startBtnRect, Qt::AlignCenter, "开始游戏 (Start)");

        painter.setBrush(btnColor); painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(guideBtnRect, 8, 8);
        painter.setPen(textColor); painter.drawText(guideBtnRect, Qt::AlignCenter, "游戏指南 (Guide)");

        painter.setBrush(btnColor); painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(optionsBtnRect, 8, 8);
        painter.setPen(textColor); painter.drawText(optionsBtnRect, Qt::AlignCenter, "选项设定 (Options)");

        painter.setBrush(QColor(80, 20, 20, 200));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(exitBtnRect, 8, 8);
        painter.setPen(textColor); painter.drawText(exitBtnRect, Qt::AlignCenter, "退出游戏 (Exit)");

        painter.setPen(QColor(255, 255, 255, 120));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(10, 580, "v1.0 | 26C++ 工程实验班项目");
    }
    // 状态 2: 指南与选项
    else if (state == Guide || state == Options) {
        painter.fillRect(rect(), QColor(30, 40, 50));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 24, QFont::Bold));

        if (state == Guide) {
            painter.drawText(QRect(0, 50, 800, 50), Qt::AlignCenter, "【 游 戏 指 南 】");
            painter.setFont(QFont("Microsoft YaHei", 14));
            QString guideText = "背景设定：你是指挥官，负责在血管中部署免疫细胞抵抗病原体。\n\n"
                                "操作方法：\n"
                                "1. 点击灰色的虚线基座，选中防线空位。\n"
                                "2. 按下键盘数字键选择调遣：\n"
                                "   [1] 巨噬细胞(M)：50 ATP，近战单体高伤。\n"
                                "   [2] B细胞(B)：80 ATP，超远距离连击。\n"
                                "   [3] T细胞(T)：120 ATP，中距离群体AOE毒杀。\n\n"
                                "使命：保护核心血量，击溃所有病原体！";
            painter.drawText(QRect(100, 140, 600, 300), Qt::AlignLeft | Qt::TextWordWrap, guideText);
        } else {
            painter.drawText(QRect(0, 50, 800, 50), Qt::AlignCenter, "【 选 项 】");
            painter.drawText(QRect(0, 200, 800, 50), Qt::AlignCenter, "音量调节与画面设置功能开发中...");
        }

        painter.setBrush(QColor(100, 100, 120, 200));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(backBtnRect, 8, 8);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
        painter.drawText(backBtnRect, Qt::AlignCenter, "返 回");
    }
    // 状态 3: 游戏中
    else if (state == Playing) {
        if (!gameBgImg.isNull()) painter.drawPixmap(0, 0, this->width(), this->height(), gameBgImg);
        else {
            painter.fillRect(rect(), QColor(255, 240, 245));
            painter.setPen(QPen(QColor(255, 180, 190), 40, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            for (int i = 0; i < waypoints.size() - 1; i++) painter.drawLine(waypoints[i], waypoints[i+1]);
        }

        if (selectedSlot && selectedSlot->builtTower) selectedSlot->builtTower->drawRange(painter);

        for (TowerSlot* s : towerSlots) s->draw(painter);
        for (Enemy* e : enemies) e->draw(painter);
        for (const Laser& l : lasers) { painter.setPen(QPen(l.color, 3)); painter.drawLine(l.x1, l.y1, l.x2, l.y2); }

        if (selectedSlot && !selectedSlot->builtTower) {
            painter.setBrush(QColor(0,0,0,100));
            painter.drawRect(selectedSlot->x - 60, selectedSlot->y - 60, 120, 40);
            painter.setPen(Qt::white);
            painter.drawText(selectedSlot->x - 55, selectedSlot->y - 35, "1:M(50) 2:B(80) 3:T(120)");
        }

        painter.setPen(Qt::black); painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(20, 30, QString("ATP 能量: %1 | 核心生命: %2").arg(money).arg(baseHp));
    }
    // 状态 4: 游戏结束
    else if (state == GameOver) {
        painter.fillRect(rect(), QColor(0, 0, 0, 200));
        painter.setPen(Qt::red);
        painter.setFont(QFont("Microsoft YaHei", 48, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "免疫系统崩溃\nGAME OVER");
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPoint pos = event->pos();

    if (state == Menu) {
        if (startBtnRect.contains(pos)) {
            if(clickSound) clickSound->play();
            state = Playing;
        }
        else if (guideBtnRect.contains(pos)) {
            if(clickSound) clickSound->play();
            state = Guide;
        }
        else if (optionsBtnRect.contains(pos)) {
            if(clickSound) clickSound->play();
            state = Options;
        }
        else if (exitBtnRect.contains(pos)) {
            QApplication::quit();
        }
    }
    else if (state == Guide || state == Options) {
        if (backBtnRect.contains(pos)) {
            if(clickSound) clickSound->play();
            state = Menu;
        }
    }
    else if (state == Playing) {
        selectedSlot = nullptr;
        for (TowerSlot* s : towerSlots) {
            if (std::hypot(pos.x() - s->x, pos.y() - s->y) <= 30) {
                selectedSlot = s;
                if(clickSound) clickSound->play();
                break;
            }
        }
    }
    update(); // 触发界面刷新
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (state == Playing && selectedSlot && !selectedSlot->builtTower) {
        if (event->key() == Qt::Key_1 && money >= 50) {
            selectedSlot->builtTower = new MacrophageTower(selectedSlot->x, selectedSlot->y);
            money -= 50;
        }
        else if (event->key() == Qt::Key_2 && money >= 80) {
            selectedSlot->builtTower = new BCellTower(selectedSlot->x, selectedSlot->y);
            money -= 80;
        }
        else if (event->key() == Qt::Key_3 && money >= 120) {
            selectedSlot->builtTower = new TCellTower(selectedSlot->x, selectedSlot->y);
            money -= 120;
        }
    }
    update(); // 触发界面刷新
}