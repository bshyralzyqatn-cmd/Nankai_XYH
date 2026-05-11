#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include <QPainter>
#include <QPointF>
#include <QList>
#include <cmath>

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
        painter.setBrush(Qt::red); painter.drawRect(x - 10, y - 15, 20, 4);
        painter.setBrush(Qt::green); painter.drawRect(x - 10, y - 15, 20 * (hp / maxHp), 4);
        drawShape(painter);
    }
    virtual void drawShape(QPainter &painter) = 0;
};

class Bacteria : public Enemy {
public:
    Bacteria(double x, double y) : Enemy(x, y, 100, 2.5, 10) {}
    void drawShape(QPainter &painter) override { painter.setBrush(Qt::darkGreen); painter.drawEllipse(x-10, y-10, 20, 20); }
};

class Virus : public Enemy {
public:
    Virus(double x, double y) : Enemy(x, y, 60, 4.5, 15) {}
    void drawShape(QPainter &painter) override { painter.setBrush(Qt::darkMagenta); painter.drawRect(x-10, y-10, 20, 20); }
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
        painter.setBrush(Qt::yellow); painter.drawEllipse(x-15, y-15, 30, 30);
        painter.setPen(Qt::black); painter.drawText(x-5, y+5, "M");
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
        painter.setBrush(Qt::cyan); painter.drawEllipse(x-15, y-15, 30, 30);
        painter.setPen(Qt::black); painter.drawText(x-5, y+5, "B");
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
        painter.setBrush(Qt::blue); painter.drawEllipse(x-15, y-15, 30, 30);
        painter.setPen(Qt::white); painter.drawText(x-5, y+5, "T");
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