#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "../../inc/common/common_types.hpp"

#ifndef LINKER_HPP
#define LINKER_HPP

class Linker {
public:
        typedef struct {
                SymTab file_symbol_table;
                SecTab file_section_table;
                MemTab file_memory_table;
                RelaTab file_reloation_table;
        } FileArgs;

        Linker(std::vector<std::string> f_names, std::string o_file, std::map<uint32_t, std::string> s_placements): 
                file_names(f_names), output_file(o_file), section_placements(s_placements) {} 

        int32_t writeExecutable();
        int32_t writeRelocatable();
        static void writeHelp();
protected:
        /* pass file descriptions which contain that section */
        int32_t placeSection(std::vector<FileArgs>& file_args, std::string section_name, uint32_t address, SecType type);
        int32_t placeRelocs(std::vector<FileArgs>& file_args, std::string target_section);
        int32_t __placeSection(SecEntry& section, MemTab& sec_mem,uint32_t address);
        int32_t __addSymbol(SymEntry& symbol);
        int32_t __addSymbol(SecEntry& section);
        int32_t readFile(std::vector<FileArgs>& file_args);
        int32_t __section(std::string section_name, SecType type);
        int32_t init_mem(const char* buf, int32_t size, std::string section);
        void __writeStrtab();
        void __writeSymtab();
private:
        SymTab symbol_table;
        SecTab section_table;
        MemTab memory_table;
        RelaTab relocation_table; 
        std::vector<std::string> file_names;
        std::string output_file;
        std::map<uint32_t, std::string> section_placements;
        /* counters, again */
        uint32_t placement_address = 0;
        uint32_t section_table_index = 0;
        uint32_t symbol_table_index = 0;
};

#endif