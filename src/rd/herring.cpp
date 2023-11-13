#include "../../inc/rd/herring.hpp"
#include <algorithm>
#include <iomanip>

void Herring::writeSymbols() {
        std::cout << "contents of section symtab" << std::endl;
        std::cout << std::right << "NUM:\t" << "VALUE\t" << "TYPE\t" << "  BIND\t" << "SECTION\t" <<"  NAME" << std::endl;
        for(auto &s : sym_tab) {
                SymEntry& sym = s.second;

                std::string bind_str;
                switch (sym.bind) {
                        case LOCAL_BIND:
                                bind_str = "LOCAL";
                                break;
                        case GLOBAL_BIND:
                                bind_str = "GLOBAL";
                                break;
                        case EXTERN_BIND:
                                bind_str = "EXTERN";
                        break;
                }

                std::string type_str;
                switch (sym.type) {
                        case ABS_TYPE:
                                type_str = "ABS";
                                break;
                        case SECTION_TYPE:
                                type_str = "SECTION";
                                break;
                        default:
                                type_str = "UNDEFINED";
                                break;
                }
                std::cout << std::right << std::setfill(' ') << std::setw(4) << std::hex << std::setfill('0') << sym.symbol_index << ": ";
                std::cout << std::setfill('0') << std::setw(8) << sym.value << " ";
                std::cout << std::left << std::setfill(' ') << std::setw(9) << type_str << " ";
                std::cout << std::left << std::setw(6) << bind_str << "    ";
                std::cout << std::setw(5) << sym.section_entry << " ";
                std::cout << sym.symbol_name << std::endl;
        }
}

void Herring::writeSections() {
        std::cout << "Section Headers" << std::endl;
        std::cout << "[Nr] Name             Type        Offset\n";
        std::cout << "     Size             Link Info\n";

        for(auto& s : sec_tab) {
            SecEntry& sec = s.second;

            std::string  type_str;
            switch(sec.type) {
                case NULL_SECTION:
                        type_str = "NULL";
                        break;
                case PROGBITS_SECTION:
                        type_str = "PROGBITS";
                        break;
                case RELA_SECTION:
                        type_str = "RELA";
                        break;
                case SYMTAB_SECTION:
                        type_str = "SYMTAB";
                        break;
                case STRTAB_SECTION:
                        type_str = "STRTAB";
                        break;
                }
                std::string short_name = sec.section_name.size() > 17? sec.section_name.substr(0, 13) + "[...]" : sec.section_name;

                std::cout << "[" << std::hex << std::setw(1) << sec.section_index << "] ";
                std::cout << std::left << std::setw(17) << std::setfill(' ') << short_name << " ";
                std::cout << std::left << std::setw(8) << type_str << "   ";
                std::cout << std::right << std::setw(16) << std::setfill('0') << std::hex << sec.offset << std::endl;
                std::cout << "     ";
                std::cout << std::right << std::setw(16) << std::setfill('0') << sec.size << " ";
                std::cout << std::left << std::setw(1) << sec.link << "     ";
                std::cout << std::setw(1) << sec.info << std::endl;
        } 
}

void Herring::writeHeader() {

}

void Herring::writeRela() {
        for(auto& r : relas_tab) {
                std::cout << "Reloaction section '" << r.first << "' contains " <<  r.second.size() << " entries" << std::endl;
                std::cout << "Offset" << "        " << "Sym. Value" << "     "  << "Addend" << "    " << "Sym. name" << std::endl;    
                for(auto& entry : r.second) {
                        SymEntry sym = std::find_if(sym_tab.begin(), 
                                                    sym_tab.end(), 
                                                    [&cur = entry.symbol_index] (auto& test_s) -> bool { return test_s.second.symbol_index == cur; })->second;
                
                        std::cout << std::hex << std::right << std::setfill('0') << std::setw(8) << entry.offset << "      ";
                        std::cout << std::setw(8) << sym.value << "       " << std::setw(4) << entry.addend << "       ";
                        std::cout << std::left << sym.symbol_name << std::endl;
                }
        }
}

void Herring::writeHelp(){
        std::cout << "Usage: herring [OPTIONS] file" << std::endl;
        std::cout << "Options:\n";
        std::cout << "  -h, --help             Display this help message" << std::endl;;
        std::cout << "  -S, --section-headers  Show section headers" << std::endl;
        std::cout << "  -s, --symbols          Show symbol table" << std::endl;
        std::cout << "  -r, --relocs           Show relocation sections" << std::endl;
        std::cout << "  -x, --hexdump          Show memory contents of each section/program segment" << std::endl;
}
/* no point if i dont have an executable file as an output */
void Herring::writeMem(){
        uint32_t start_address = 0;
        if(fheader.file_type == EXECUTABLE_FILE) {
                __writeExecMem();
        } else {
                __writeLinkMem();
        }
}

void Herring::__writeExecMem() {
        for(auto s : seg_tab) {
                std::cout << "Memory contents at location: " << std::hex << s.first << std::endl;
                uint32_t start_address = (s.first / 8) * 8;
                std::vector<uint8_t> data(s.second->tellp());
                s.second->read(reinterpret_cast<char*>(&data[0]), data.size());

                for(int32_t i = start_address; i < s.first; i++) {
                        data.insert(data.begin(), 0x00);
                }

                uint32_t end_zeroes = (8 - data.size() % 8) % 8;
                data.insert(data.end(), end_zeroes, 0x00);

                for(uint8_t byte : data) {
                        if(start_address % 8 == 0) {
                                std::cout << std::hex << std::right << std::setfill('0') << std::setw(4) << start_address << ": ";
                        }

                        std::cout << std::setw(2) << static_cast<uint32_t>(byte) << " ";
                        if (start_address % 8 == 7) {
                                std::cout << std::endl;
                        }

                        start_address++;
                }
                std::cout << std::endl;
        }
}

void Herring::__writeLinkMem() {
        for(auto s : mem_init) {
                std::cout << "memory contents for section: " << std::hex << s.first << std::endl;
                uint32_t start_address = 0;
                std::vector<uint8_t> data(s.second->tellp());
                s.second->read(reinterpret_cast<char*>(&data[0]), data.size());

                uint32_t end_zeroes = (8 - data.size() % 8) % 8;
                data.insert(data.end(), end_zeroes, 0x00);

                for(uint8_t byte : data) {
                        if(start_address % 8 == 0) {
                                std::cout << std::hex << std::right << std::setfill('0') << std::setw(4) << start_address << ": ";
                        }

                        std::cout << std::setw(2) << static_cast<uint32_t>(byte) << " ";
                        if (start_address % 8 == 7) {
                                std::cout << std::endl;
                        }

                        start_address++;
                }
                std::cout << std::endl;
        }
}