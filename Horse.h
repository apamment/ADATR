#pragma once

#include <string>

class Horse {
public:
    Horse(int number, int y, int x);
    void draw();
    bool inc_pos();
    void set_place(int place);
    int get_place();
    std::string name;
    std::string colour;
private:
    int pos;
    int place;
    int x;
    int y;

};
