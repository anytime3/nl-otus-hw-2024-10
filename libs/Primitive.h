#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <memory>
#include <iostream>

class Primitive {
public:
    virtual ~Primitive() = default;
    virtual void draw() const = 0;
};

class Circle : public Primitive {
public:
    void draw() const override {
        std::cout << "Drawing Circle" << std::endl;
    }
};

class Rectangle : public Primitive {
public:
    void draw() const override {
        std::cout << "Drawing Rectangle" << std::endl;
    }
};

#endif // PRIMITIVE_H
