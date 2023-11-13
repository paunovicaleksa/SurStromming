#include "../../inc/asm/assembler.hpp"
#include "../../inc/common/Fish32.hpp"
#include <cmath>
#include <memory>
#include <vector>
#include <algorithm>

extern int yyparse();
extern FILE* yyin;

#define VALID_VALUE(value) \
    ((value) >= -2048 && (value) <= 2047)

    
std::unordered_map<std::string, Registers> register_mappings {
        {"%r0", R0},
        {"%r1", R1},
        {"%r2", R2},
        {"%r3", R3},
        {"%r4", R4},
        {"%r5", R5},
        {"%r6", R6},
        {"%r7", R7},
        {"%r8", R8},
        {"%r9", R9},
        {"%r10", R10},
        {"%r11", R11},
        {"%r12", R12},
        {"%r13", R13},
        {"%r14", R14},
        {"%r15", R15},
        {"%status", STATUS},
        {"%handler", HANDLER},
        {"%cause", CAUSE},
        {"%sp", R14}, 
        {"%pc", R15}  
};

Opcodes Assembler::__getOpcode(InstrDesc instruction) {
        std::unordered_map<AsmInstruction, Opcodes> direct_translations{
                {HALT_ASM, HALT_PROC},
                {INT_ASM, INT_PROC},
                {XCHG_ASM, XCHG_PROC},
                {ADD_ASM, ADD_PROC},
                {SUB_ASM, SUB_PROC},
                {MUL_ASM, MUL_PROC},
                {DIV_ASM, DIV_PROC},
                {NOT_ASM, NOT_PROC},
                {AND_ASM, AND_PROC},
                {OR_ASM, OR_PROC},
                {XOR_ASM, XOR_PROC},
                {SHL_ASM, SHL_PROC},
                {SHR_ASM, SHR_PROC},
                {CSRRD_ASM, CSRRD_PROC},
                {CSRWR_ASM, CSRWR_PROC},
                {LD_REG_DIR_ASM, LD_REG_DIR_PROC},
                {LD_REG_IND_ASM, LD_REG_IND_PROC},
                {LD_REG_IND_DISP_ASM, LD_REG_IND_PROC},
                {ST_REG_DIR_ASM, LD_REG_DIR_PROC},
                {ST_REG_IND_ASM, ST_REG_IND_PROC},
                {ST_REG_IND_DISP_ASM, ST_REG_IND_PROC},
                {PUSH_ASM, PUSH_PROC},
                {POP_ASM, POP_PROC},
                {RET_ASM, POP_PROC},
                {IRET_ASM, CSRRD_MEM_PROC},
                {LD_MEM_DIR_ASM, LD_REG_IND_PROC}
        };

        std::unordered_map<AsmInstruction, Opcodes> reg_dir {
                {CALL_ASM, CALL_PROC},
                {JMP_ASM, JUMP_PROC},
                {BGT_ASM, BGT_PROC},
                {BEQ_ASM, BEQ_PROC},
                {BNE_ASM, BNE_PROC},
                {LD_IMMED_ASM, LD_REG_DIR_PROC},
                {ST_MEM_DIR_ASM, ST_REG_IND_PROC}
        };

        std::unordered_map<AsmInstruction, Opcodes> reg_ind {
                {CALL_ASM, LONG_CALL_PROC},
                {JMP_ASM, LONG_JUMP_PROC},
                {BGT_ASM, LONG_BGT_PROC},
                {BEQ_ASM, LONG_BEQ_PROC},
                {BNE_ASM, LONG_BNE_PROC},
                {LD_IMMED_ASM, LD_REG_IND_PROC},
                {ST_MEM_DIR_ASM, ST_MEM_IND_PROC}
        };

        if(direct_translations.count(instruction.asm_instruction)) {
                return direct_translations[instruction.asm_instruction];
        }

        if(instruction.arg.arg_type == SYMBOL_ARG && sym_tab[instruction.arg.symbol].section == current_section) {
                return reg_dir[instruction.asm_instruction];
        } 

        return reg_ind[instruction.asm_instruction];
}

inline uint32_t combineInstructions(uint8_t opcode, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint32_t displacement) {
    uint32_t combined_instr = (displacement & (static_cast<uint32_t>(-1) >> 20))
                             | (static_cast<uint32_t>(opcode) << 24)
                             | (static_cast<uint32_t>(reg1) << 20)
                             | (static_cast<uint32_t>(reg2) << 16)
                             | (static_cast<uint32_t>(reg3) << 12);
    return combined_instr;
}

