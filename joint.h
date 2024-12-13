#ifndef JOINT_H
#define JOINT_H
#pragma once
//#include <vector>
#include <cmath>
#include <stack>

struct Point {
    float x;
    float y;
    Point(float x, float y) {this->x=x;this->y=y;}
    Point(const Point& right) {x=right.x; y=right.y;};
};

class Joint {
private:
    Point point;
    float angle;
    float radius;
public:
    //std::vector<Joint*> hands;
    Joint* next;
    //Joint() {}
    Joint(float radius, const Point point, int angle = 90, Joint* next = nullptr): point(point.x, point.y) {
        this->radius = radius;
        this->angle = angle;
        this->next = next;
        //hands = {};
    }
    Joint(std::stack<float> rad, const Point point): point(point.x, point.y) {
        this->radius = rad.top();
        rad.pop();
        this->angle = 90;
        this->next = create(rad, point);
        //hands = {};
    }
    Joint* create(std::stack<float>& rad, Point point) {
        if (rad.size()>1) {
            return new Joint(rad, point);
        }
        else {
            float val = rad.top();
            rad.pop();
            return new Joint(val, point, angle);
        }
        return this;
    }
    ~Joint() {
        while (next!=nullptr) {
            auto buf = next;
            next = next->next;
            delete buf;
        }
    }
    void AddJoint(float radius) {
        Joint* ptr = this;
        while (ptr->next!=nullptr) {ptr=ptr->next;}
        ptr->next = new Joint(radius, point, angle);
    }
    int calculate_angle(Point right) {
        Point delta(right.x-point.x, right.y-point.y);
        return (std::atan2(delta.x, delta.y)*180)/M_PI;
    }

    void changecoords(Point coords) {
        angle = calculate_angle(Point(point.x, point.y));
        point.x = coords.x;
        point.y = coords.y;
    }
    void move(Point coords) {
        changecoords(coords);
        if (next != nullptr) {
            checkPart(*next);
        }
    }

    void checkPart(Joint& part) {
        Point delta(point.x - part.point.x, point.y - part.point.y);
        float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        if (distance > part.radius+2) {
            float normal_x = (delta.x / distance);
            float normal_y =  (delta.y / distance);
            Point mvdelta(point.x - part.radius * normal_x, point.y - part.radius * normal_y);
            part.move(mvdelta);
        }
    }

    Point getPoint(){return point;}
    float getR(){return radius;}
    int getA(){return angle;}
};

#endif // JOINT_H
