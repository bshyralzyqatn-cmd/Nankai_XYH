#ifndef ENEMY_H
#define ENEMY_H
#include "shape.h"

class EnemySquare : public Shape {
public:
    EnemySquare(int startX, int startY) {
        x = startX;
        y = startY;
    }

    void draw(QPainter &painter) override {
        painter.setBrush(Qt::red);
        painter.drawRect(x, y, 30, 30);
    }

    void move() override {
        y += 6;
    }

    double getArea() override { return 30.0 * 30.0; }
    double getPerimeter() override { return 4.0 * 30.0; }
};
#endif // ENEMY_H