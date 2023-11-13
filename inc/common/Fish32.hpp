#include <fstream>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <unordered_map>
#include "common_types.hpp"


#ifndef FISH_32
#define FISH_32
/* Formatted Intermediate Symbolic Handler */
class Fish32 {
public:
        Fish32(std::string f_name): file_name(f_name) {}

        int32_t writeLinkable(const SymTab& symbol_table, const SecTab& section_table, const MemTab& mem_init);
        int32_t writeExecutable(const SecTab& section_table, const MemTab& mem_init);

        int32_t readFile(FishHeader& fheader, SymTab& symbol_table, SecTab& section_table, MemTab& mem_init, RelaTab& rela_tab, SegTab& seg_tab);
        int32_t readExecutable(SegTab& mem_init);
        int32_t readLinkable(SymTab& symbol_table, SecTab& section_table, MemTab& mem_init, RelaTab& rela_tab);
protected:
        int32_t __writeExecutable(std::ofstream& write_file, const SecTab& section_table, const MemTab& mem_init);
        int32_t __writeLinkable(std::ofstream& write_file, const SymTab& symbol_table, const SecTab& section_table, const MemTab& mem_init);
        int32_t __readExecutable(std::ifstream& read_file, SegTab& mem_init);
        int32_t __readLinkable(std::ifstream& read_file, SymTab& symbol_table, SecTab& section_table, MemTab& mem_init, RelaTab& rela_tab);
private:
        std::string file_name;
        uint8_t magic_number = 0x4f;
        std::string magic_name = "FISH";
        FishHeader file_header;
};

#endif