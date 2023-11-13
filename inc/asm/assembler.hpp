#include <deque>
#include <vector>
#include <sstream>
#include <memory>
#include <cstdio>
#include <unordered_map>
#include <iostream>
#include <unistd.h>
#include "asm_types.hpp"
#include "../common/common_types.hpp"

#ifndef ASM_HPP
#define ASM_HPP

const std::string null_section = "";
const std::string symtab = "symtab";
const std::string strtab = "strtab";
const std::string shstrtab = "shstrtab";

class Assembler{
public:
        Assembler(std::string input_f, std::string output_f): current_section(""), location_counter(0), pass(0), input_file(input_f), output_file(output_f) {
                __section(null_section, NULL_SECTION);
                __section(symtab, SYMTAB_SECTION);
                __section(strtab, STRTAB_SECTION);
                __section(shstrtab, STRTAB_SECTION);
        }


        int32_t getPass() const { return pass; }
        std::string getCurrentSection() const { return current_section; }

        int32_t firstPass();
        int32_t secondPass();

        int32_t addSymbol(std::string sym_name, SymBind bind, bool is_definition, SymType type);
        int32_t checkSymbol(std::string sym_name, SymBind bind, SymType type); 
        int32_t addSection(std::string sec_name);

        void incCounter(uint32_t val) { location_counter += val; }
        void endPass();

        int32_t initLiteral(uint32_t value, std::string section);
        int32_t initSymbol(std::string symbol_name, std::string section);
        int32_t initWords(std::deque<ParserArg>& word_args);
        int32_t initSkip(uint32_t cnt);

        int32_t initInstruction(InstrDesc instruction);
protected:
        int32_t __displacement(uint8_t opcode, InstrDesc instruction);
        int32_t __reloc(RelocEntry reloc_entry, std::string section);
        int32_t __section(std::string section_name, SecType type);
        void __writeStrtab();
        void __writeSymtab();
        int32_t __init_mem(const char* buf, int32_t size, std::string section_name);
        Opcodes __getOpcode(InstrDesc instruction);
private:
        std::string input_file;
        std::string output_file;
        uint32_t location_counter;
        std::string current_section;
        SymTab sym_tab;
        SecTab sec_tab;
        MemTab mem_init;
        std::vector<uint32_t> literal_pool; /* literal pool for current section, should be enough? */
        std::unordered_map<std::string, uint32_t> symbol_pool;
        /* counters */
        int32_t pass;
        uint32_t section_counter = 0;
        uint32_t symbol_counter = 0;
};

#endif