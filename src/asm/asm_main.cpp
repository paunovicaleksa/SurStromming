#include <cstdio>
#include <iostream>
#include "../../inc/asm/assembler.hpp"
#include "../../inc/common/Fish32.hpp"
#include <getopt.h>

Assembler* asem;

int main(int argc, char* argv[]){

        int32_t index;
        int32_t iarg = 0;
        std::string input_file = "";
        std::string output_file = "";
        std::string error_message =  "Usage: assembler -o [out_file] [input_file]";
        while((iarg = getopt(argc, argv, "o:")) != -1) {
                switch(iarg) {
                        case 'o': 
                                output_file = optarg; 
                                break;
                        default:
                                std::cout << error_message << std::endl;
                                return 0;
                }
        }

        if(optind == argc) {
                std::cout << error_message << std::endl;
                return 1;
        }

        input_file = argv[optind];

        asem = new Assembler(input_file, output_file);
        if(asem->firstPass()){
                return 1;       
        }
        
        if(asem->secondPass()){
                return 1;       
        }

        return 0;
}