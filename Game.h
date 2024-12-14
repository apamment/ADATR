#pragma once

#include "sqlite3.h"

class Game {
public:
    int run();
private:
    int open_player_db(sqlite3 **db);
    void load_player();
    void display_scores();
    void save_player();
    bool play_game();
    uint32_t coin;
    time_t lastplayed;
    int turns;
};