int32_t Assembler::initInstruction(InstrDesc instruction) {
        if(pass == 1) return 0;
        if(instruction.arg.arg_type == SYMBOL_ARG && sym_tab.count(instruction.arg.symbol) == 0) return 1; /* error */

        location_counter +=4;
        ParserArg instr_arg = instruction.arg;

        AsmInstruction asm_instr = instruction.asm_instruction;
        uint32_t proc_instr = 0;
        uint8_t opcode = __getOpcode(instruction);
        int32_t displacement = __displacement(opcode, instruction);
        switch(asm_instr){
               case IRET_ASM: {
                        proc_instr = combineInstructions(opcode, STATUS, R14, 0, displacement);
                        initLiteral(proc_instr, current_section);
                        initInstruction({POP_ASM, 15, 14, 0, {"", 8, LITERAL_ARG}});
                        break;
               }
               case LD_MEM_DIR_ASM: {
                        proc_instr = combineInstructions(opcode, instruction.reg1, R15, instruction.reg3, displacement);
                        initLiteral(proc_instr, current_section);
                        if(instr_arg.arg_type == SYMBOL_ARG && sym_tab[instr_arg.symbol].section == current_section) {
                                initInstruction({ADD_ASM, 0, 0, 0, {"", 0, REG_ARG}});
                        } else {
                                initInstruction({LD_REG_IND_ASM, instruction.reg1, instruction.reg1, 0, {"", 0, REG_ARG}});
                        }
                        break;
               }
               case LD_REG_IND_DISP_ASM: 
               case ST_REG_IND_DISP_ASM: {
                        int32_t arg_value = instr_arg.arg_type == LITERAL_ARG? instr_arg.literal : sym_tab[instr_arg.symbol].value;
                        if(instr_arg.arg_type == SYMBOL_ARG && sym_tab[instr_arg.symbol].type != ABS_TYPE) {
                                return 1;
                        }

                        if(!VALID_VALUE(arg_value)) {
                                return 1;
                        }
               }
              default: {
                        proc_instr = combineInstructions(opcode, instruction.reg1, instruction.reg2, instruction.reg3, displacement);
                        initLiteral(proc_instr, current_section);
                        break;
               }
        }

        return 0;
}

int32_t Assembler::__displacement(uint8_t opcode, InstrDesc instruction) {
        int32_t displacement;
        ParserArg arg = instruction.arg;
        std::unordered_map<AsmInstruction, int32_t> known_displacements {
                {PUSH_ASM, instruction.arg.literal},
                {POP_ASM, instruction.arg.literal},
                {RET_ASM, instruction.arg.literal},
                {ST_REG_IND_DISP_ASM, instruction.arg.arg_type == LITERAL_ARG? instruction.arg.literal : sym_tab[instruction.arg.symbol].value},
                {LD_REG_IND_DISP_ASM, instruction.arg.arg_type == LITERAL_ARG? instruction.arg.literal : sym_tab[instruction.arg.symbol].value},
                {IRET_ASM, 8}
        };

        if(known_displacements.count(instruction.asm_instruction)) {
                return known_displacements[instruction.asm_instruction];
        }

        if(arg.arg_type == REG_ARG) return 0;

        if(arg.arg_type == SYMBOL_ARG && sym_tab[arg.symbol].section == current_section) {
                displacement = sym_tab[arg.symbol].value - location_counter; 
                return displacement;
        }

        uint32_t pool_value;
        if(arg.arg_type == SYMBOL_ARG && !symbol_pool.count(arg.symbol)) {
                int32_t index = literal_pool.size();
                symbol_pool.insert(std::pair(arg.symbol, index));
                SymEntry sym = sym_tab[arg.symbol];
                RelocEntry rela_entry = {
                        offset : sec_tab[current_section].size + static_cast<uint32_t>(literal_pool.size()) * 4,
                        symbol_index : sym.bind == LOCAL_BIND? sym_tab[sym.section].symbol_index : sym.symbol_index,
                        addend : sym.bind == LOCAL_BIND? (int32_t)sym.value : 0, 
                        symbol_name :  sym.bind == LOCAL_BIND? sym_tab[sym.section].symbol_name: sym.symbol_name
                };

                displacement = sec_tab[current_section].size + literal_pool.size() * 4 - location_counter;
                __reloc(rela_entry, current_section);
                literal_pool.push_back(0);
        }

        if(arg.arg_type == SYMBOL_ARG && symbol_pool.count(arg.symbol)) {
                int32_t index = symbol_pool[arg.symbol];
                displacement = sec_tab[current_section].size + index * 4 - location_counter; 
        }

        if(arg.arg_type == LITERAL_ARG) {
                auto it = std::find(literal_pool.begin(), literal_pool.end(), arg.literal);
                if(arg.literal != 0 && it != literal_pool.end()) {
                        int32_t index = it - literal_pool.begin();
                        displacement = sec_tab[current_section].size + index * 4 - location_counter;
                } else {
                        displacement = sec_tab[current_section].size + literal_pool.size() * 4 - location_counter; 
                        literal_pool.push_back(arg.literal);
                }
        }

        return displacement;
}

