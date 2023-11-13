#include "../../inc/emulator/terminal.hpp"

int32_t Terminal::putChar(char c) {
        write(STDOUT_FILENO, &c, 1);
        return 0;
}

int32_t Terminal::getChar(char *c) {
        char d;
        if(read(STDIN_FILENO, c, 1) > 0) return 1;
        return 0;
}