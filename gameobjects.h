#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include <QPainter>
#include <QPointF>
#include <QList>
#include <cmath>
#include <QPixmap>
class GameObject {
public:
    double x, y;
    virtual ~GameObject() {}
    virtual void draw(QPainter &painter) = 0;
};

// --- 敌人系统 ---
class Enemy : public GameObject {
public:
    double hp, maxHp, speed;
    int waypointIndex;
    int reward;
    Enemy(double startX, double startY, double h, double s, int r) {
        x = startX; y = startY; hp = maxHp = h; speed = s; reward = r; waypointIndex = 0;
    }
    bool move(const QList<QPointF>& path) {
        if (waypointIndex >= path.size()) return true;
        QPointF target = path[waypointIndex];
        double dx = target.x() - x, dy = target.y() - y;
        double dist = std::hypot(dx, dy);
        if (dist <= speed) { x = target.x(); y = target.y(); waypointIndex++; }
        else { x += (dx / dist) * speed; y += (dy / dist) * speed; }
        return false;
    }
    void draw(QPainter &painter) override {
        painter.setBrush(Qt::red); painter.drawRect(x - 20, y - 25, 40, 4);
        painter.setBrush(Qt::green); painter.drawRect(x - 20, y - 25, 40 * (hp / maxHp), 4);
        drawShape(painter);
    }
    virtual void drawShape(QPainter &painter) = 0;

};

class Bacteria : public Enemy {
public:
    Bacteria(double x, double y) : Enemy(x, y, 100, 2.5, 25) {}

    void drawShape(QPainter &painter) override {
        // 加载细菌的专属图片
        QPixmap pix("bacteria.png");

        if (!pix.isNull()) {
            // 这里设置大小为 45，如果你想让细菌比病毒更大或更小，可以修改这里
            int size =50;

            // 偏移量永远是 size 的一半，确保坐标是图片的中心点
            painter.drawPixmap(x - size/2, y - size/2, size, size, pix);
        } else {
            // 如果没找到图，继续画绿色的圆作为兜底
            painter.setBrush(Qt::darkGreen);
            painter.drawEllipse(x - 10, y - 10, 20, 20);
        }
    }
};

class Virus : public Enemy {
public:
    Virus(double x, double y) : Enemy(x, y, 60, 4.5, 40) {}

    void drawShape(QPainter &painter) override {
        // 每次绘制时实时加载（虽然效率低一点，但绝对好理解，适合改大作业）
        QPixmap pix("virus.png");

        if (!pix.isNull()) {
            // 参数含义：(x起始位置, y起始位置, 宽度, 高度, 图片源)
            painter.drawPixmap(x - 30, y - 30, 60, 60, pix);
        } else {
            // 如果找不到图片，依然显示紫色方块
            painter.setBrush(Qt::darkMagenta);
            painter.drawRect(x - 10, y - 10, 20, 20);
        }
    }
};

struct Laser { double x1, y1, x2, y2; QColor color; };

// --- 防御塔系统 ---
class Tower : public GameObject {
public:
    double range, damage;
    int cooldown, maxCooldown;
    Tower(double tx, double ty, double r, double d, int cd) {
        x = tx; y = ty; range = r; damage = d; maxCooldown = cd; cooldown = 0;
    }
    void drawRange(QPainter &painter) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 160, 255, 40));
        painter.drawEllipse(x - range, y - range, range * 2, range * 2);
    }
    virtual void fire(QList<Enemy*>& enemies, QList<Laser>& lasers) = 0;
};

class MacrophageTower : public Tower {
public:
    MacrophageTower(double x, double y) : Tower(x, y, 100, 40, 25) {}
    void draw(QPainter &painter) override {
        // 使用半透明黄色 (R:255, G:255, B:0, Alpha:100)
        painter.setBrush(QColor(255, 230, 0, 160));
        painter.setPen(Qt::NoPen); // 如果不想要外圈黑线就加这行
        painter.drawEllipse(x-15, y-15, 30, 30);

        // 写字母
        painter.setPen(Qt::black);
        painter.drawText(x-5, y+5, "M");
    }
    void fire(QList<Enemy*>& enemies, QList<Laser>& lasers) override {
        if (cooldown > 0) { cooldown--; return; }
        for (Enemy* e : enemies) {
            if (std::hypot(e->x-x, e->y-y) <= range) {
                e->hp -= damage; cooldown = maxCooldown;
                lasers.append({x, y, e->x, e->y, Qt::yellow}); break;
            }
        }
    }
};

class BCellTower : public Tower {
public:
    BCellTower(double x, double y) : Tower(x, y, 220, 12, 10) {}
    void draw(QPainter &painter) override {
        // 使用半透明黄色 (R:255, G:255, B:0, Alpha:100)
        painter.setBrush(QColor(0, 255, 255, 160));
        painter.setPen(Qt::NoPen); // 如果不想要外圈黑线就加这行
        painter.drawEllipse(x-15, y-15, 30, 30);

        // 写字母
        painter.setPen(Qt::black);
        painter.drawText(x-5, y+5, "B");
    }
    void fire(QList<Enemy*>& enemies, QList<Laser>& lasers) override {
        if (cooldown > 0) { cooldown--; return; }
        for (Enemy* e : enemies) {
            if (std::hypot(e->x-x, e->y-y) <= range) {
                e->hp -= damage; cooldown = maxCooldown;
                lasers.append({x, y, e->x, e->y, Qt::cyan}); break;
            }
        }
    }
};

class TCellTower : public Tower {
public:
    TCellTower(double x, double y) : Tower(x, y, 140, 25, 45) {}
    void draw(QPainter &painter) override {
        // 使用半透明黄色 (R:255, G:255, B:0, Alpha:100)
        painter.setBrush(QColor(0, 0, 255, 100));
        painter.setPen(Qt::NoPen); // 如果不想要外圈黑线就加这行
        painter.drawEllipse(x-15, y-15, 30, 30);

        // 写字母
        painter.setPen(Qt::black);
        painter.drawText(x-5, y+5, "T");
    }
    void fire(QList<Enemy*>& enemies, QList<Laser>& lasers) override {
        if (cooldown > 0) { cooldown--; return; }
        bool h = false;
        for (Enemy* e : enemies) {
            if (std::hypot(e->x-x, e->y-y) <= range) {
                e->hp -= damage; lasers.append({x, y, e->x, e->y, Qt::blue}); h = true;
            }
        }
        if (h) cooldown = maxCooldown;
    }
};

class TowerSlot {
public:
    double x, y;
    Tower* builtTower;
    TowerSlot(double tx, double ty) : x(tx), y(ty), builtTower(nullptr) {}
    ~TowerSlot() { if (builtTower) delete builtTower; }
    void draw(QPainter &painter) {
        if (builtTower) builtTower->draw(painter);
        else { painter.setPen(QPen(Qt::gray, 2, Qt::DashLine)); painter.drawEllipse(x-15, y-15, 30, 30); }
    }
};

#endif