int32_t Assembler::firstPass(){
        yyin = fopen(input_file.c_str(), "r"); 
        if(!yyin) {
                std::cout << "error opening file!" << std::endl; 
                return 1;
        }
        pass = 1;
        if(yyparse()){
                std::cout << "parsing error!" << std::endl;
                return 1;
        }

        return 0;  
}

int32_t Assembler::secondPass(){
        rewind(yyin);
        pass = 2; 
        if(yyparse()){
                std::cout << "parsing error!" << std::endl;
                return 1;
        }
        __writeStrtab();
        __writeSymtab();

        Fish32 fish(output_file);
        fish.writeLinkable(sym_tab, sec_tab, mem_init);
        return 0;
}

int32_t Assembler::addSymbol(std::string sym_name ,SymBind bind, bool is_definition, SymType type) {
        if(!sym_tab.count(sym_name)) {
                SymEntry entry = {
                        value : is_definition? location_counter : 0,
                        size : 0,
                        type : type,
                        bind : bind,
                        name_entry : 0,
                        symbol_index : symbol_counter++,
                        section_entry : is_definition? sec_tab[current_section].section_index : 0, 
                        symbol_name : sym_name,
                        section : is_definition? current_section : ""
                };

                sym_tab.insert(std::pair(sym_name, entry));
                return 0;
        } 
        
        /* the symbol exists */
        SymEntry& entry = sym_tab.find(sym_name)->second;
        
        if(entry.bind != EXTERN_BIND && bind == GLOBAL_BIND) {
                entry.bind = GLOBAL_BIND;
                return 0;
        }

        if(entry.section == "" && entry.bind != EXTERN_BIND && is_definition) {
                entry.section = current_section;
                entry.section_entry = sec_tab[current_section].section_index;
                entry.type = type; /* in case we are adding a section? */
                entry.value = location_counter;
                return 0;
        }

        return 1;
}

int32_t Assembler::addSection(std::string sec_name) {
        if(sec_tab.count(sec_name) && pass == 1) {
                return 1;
        }

        if(pass == 1 && current_section != "") {
                sec_tab.at(current_section).size = sym_tab.at(current_section).size = location_counter; /* update section size */
        }

        if (pass == 2 && current_section != "") {
                __init_mem(reinterpret_cast<const char*>(&literal_pool[0]), literal_pool.size() * 4, current_section);
                sec_tab[current_section].size += literal_pool.size() * 4;
                literal_pool.clear();
                symbol_pool.clear();
        }

        location_counter = 0;
        current_section = sec_name;
        __section(sec_name, PROGBITS_SECTION); 

        return 0;        
}

int32_t Assembler::__section(std::string section_name, SecType type) {
       if(!sec_tab.count(section_name)) {
                SecEntry entry = {
                        size : 0,
                        type : type,
                        offset : 0,
                        section_index : section_counter++,
                        name_entry : 0,
                        link : 0,
                        info : 0,
                        section_name : section_name,
                };

                sec_tab.insert(std::pair(section_name, entry));

                if((type == PROGBITS_SECTION || type == NULL_SECTION) && addSymbol(section_name, GLOBAL_BIND, true, SECTION_TYPE)) {
                        return 1;
                }
        }

        return 1;
}

int32_t Assembler::checkSymbol(std::string sym_name, SymBind bind, SymType type){
        if(!sym_tab.count(sym_name)) {
                return 1;
        }
        SymEntry sym = sym_tab[sym_name];

        if(bind == EXTERN_BIND && sym.section != "") {
                return 1;
        }

        if(bind == GLOBAL_BIND && sym.type != ABS_TYPE && sym.section == "") {
                return 1;
        }

        return 0;
}

int32_t Assembler::initLiteral(uint32_t value, std::string section) {
        __init_mem(reinterpret_cast<const char*>(&value), sizeof(value), section);
        return 0;
}

int32_t Assembler::initSymbol(std::string symbol_name, std::string section) {
        if(!sym_tab.count(symbol_name)) {
                return 1;
        }
       /* if symbol bind is ABS, reloc is not needed! */ 
        auto& sym = sym_tab[symbol_name];
        
        
        auto reloc_entry = RelocEntry {
                offset : location_counter,
                symbol_index : sym.bind == LOCAL_BIND? sym_tab[sym.section].symbol_index : sym.symbol_index,
                addend : sym.bind == LOCAL_BIND? (int32_t)sym.value : 0, 
                symbol_name :  sym.bind == LOCAL_BIND? sym_tab[sym.section].symbol_name: sym.symbol_name
        };

        initLiteral(0, section);
        if(__reloc(reloc_entry, section)) {
                return 1;
        }

        return 0;
}

