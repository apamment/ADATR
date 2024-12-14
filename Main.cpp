#include <iostream>
extern "C" {
#include "magidoor/MagiDoor.h"
}
#include "Game.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage " << argv[0] << " DROPFILE [socket]" << std::endl;
        return -1;
    }

    int socket = -1;

    if (argc == 3) {
        socket = strtol(argv[2], NULL, 10);
    }

    md_init(argv[1], socket);

    Game g;
    g.run();

    md_exit(0);

}
