#ifndef SHAPE_H
#define SHAPE_H
#include <QPainter>

class Shape {
public:
    virtual ~Shape() {} // 虚析构函数

    virtual void draw(QPainter &painter) = 0;
    virtual void move() = 0;

    virtual double getArea() = 0;
    virtual double getPerimeter() = 0;

    int x, y;
};
#endif // SHAPE_H