int32_t Assembler::initSkip(uint32_t cnt) {
        if(pass == 2) {
                std::vector<uint8_t> value(cnt, 0);
                __init_mem(reinterpret_cast<const char*>(&value[0]), cnt, current_section);
        }

        location_counter += cnt;
        return 0;
}

int32_t Assembler::__init_mem(const char* buf, int32_t size, std::string section_name) {
        if(buf == nullptr) {
                return 1;
        }

        if(!mem_init.count(section_name)) {
                auto ss = std::make_shared<std::stringstream>();
                mem_init[section_name] = ss; 
        }        
        
        auto& ss = mem_init[section_name];
        ss->write(buf, size);

        return 0;
}

int32_t Assembler::__reloc(RelocEntry reloc_entry, std::string section) {
        std::string rela_name = ".rela." + section;

        if(!sec_tab.count(rela_name)) {
                __section(rela_name, RELA_SECTION);
        }

        auto& rela_desc = sec_tab[rela_name];
        
        if(rela_desc.type != RELA_SECTION) {
                return 1;
        }
        rela_desc.link = sec_tab[section].section_index;
        __init_mem(reinterpret_cast<const char*>(&reloc_entry), sizeof(RelocEntry) - sizeof(std::string), rela_name);
        rela_desc.size += sizeof(RelocEntry) - sizeof(std::string);

        return 0;
}

int32_t Assembler::initWords(std::deque<ParserArg>& word_args) {
        if(pass == 1) {
                location_counter += word_args.size() * 4;
                return 0;
        }

        for(auto& arg : word_args) {
                if(arg.arg_type == LITERAL_ARG && initLiteral(arg.literal, current_section) ||
                (arg.arg_type == SYMBOL_ARG && initSymbol(arg.symbol, current_section))) {
                        return 1;
                }
                location_counter += 4;
        }

        return 0;
}

void Assembler::endPass() {
        if(current_section != "" && pass == 1) {
                sec_tab.at(current_section).size = sym_tab.at(current_section).size = location_counter; /* update section size */
        }

        if(pass == 2 && current_section != "") {
                __init_mem(reinterpret_cast<const char*>(&literal_pool[0]), literal_pool.size() * 4, current_section);
                sec_tab[current_section].size += literal_pool.size() * 4;
                literal_pool.clear();
                symbol_pool.clear();
        }

        location_counter = 0;
        current_section = "";
}

void Assembler::__writeStrtab() {
        uint32_t entry = 0;
        for(auto& s : sym_tab) {
                __init_mem(s.first.c_str(), s.first.size() + 1, strtab);
                s.second.name_entry = entry++;
                sec_tab[strtab].size += s.first.size() + 1;
        }

        entry = 0;
        for(auto& s : sec_tab) {
                __init_mem(s.first.c_str(), s.first.size() + 1, shstrtab);
                s.second.name_entry = entry++;
                sec_tab[shstrtab].size += s.first.size() + 1;
        }
}
/* vrv nepotrebno!! i vr cemo izbaciti */
void Assembler::__writeSymtab() {
        SecEntry& symtab_section = sec_tab[symtab];
        symtab_section.link = sec_tab[strtab].section_index;
        uint32_t entry = 0;
        for(auto& s : sym_tab) {
                s.second.name_entry = entry++;
                __init_mem(reinterpret_cast<const char*>(&s.second), sizeof(s.second) - 2 * sizeof(std::string), symtab); 
                symtab_section.size += sizeof(s.second) - 2 * (sizeof(std::string));
        }
}

int32_t Assembler::initAscii(std::string asciistr) {
        if(current_section == "") return 1;
        asciistr.erase(std::remove(asciistr.begin(), asciistr.end(), '\"'), asciistr.end());
        std::string outstr = "";
        for(int32_t i = 0; i < asciistr.size(); i++) {
                if(asciistr[i] == '\\' && i != asciistr.size() - 1) {
                        switch (asciistr[i+1]) {
                                case 'n':
                                        outstr += '\n';
                                        i++;
                                        break;
                                case 't':
                                        outstr += '\t';
                                        i++;
                                        break;
                                case 'b':
                                        outstr += '\b';
                                        i++;
                                        break;
                                case 'r':
                                        outstr += '\r';
                                        i++;
                                        break;
                                case '0':
                                        outstr += '\0';
                                        i++;
                                        break;
                                case '\\':
                                        i++;
                                default:
                                        outstr += '\\';
                                        break;
                        } 
                } else {
                        outstr += asciistr[i];
                }
        }

        if(pass == 2) __init_mem(outstr.c_str(), outstr.size(), current_section);
        location_counter += outstr.size();

        return 0;
}