extern "C" {
#include "magidoor/MagiDoor.h"
}
#include "Horse.h"

std::string names[] = {
"Gomer",
"Donkey",
"Radio Flyer",
"Lostma Cowboy",
"Klip Klop",
"Foalovit",
"Clodhopper",
"Waffle House",
"Ed",
"Bartholomew",
"LumberJack",
"Bucktooth",
"Loco",
"Sir Topham Hatt",
"General Tso",
"Carrots",
"Hall & Oats",
"Overbite",
"Sprinkles",
"Bandito",
"Lollygagger",
"Heisenberg",
"Hemi",
"Frankenstein",
"Horsepower",
"Moose",
"Bing Bong",
"Rainbow Unicorn",
"Rusty Saddle",
"Transmission",
"Crouching Tiger",
"Cowboy",
"Jalopy",
"Stable Boy",
"Foxtrot"
};


Horse::Horse(int number, int y, int x) {
    switch (number) {
        case 1:
        this->colour = "bright magenta";
        this->name = names[rand() % 7];
        break;
        case 2:
        this->colour = "bright cyan";
        this->name = names[rand() % 7 + 6];
        break;
        case 3:
        this->colour = "bright yellow";
        this->name = names[rand() % 7 + 13];
        break;
        case 4:
        this->colour = "bright red";
        this->name = names[rand() % 7 + 20];
        break;
        case 5:
        this->colour = "bright blue";
        this->name = names[rand() % 7 + 27];
        break;        
    }
    this->x = x;
    this->y = y;
    this->pos = 1;
    this->place = 0;
}

bool Horse::inc_pos() {
    pos++;
    if (pos == 54) return true;

    return false;
}

void Horse::draw() {
    md_printf("\x1b[%d;%dH`%s` ,\xd6\xb7^`white`", y, x + pos - 1, colour.c_str());
}

void Horse::set_place(int place) {
    this->place = place;
    md_printf("\x1b[%d;63H%d", y, place);
}

int Horse::get_place() {
    return place;
}
