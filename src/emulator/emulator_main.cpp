#include "../../inc/emulator/emulator.hpp"



int main(int argc, char* argv[]) {
        if(argc < 2){
                std::cout << "Usage: emulator [input_file]" << std::endl;
                return 1;
        }
        Emulator emu(argv[1]);
        if(emu.emulate()) {
                return 1;
        }

        return 0;
}