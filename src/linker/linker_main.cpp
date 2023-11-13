#include <iostream>
#include "../../inc/linker/linker.hpp"
#include <getopt.h>
#include <map>

int main(int argc, char* argv[]) {
        const struct option longopts[] = {
                {"help", no_argument, 0, 'h'},
                {"hex", no_argument, 0, 'x'},
                {"place", required_argument, 0, 'p'},
                {"out", required_argument, 0, 'o'},
                {0, 0, 0, 0}
        };

        int32_t iarg = 0;
        std::map<uint32_t, std::string> place_args;
        std::string option_arg;
        std::string output_file = "";
        uint64_t pos;
        bool show_help = false, hex = false, place = false;
        while((iarg = getopt_long_only(argc, argv, "o:hxp:", longopts, nullptr)) != -1) {
                switch(iarg) {
                        case 'o':
                                output_file = optarg;
                                break;
                        case 'h':
                                Linker::writeHelp();
                                return 0;
                                break;
                        case 'x':
                                hex = true;
                                break;
                        case 'p':
                                option_arg = optarg;
                                pos = option_arg.find('@');
                                if(pos != std::string::npos) {
                                        uint32_t address = std::stol(option_arg.substr(pos + 1), nullptr, 16);
                                        std::string section_name = option_arg.substr(0, pos);
                                        place_args.insert(std::pair(address, section_name));
                                } else {
                                        Linker::writeHelp();
                                        return 1;
                                }
                                break;
                        default:
                                Linker::writeHelp();
                                return 1;
                                break;
                }
        }

        if(optind == argc || output_file == "") {
                Linker::writeHelp();
                return 1;
        }

        std::vector<std::string> input_files;
        for(int32_t i = optind; i < argc; i++) {
                input_files.push_back(argv[i]);
        }

        Linker linker(input_files, output_file, place_args);
        if(hex) linker.writeExecutable();
}