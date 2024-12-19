#ifndef JOINT_H
#define JOINT_H
#pragma once
#include <cmath>
#include <stack>

struct Point {
    float x;
    float y;
    Point(float x, float y) : x(x), y(y) {}
    Point(const Point& right) : x(right.x), y(right.y) {}
};

class Joint {
private:
    Point point;
    float radius;

public:
    Joint* next;
    Joint(float radius, const Point point, Joint* next = nullptr);
    Joint(std::stack<float> rad, const Point point);
    ~Joint();

    void changecoords(Point coords);
    void move(Point coords);
    void checkPart(Joint& part);

    int size();
    Point getPoint();
    float getR();

private:
    Joint* create(std::stack<float>& rad, Point point);
};

#endif // JOINT_H
