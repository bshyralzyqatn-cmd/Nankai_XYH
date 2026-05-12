#include "mainwindow.h"
#include <QRandomGenerator>
#include <QApplication>
#include <QPainter>
#include <QWidget>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), selectedSlot(nullptr)
{
    // --- 1. 窗口基础设置 ---
    this->setFixedSize(1200, 800);
    this->setWindowTitle("【南开大学26C++】免疫战争：ATP保卫战");

    state = Menu;

    // --- 2. 资源加载 ---
    QString bgPath = QCoreApplication::applicationDirPath() + "/menu_bg.png";
    menuBgImg.load(bgPath);

    // 请确保你的战斗背景图片名字叫 battle_bg.png，并且已经放在了 exe 旁边的 build 文件夹里
    QString gameBgPath = QCoreApplication::applicationDirPath() + "/battle_bg.png";
    gameBgImg.load(gameBgPath);

    audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(0.5f); // 设置音乐音量为 50%
    // ... 后面的音乐代码保持不变 ...

    bgmPlayer = new QMediaPlayer(this);
    bgmPlayer->setAudioOutput(audioOutput);

    // 【关键修改：精准对应你的文件名 "Video Project.m4a"】
    QString bgmPath = QCoreApplication::applicationDirPath() + "/Video Project.m4a";
    bgmPlayer->setSource(QUrl::fromLocalFile(bgmPath));

    // 设置无限循环播放
    bgmPlayer->setLoops(QMediaPlayer::Infinite);

    // 开始播放！
    bgmPlayer->play();
    // ==========================================
    // ▲▲▲ 背景音乐代码结束 ▲▲▲
    // ==========================================
    // --- 3. 音效系统初始化 ---
    clickSound = new QSoundEffect(this);
    clickSound->setSource(QUrl::fromLocalFile("click.wav"));
    clickSound->setVolume(0.8f);

    laserSound = new QSoundEffect(this);
    laserSound->setSource(QUrl::fromLocalFile("laser.wav"));
    laserSound->setVolume(0.4f);

    // --- 4. 按钮“隐形热区”坐标设定 (在这里调位置！) ---
    // 格式：QRect(X坐标, Y坐标, 宽度, 高度)
    // 每次改完这几个数字，重新运行看红色方块有没有对齐底图
    startBtnRect   = QRect(100, 360, 260, 95);  // 开始游戏
    guideBtnRect   = QRect(115, 475, 230, 80);   // 细胞图鉴 (指南)
    exitBtnRect    = QRect(115, 570, 230, 80);   // 退出游戏

    // 如果底图上没有选项按钮，就设为0隐藏
    optionsBtnRect = QRect(0, 0, 0, 0);

    backBtnRect    = QRect(490, 700, 220, 50);  // 返回按钮

    // --- 5. 游戏初始逻辑数据 ---
    money = 150;
    baseHp = 10;
    spawnTimer = 0;
    totalEnemies = 50; // <--- 新增：本关总怪数设为 30
    spawnedCount = 0;  // <--- 新增：已刷出 0 个
    waypoints << QPointF(310, 390) << QPointF(630, 190) << QPointF(730, 190) << QPointF(880, 300) << QPointF(880, 400)<< QPointF(950, 440) << QPointF(1000, 420);

    towerSlots << new TowerSlot(280, 473) << new TowerSlot(365, 274) << new TowerSlot(475, 196)
               << new TowerSlot(595, 129) << new TowerSlot(870, 205) << new TowerSlot(965, 353);

    // --- 6. 游戏主循环控制 ---
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
        if (state == Playing) {
            // 刷怪逻辑：只有当已刷出的怪小于总数时，才继续刷
            if (spawnedCount < totalEnemies) {
                if (++spawnTimer >= 40) {
                    spawnTimer = 0;
                    if (QRandomGenerator::global()->bounded(100) < 65)
                        enemies.append(new Bacteria(50, 390));
                    else
                        enemies.append(new Virus(50, 390));

                    spawnedCount++; // 记录：又刷出了一个怪
                }
            }

            // ... (中间的塔开火、激光特效代码保持不变) ...
            lasers.clear(); // 每一帧先清空上一帧的激光线
            for (TowerSlot* s : towerSlots) {
                if (s->builtTower) {
                    s->builtTower->fire(enemies, lasers); // 塔寻找目标并开火，将激光存入 lasers
                }
            }
            // 如果这一帧有塔开火了，播放音效
            if (lasers.size() > 0 && laserSound) {
                laserSound->play();
            }
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

            // 结局判定
            if (baseHp <= 0) {
                state = GameOver; // 血量归零，失败
            }
            // 【关键判定】：怪全刷完了，并且场上的怪都被清空了，且基地没爆
            else if (spawnedCount >= totalEnemies && enemies.isEmpty()) {
                state = GameWin;  // 恭喜通关！
            }
        }
        update();
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
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // --- 状态 1: 主菜单界面绘制 ---
    if (state == Menu) {
        if (!menuBgImg.isNull()) {
            painter.drawPixmap(0, 0, this->width(), this->height(), menuBgImg);
        } else {
            painter.fillRect(rect(), QColor(20, 25, 35));
        }


        painter.setPen(QColor(255, 255, 255, 120));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(10, 785, "v1.0 | 南开大学26C++ 工程实验班项目");
    }

    // --- 状态 2: 游戏指南与设置页面 ---
    else if (state == Guide || state == Options) {
        painter.fillRect(rect(), QColor(30, 40, 50));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 24, QFont::Bold));

        if (state == Guide) {
            painter.drawText(QRect(0, 100, 1200, 50), Qt::AlignCenter, "【 细 胞 图 鉴 】");
            painter.setFont(QFont("Microsoft YaHei", 14));
            QString guideText = "战备信息：部署免疫细胞防御塔，防止病毒穿透血管终点。\n\n"
                                "操作口诀：\n"
                                "1. [鼠标点击] 灰色基座选中位置。\n"
                                "2. [键盘按键] 决定部署单位：\n"
                                "   - 1 键：巨噬细胞(M) 近战肉搏，高额单体伤害。\n"
                                "   - 2 键：B细胞(B) 远程射击，超远火力覆盖。\n"
                                "   - 3 键：T细胞(T) 特种AOE，中程群体清扫。\n\n"
                                "使命：为人类免疫系统尊严而战！";
            painter.drawText(QRect(200, 250, 800, 400), Qt::AlignLeft | Qt::TextWordWrap, guideText);
        } else {
            painter.drawText(QRect(0, 100, 1200, 50), Qt::AlignCenter, "【 战 场 设 置 】");
            painter.setFont(QFont("Microsoft YaHei", 14));
            painter.drawText(QRect(0, 350, 1200, 50), Qt::AlignCenter, "音量调节与图形性能优化功能正在接入中...");
        }

        painter.setBrush(QColor(100, 100, 120, 200));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(backBtnRect, 8, 8);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
        painter.drawText(backBtnRect, Qt::AlignCenter, "回 到 主 舱");
    }

    // --- 状态 3: 战斗进行中界面 ---
    else if (state == Playing) {
        if (!gameBgImg.isNull()) painter.drawPixmap(0, 0, this->width(), this->height(), gameBgImg);
        else {
            painter.fillRect(rect(), QColor(255, 240, 245));
            painter.setPen(QPen(QColor(255, 180, 190), 50, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            for (int i = 0; i < waypoints.size() - 1; i++)
                painter.drawLine(waypoints[i], waypoints[i+1]);
        }

        if (selectedSlot && selectedSlot->builtTower)
            selectedSlot->builtTower->drawRange(painter);

        for (TowerSlot* s : towerSlots) s->draw(painter);
        for (Enemy* e : enemies) e->draw(painter);

        for (const Laser& l : lasers) {
            painter.setPen(QPen(l.color, 4));
            painter.drawLine(l.x1, l.y1, l.x2, l.y2);
        }

        if (selectedSlot && !selectedSlot->builtTower) {
            painter.setBrush(QColor(0,0,0,160));
            painter.drawRect(selectedSlot->x - 70, selectedSlot->y - 80, 140, 45);
            painter.setPen(Qt::white);
            painter.drawText(selectedSlot->x - 65, selectedSlot->y - 50, "1:M(50) 2:B(80) 3:T(120)");
        }

        painter.setPen(Qt::black); painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        painter.drawText(30, 40, QString("ATP 能量池: %1 | 免疫系统完整度: %2").arg(money).arg(baseHp));
    }

    // --- 状态 4: 失败结局界面 ---
    else if (state == GameOver) {
        painter.fillRect(rect(), QColor(0, 0, 0, 220));
        painter.setPen(Qt::red);
        painter.setFont(QFont("Microsoft YaHei", 50, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "免疫防线失守\n病原体已接管身体");
    }
    // --- 状态 5: 胜利结局界面 (新加的) ---
    else if (state == GameWin) {
        // 1. 加载胜利背景图 (假设文件名为 victory_bg.png)
        QPixmap victoryPix(QCoreApplication::applicationDirPath() + "/victory_bg.png");

        if (!victoryPix.isNull()) {
            // 绘制照片，铺满整个窗口
            painter.drawPixmap(0, 0, this->width(), this->height(), victoryPix);

            // 为了让字更清晰，可以叠一层淡淡的白色半透明蒙版（可选）
            painter.fillRect(rect(), QColor(255, 255, 255, 60));
        } else {
            // 如果照片加载失败，保留原有的半透明白色背景兜底
            painter.fillRect(rect(), QColor(255, 255, 255, 200));
        }

        // 2. 绘制文字
        // 设置字体
        painter.setFont(QFont("Microsoft YaHei", 50, QFont::Bold));

        // --- 修改点：透明度调低（从255调到150），颜色更透明 ---
        // 参数含义：R(50), G(200), B(50), Alpha(150)
        painter.setPen(QColor(50, 200, 50, 150));

        // --- 修改点：调整位置靠上 ---
        // 以前是 rect() (居中)，现在我们定义一个靠上的矩形框
        // QRect(左上角X, 左上角Y, 宽度, 高度)
        // 把 Y 设为 150，文字就会出现在屏幕上半部分
        QRect topRect(0, 150, this->width(), 150);

        painter.drawText(topRect, Qt::AlignCenter, "防线守卫成功！");
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
            if (std::hypot(pos.x() - s->x, pos.y() - s->y) <= 40) {
                selectedSlot = s;
                if(clickSound) clickSound->play();
                break;
            }
        }
    }
    update();
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
    update();
}