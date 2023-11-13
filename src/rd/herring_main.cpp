#include "../../inc/rd/herring.hpp"
#include <iostream>
#include <getopt.h>

int main(int argc, char* argv[]) {
        const struct option longopts[] = {
                {"help", no_argument, 0, 'h'},
                {"section-headers", no_argument, 0, 'S'},
                {"symbols", no_argument, 0, 's'},
                {"relocs", no_argument, 0, 'r'},
                {"hexdump", no_argument, 0, 'x'},
                {0, 0, 0, 0}
        };

        if(argc < 2) {
                return 1;
        }
        bool show_help = false, section_headers = false, symbols = false, relocs = false, hexdump = false;
        int32_t iarg = 0;
        int32_t index;
        while((iarg = getopt_long(argc, argv, "hsrxS", longopts, &index)) != -1) {
                switch(iarg) {
                        case 'h':
                                Herring::writeHelp();
                                return 0;
                                break;
                        case 'S':
                                section_headers = true;
                                break;
                        case 's':
                                symbols = true;
                                break;
                        case 'r':
                                relocs = true;
                                break;
                        case 'x' :
                                hexdump = true;
                                break;
                        default:
                                Herring::writeHelp();
                                return 1;
                                break;
                }
        }
        /* parse actual arguments i guess */
        if(optind == argc ) {
                std::cout << "you must input file." << std::endl;
                return 1;
        } 
        
        std::string filename = argv[optind];
        Herring herring(filename);

        if(symbols) herring.writeSymbols();
        if(section_headers) herring.writeSections();
        if(relocs) herring.writeRela();
        if(hexdump) herring.writeMem();
}