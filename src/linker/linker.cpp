#include "../../inc/linker/linker.hpp"
#include <algorithm>
#include "../../inc/common/Fish32.hpp"


int32_t Linker::writeExecutable() {
        /* maybe put these in a struct, not sure how to avoid copying multiple times. */
        std::vector<FileArgs> file_args;
        for(auto input_name : file_names) {
                Fish32 f32(input_name);
                SymTab local_symtab;
                SecTab local_sectab;
                MemTab local_mem;
                RelaTab local_relas;

                f32.readLinkable(local_symtab, local_sectab, local_mem, local_relas);
                file_args.push_back({local_symtab, local_sectab, local_mem, local_relas});
        }
        /* find only if one!! */
        for(auto& [address, name] : section_placements) {
                /* sections are placed in ascending order by address, this much should be enough, do check though. */
                if(address < placement_address) {
                        std::cout << "Section name " << name << " could not be placed at " << address << std::endl;
                        return 1;
                }

                if(placeSection(file_args, name, address)){
                        return 1;
                }
        }
        /* normal sections here */
        for(auto &arg : file_args) {
                for (auto &place_section : arg.file_section_table) {
                        if(place_section.second.type != PROGBITS_SECTION || section_table.count(place_section.first) == 1) {
                                continue;
                        }

                        if(placeSection(file_args, place_section.first, placement_address)){
                                return 1;
                        }
                }
        }

        /* create symbol table with new values, using offset and section names i guess */
        for(auto &arg : file_args) {
                for(auto &symbol_entry : arg.file_symbol_table) {
                        SymEntry& current_symbol = symbol_entry.second;
                        if(current_symbol.bind != GLOBAL_BIND || current_symbol.section == "" || current_symbol.type == SECTION_TYPE) {
                                continue;
                        }
                        if(symbol_table.count(current_symbol.symbol_name)) {
                                std::cout << "Symbol name " << current_symbol.symbol_name << " could not be resolved unambiguously" << std::endl;
                                return 1;
                        }
                        current_symbol.value = section_table[current_symbol.section].offset + current_symbol.value;
                        current_symbol.symbol_index = symbol_table_index++;
                        symbol_table.insert(std::pair(current_symbol.symbol_name, current_symbol));
                }
        }
       /* for undefined symbols */ 
        for(auto& arg: file_args) {
                for(auto &symbol_entry : arg.file_symbol_table) {
                        SymEntry& current_symbol = symbol_entry.second;
                        if(current_symbol.bind == EXTERN_BIND && symbol_table.count(current_symbol.symbol_name) == 0) {
                                std::cout << "Symbol name " << current_symbol.symbol_name << " could not be resolved unambiguously" << std::endl;
                                return 1;
                        }
                }
        }
       /* relocations now? */ 
        for(auto& arg : file_args) {
                for(auto& section_relocations : arg.file_reloation_table) {
                        SecEntry& relocation_section = arg.file_section_table[section_relocations.first];
                        auto it = std::find_if(arg.file_section_table.begin(), 
                                               arg.file_section_table.end(),
                                               [&link = relocation_section.link](auto& pair) -> bool { return pair.second.section_index == link; });

                        if(it == arg.file_section_table.end()) {
                                return 1;
                        }
                        
                        SecEntry& target_section = it->second; 
                        auto target_memory = memory_table[target_section.section_name];
                        for(auto& relocation : section_relocations.second) {
                                uint32_t write_value = symbol_table[relocation.symbol_name].value + relocation.addend; 
                                target_memory->seekp(relocation.offset);
                                target_memory->write(reinterpret_cast<const char*>(&write_value), sizeof(uint32_t));
                        }
                }
        }

        Fish32 f32(output_file);
        if(f32.writeExecutable(section_table, memory_table)) {
                std::cout << "Linker failed writing to file" << std::endl;
                return 1;
        }

        return 0;
}
/* maybe send a ref to a vector with all the elements with that name?? */
int32_t Linker::__placeSection(SecEntry& section, MemTab& sec_mem, uint32_t address) {
        if(!memory_table.count(section.section_name)) {
                auto ss = std::make_shared<std::stringstream>();
                memory_table.insert(std::pair(section.section_name, ss));
        }

        if(sec_mem.count(section.section_name)) {
                std::vector<uint8_t> buff(section.size);
                sec_mem[section.section_name]->read(reinterpret_cast<char*>(&buff[0]), section.size);
                memory_table[section.section_name]->write(reinterpret_cast<const char*>(&buff[0]), section.size);
        }

        return 0;
}

int32_t Linker::__addSymbol(SecEntry& section) {
        SymEntry section_symbol {
                value : static_cast<uint32_t>(section.offset),
                size : section.size,
                type : SECTION_TYPE,
                bind : GLOBAL_BIND,
                symbol_index : symbol_table_index++,
                section_entry : section.section_index,
                symbol_name : section.section_name,
                section : section.section_name
        };

        symbol_table.insert(std::pair(section.section_name, section_symbol));
        return 0;
}

int32_t Linker::placeSection(std::vector<FileArgs>& file_args, std::string section_name, uint32_t address) {
        auto it = std::find_if(file_args.begin(), 
                               file_args.end(), 
                               [&sn = section_name](FileArgs& fa) -> bool { return fa.file_section_table.count(sn) == 1; });
        if(it == file_args.end()) {
                return 1;
        }

        /* place the section */
        SecEntry current_section {
                size : 0,
                type : PROGBITS_SECTION, 
                offset : address,
                section_index : section_table_index++,
                name_entry : 0, 
                link : 0,
                info : 0,
                section_name : section_name
        };

        uint32_t shift = 0;
        for(; it != file_args.end(); it++) {
                SecEntry& place_section = (*it).file_section_table[section_name]; 
                MemTab& current_memory = (*it).file_memory_table;
                SymTab& file_symbol_table = (*it).file_symbol_table;
                RelaTab& file_relocation_table = (*it).file_reloation_table;
                SecTab& file_section_table = (*it).file_section_table;

                if(place_section.type != PROGBITS_SECTION) {
                        continue;
                }

                if(__placeSection(place_section, current_memory, address)) {
                        std::cout << "Could not place section " << current_section.section_name << std::endl;
                        return 1;
                }

                for(auto& sym_entry : file_symbol_table) {
                        if(sym_entry.second.section == section_name) { 
                                sym_entry.second.value += shift; 
                        }
                }

                auto re_it = std::find_if(file_relocation_table.begin(), 
                                          file_relocation_table.end(), 
                                          [&ps = place_section, &st = file_section_table](auto& rela_tab) -> bool { 
                                                return st[rela_tab.first].link == ps.section_index; 
                                          });
                
                for(; re_it != file_relocation_table.end(); re_it++) {
                        for(auto& entry : (*re_it).second) {
                                entry.offset += shift;
                        }
                }

                /* add to section table now? */ 
                address += place_section.size;
                shift += place_section.size;
                current_section.size += place_section.size; 
        }

        placement_address = address;
        section_table.insert(std::pair(current_section.section_name, current_section));
        __addSymbol(current_section);

        return 0;
}

void Linker::writeHelp() {
        std::cout << "Usage: linker [OPTIONS] files" << std::endl;
        std::cout << "Options:\n";
        std::cout << "  -h, [-]-help                        Display this help message" << std::endl;;
        std::cout << "  -p, [-]-place=section@hex_address   Place the section @ address" << std::endl;
        std::cout << "  -x, [-]-hex                         Write an executable file" << std::endl;
        std::cout << "  -o, [-]-out [file]                  Output to file" << std::endl;
}