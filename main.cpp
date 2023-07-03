#include "serverlistener.h"
#include <iostream>

int main(int argc, char **argv) {
    try {
        ServerListener(argc > 1 ? argv[1] : "80").run();
    } catch (std::exception const &e) {
        std::cout << e.what() << "\n";
    }
    return 0;
}
