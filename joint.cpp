#include "joint.h"

Joint::Joint(float radius, const Point point, Joint* next)
    : point(point.x, point.y), radius(radius), next(next) {}

Joint::Joint(std::stack<float> rad, const Point point)
    : point(point.x, point.y) {
    this->radius = rad.top();
    rad.pop();
    this->next = create(rad, point);
}

Joint::~Joint() {
    while (next != nullptr) {
        auto buf = next;
        next = next->next;
        delete buf;
    }
}

void Joint::changecoords(Point coords) {
    point.x = coords.x;
    point.y = coords.y;
}

void Joint::move(Point coords) {
    changecoords(coords);
    if (next != nullptr) {
        checkPart(*next);
    }
}

void Joint::checkPart(Joint& part) {
    Point delta(point.x - part.point.x, point.y - part.point.y);
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    if (distance > part.radius) {
        float normal_x = (delta.x / distance);
        float normal_y = (delta.y / distance);
        Point mvdelta(point.x - part.radius * normal_x, point.y - part.radius * normal_y);
        part.move(mvdelta);
    }
}

Point Joint::getPoint() {
    return point;
}
int Joint::size() {
    auto buf = this->next;
    int counter = 1;
    while(buf!=nullptr) {
        buf = buf->next;
        counter++;
    }
    return counter;
}
float Joint::getR() {
    return radius;
}

Joint* Joint::create(std::stack<float>& rad, Point point) {
    if (rad.size() > 1) {
        return new Joint(rad, point);
    } else {
        float val = rad.top();
        rad.pop();
        return new Joint(val, point);
    }
    return this;
}
