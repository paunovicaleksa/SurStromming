#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <cstdint>

#ifndef TERMINAL_HPP
#define TERMINAL_HPP
#define TERM_OUT 0xFFFFFF00u
#define TERM_IN 0xFFFFFF04u

class Terminal {
public:
        Terminal() {
                tcgetattr(STDIN_FILENO, &oldstdio); 
                memset(&stdio,0,sizeof(stdio));
                stdio.c_iflag=0;
                stdio.c_oflag=0;
                stdio.c_cflag=0;
                stdio.c_lflag=0;
                stdio.c_cc[VMIN]=1;
                stdio.c_cc[VTIME]=0;
                tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
                tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
                fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); 
        }

        ~Terminal() {
                tcsetattr(STDOUT_FILENO, TCSAFLUSH ,&oldstdio);
        }

        int32_t getChar(char* c);
        int32_t putChar(char c);
private:
        struct termios oldstdio;
        struct termios stdio;
};

#endif