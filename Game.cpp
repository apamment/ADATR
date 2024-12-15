extern "C" {
#include "magidoor/MagiDoor.h"
}
#include <vector>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include "sqlite3.h"
#include "Game.h"
#include "Horse.h"

void Game::display_scores() {
	sqlite3 *db;
	sqlite3_stmt *res;
	int i;
	const char *sql = "SELECT username, coin FROM players ORDER BY coin DESC LIMIT 10";
	
	if (!open_player_db(&db)) {
		md_exit(0);
	}
	
	if (sqlite3_prepare_v2(db, sql, strlen(sql), &res, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		md_exit(0);
	}
	
	md_clr_scr();
	md_sendfile("ansi/scores.ans", FALSE);
	
	i = 0;
	while (sqlite3_step(res) == SQLITE_ROW) {
		md_printf("\x1b[%d;%dH", 9 + i, 13);
		md_printf("`bright white`%s", sqlite3_column_text(res, 0));
		md_printf("\x1b[%d;%dH", 9 + i, 45);
		md_printf("`bright cyan`%u", sqlite3_column_int(res, 1));
		i++;
	}
    sqlite3_finalize(res);
    sqlite3_close(db);
	md_printf("\x1b[22;26H`white`Press any key to continue...");
	md_getc();
}

int Game::open_player_db(sqlite3 **db) {
	int rc;
	sqlite3_stmt *res;
	const char *sql = "CREATE TABLE IF NOT EXISTS players (id INTEGER PRIMARY KEY, username TEXT COLLATE NOCASE, coin INTEGER, turns INTEGER, lastplayed INTEGER)";
	rc = sqlite3_open("players.db3", db);
    if (rc) {
		// Error opening the database
        return 0;
    }
    sqlite3_busy_timeout(*db, 5000);
    
    rc = sqlite3_exec(*db, sql, 0, 0, NULL);
	if (rc != SQLITE_OK) {
		sqlite3_close(*db);
		return 0;
	}
    return 1;
}

void Game::load_player() {
	sqlite3 *db;
	sqlite3_stmt *res;
	
	const char *sql = "SELECT coin, turns, lastplayed FROM players WHERE username = ?;";
	
	if (!open_player_db(&db)) {
		md_exit(0);
	}
	
	if (sqlite3_prepare_v2(db, sql, strlen(sql), &res, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		md_exit(0);
	}
	
	sqlite3_bind_text(res, 1, mdcontrol.user_alias, -1, 0);
	
	if (sqlite3_step(res) == SQLITE_ROW) {
		coin = sqlite3_column_int(res, 0);
		turns = sqlite3_column_int(res, 1);
		lastplayed = sqlite3_column_int(res, 2);
	} else {
		coin = 1000;
		turns = 15;
		lastplayed = time(NULL);
	}
	
	sqlite3_finalize(res);
	sqlite3_close(db);
}

void Game::save_player() {
	sqlite3 *db;
	sqlite3_stmt *res;
	
	const char *sql_c = "SELECT username FROM players WHERE username = ?;";
	const char *sql_i = "INSERT INTO players (username, coin, turns, lastplayed) VALUES(?, ?, ?, ?)";
	const char *sql_u = "UPDATE players SET coin = ?, turns = ?, lastplayed = ? WHERE username = ?";
	
	if (!open_player_db(&db)) {
		md_exit(0);
	}
	
	if (sqlite3_prepare_v2(db, sql_c, strlen(sql_c), &res, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		md_exit(0);
	}
	
	sqlite3_bind_text(res, 1, mdcontrol.user_alias, -1, 0);
	
	if (sqlite3_step(res) == SQLITE_ROW) {
		sqlite3_finalize(res);
		if (sqlite3_prepare_v2(db, sql_u, strlen(sql_u), &res, NULL) != SQLITE_OK) {
			sqlite3_close(db);
			md_exit(0);
		}
		sqlite3_bind_int(res, 1, coin);
		sqlite3_bind_int(res, 2, turns);
		sqlite3_bind_int(res, 3, lastplayed);
		sqlite3_bind_text(res, 4, mdcontrol.user_alias, -1, 0);
		
		sqlite3_step(res);
	} else {
		sqlite3_finalize(res);
		if (sqlite3_prepare_v2(db, sql_i, strlen(sql_i), &res, NULL) != SQLITE_OK) {
			sqlite3_close(db);
			md_exit(0);
		}
		sqlite3_bind_text(res, 1, mdcontrol.user_alias, -1, 0);
		sqlite3_bind_int(res, 2, coin);
		sqlite3_bind_int(res, 3, turns);
		sqlite3_bind_int(res, 4, lastplayed);
	
		sqlite3_step(res);		
	}
	
	sqlite3_finalize(res);
	sqlite3_close(db);
}

int Game::run() {
    time_t now;
    struct tm thetime;
    struct tm oldtime;
    bool done = false;

    md_clr_scr();
    md_sendfile("ansi/intro.ans", false);
    md_getc();

    bool shouldquit = false;

    load_player();

	now = time(NULL);
    srand(now);
#if defined(_WIN32) || defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
	localtime_s(&thetime, &now);
	localtime_s(&oldtime, &lastplayed);
#else
	localtime_r(&now, &thetime);
	localtime_r(&lastplayed, &oldtime);
#endif	
	if (thetime.tm_yday != oldtime.tm_yday || thetime.tm_year != oldtime.tm_year) {
		turns = 15;
        coin += 100;
		lastplayed = now;
        save_player();
	}

    while (!done) {   
        md_clr_scr();
        md_sendfile("ansi/start.ans", FALSE);

        md_printf("\x1b[17;48H`bright yellow`%d", coin);
        md_printf("\x1b[18;48H`bright white`%d", turns);
        char c = md_getc();

        switch(tolower(c)) {
            case 'p':
                if (turns > 0 && coin > 0) {
                    while (!shouldquit) {
                        shouldquit = play_game();
                    }
                    shouldquit = false;
                } else if (turns == 0) {
                    md_clr_scr();
                    md_sendfile("ansi/closed.ans", FALSE);
                    md_getc();
                    done = true;
                } else {
                    md_clr_scr();
                    md_sendfile("ansi/nocoin.ans", FALSE);
                    md_getc();
                    done = true;
                }
                break;
            case 'v':
                display_scores();
                break;
            case 'q':
                done = true;
                break;
        }
    }
    display_scores();

    return 0;
}

bool Game::play_game() {
    std::vector<Horse *> horses;
    const char *places[] = {"1st", "2nd", "3rd", "4th", "5th"};
    char buffer[10];
    md_clr_scr();
    md_sendfile("ansi/track.ans", false);
    int ret = false;
    Horse *h;

    md_printf("\x1b[13;3H`bright white`Your Coin `bright yellow`$%d`white`\x1b[K", coin);

    h = new Horse(1, 3, 5);
    horses.push_back(h);
    h = new Horse(2, 5, 5);
    horses.push_back(h);
    h = new Horse(3, 7, 5);
    horses.push_back(h);
    h = new Horse(4, 9, 5);
    horses.push_back(h);
    h = new Horse(5, 11, 5);
    horses.push_back(h);


    for (size_t i = 0; i < horses.size(); i++) {
        horses.at(i)->draw();
        md_printf("\x1b[%d;65H`%s`%s`white`", 3 + i * 2, horses.at(i)->colour.c_str(), horses.at(i)->name.c_str());
    }

    md_printf("\x1b[15;3HBet on Horse `bright magenta`1`white`, `bright cyan`2`white`, `bright yellow`3`white`, `bright red`4`white` or `bright blue`5`white`? (Q to Quit)\x1b[K");
    char ch = md_get_answer("12345qQ\x1b");

    if (ch == 'q' || ch == 'Q' || ch == '\x1b') {
        for (size_t i = 0; i < horses.size();i++) {
            delete horses.at(i);
        }
        return true;
    }

    

    int beton = ch - '1';

    md_printf("\x1b[16;3HTo (W)in or (P)lace? (W/P)\x1b[K");
    bool towin = true;

    towin = (tolower(md_get_answer("wWpP")) == 'w');

    if (towin) {
        md_printf("\x1b[%d;64H`bright green`\xfb`white`", 3 + beton * 2);
    } else {
        md_printf("\x1b[%d;64H`bright black`\xfb`white`", 3 + beton * 2);
    }


    int amount = 0;

    md_printf("\x1b[17;3HBet how much? $\x1b[K");
    md_getstring(buffer, 4, '0', '9');
    amount = strtol(buffer, NULL, 10);
    if (amount > 0 && amount <= coin) {
        md_printf("\x1b[18;3HReady to Race? (Y/N)\x1b[K");
        char c = md_get_answer("YyNn");
        if (c == 'y' || c == 'Y') {
            coin -= amount;
            md_printf("\x1b[13;3H`bright white`Your Coin `bright yellow`$%d`white`\x1b[K", coin);
            turns--;
            save_player();
            bool done = false;
            int place = 0;

            while (!done) {

                int horse;

                do {
                    horse = rand() % 5;
                } while (horses.at(horse)->get_place() != 0);

                usleep(100000);

                horses.at(horse)->draw();

                if (horses.at(horse)->inc_pos()) {
                    horses.at(horse)->set_place(++place);
                    md_printf("\x1b[1;3H`%s`%s`white` comes in at %s place\x1b[K", horses.at(horse)->colour.c_str(), horses.at(horse)->name.c_str(), places[place - 1]);
                } else {
                    if (rand() % 4 == 1 && place == 0) {
                        switch(rand() % 5) {
                            case 0:
                                md_printf("\x1b[1;3H`%s`%s`white` has picked up speed!\x1b[K", horses.at(horse)->colour.c_str(), horses.at(horse)->name.c_str());
                                break;
                            case 1:
                                md_printf("\x1b[1;3H`%s`%s`white` surges forward!\x1b[K", horses.at(horse)->colour.c_str(), horses.at(horse)->name.c_str());
                                break;
                            case 2:
                                md_printf("\x1b[1;3H`%s`%s`white` makes a break for it!\x1b[K", horses.at(horse)->colour.c_str(), horses.at(horse)->name.c_str());
                                break;
                            case 3:
                                md_printf("\x1b[1;3H`%s`%s`white` can taste victory!\x1b[K", horses.at(horse)->colour.c_str(), horses.at(horse)->name.c_str());
                                break;
                            case 4:
                                md_printf("\x1b[1;3H`%s`%s`white` has 1st in it's sights!\x1b[K", horses.at(horse)->colour.c_str(), horses.at(horse)->name.c_str());
                                break;

                        }
                    }
                }

                if (place == 5) done = true;

            }

            for (int i=0;i<7;i++) {
                md_printf("\x1b[%d;3H\x1b[K", 14 + i);
            }

            if (towin) {
                if (horses.at(beton)->get_place() == 1) {
                    md_printf("\x1b[14;3H`bright green`You Win! ($%d)", amount * 4);
                    coin += amount * 4;
                } else {
                    md_printf("\x1b[14;3H`bright red`You Lose!");
                }
            } else {
                if (horses.at(beton)->get_place() <= 3) {
                    md_printf("\x1b[14;3H`bright green`You Win! ($%d)", amount * 2);
                    coin += amount * 2;
                } else {
                    md_printf("\x1b[14;3H`bright red`You Lose!");
                }
            }
            save_player();
            md_printf("\x1b[13;3H`bright white`Your Coin `bright yellow`$%d`white`\x1b[K", coin);

            md_printf("\x1b[16;3H`bright white`Play Again? (Y/N)\x1b[K");
            c = md_get_answer("YyNn");
            if (c == 'n' || c == 'N' || coin == 0 || turns == 0) {
                ret = true;
            }
        }
    }


    for (size_t i = 0; i < horses.size();i++) {
        delete horses.at(i);
    }


    return ret;
}
