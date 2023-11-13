#include "../../inc/common/Fish32.hpp"
#include <deque>
#include <vector>
#include <algorithm>
#include <iostream>


int32_t Fish32::writeExecutable(const SecTab& section_table, const MemTab& mem_init) {
        std::ofstream write_file(file_name, std::ios::binary | std::ios::out);

        if(!write_file.is_open()) {
                return 1;
        }
        /* maybe put in a function? */
        write_file.write(reinterpret_cast<const char*>(&magic_number), sizeof(uint8_t));
        write_file.write(magic_name.c_str(), magic_name.size() + 1);
        file_header = {static_cast<int32_t>(section_table.size()), 
                       0,
                       0,
                       EXECUTABLE_FILE};
        write_file.write(reinterpret_cast<const char*>(&file_header), sizeof(FishHeader));

        if(__writeExecutable(write_file, section_table, mem_init)) {
                return 1;
        }

        return 0;
}

int32_t Fish32::writeLinkable(const SymTab& symbol_table, const SecTab& section_table, const MemTab& mem_init) {
        std::ofstream write_file(file_name, std::ios::binary | std::ios::out);

        if(!write_file.is_open()) {
                return 1;
        }

        write_file.write(reinterpret_cast<const char*>(&magic_number), sizeof(uint8_t));
        write_file.write(magic_name.c_str(), magic_name.size() + 1);
        file_header = {static_cast<int32_t>(section_table.size()), 
                       static_cast<int32_t>(symbol_table.size()),
                       section_table.at("shstrtab").section_index,
                       LINKABLE_FILE};
        write_file.write(reinterpret_cast<const char*>(&file_header), sizeof(FishHeader));
        /* write to file */ 
        if(__writeLinkable(write_file, symbol_table, section_table, mem_init)) {
                return 1;
        }

        return 0;
}

int32_t Fish32::readFile(FishHeader& fheader, SymTab& symbol_table, SecTab& section_table, MemTab& mem_init, RelaTab& rela_tab, SegTab& seg_tab) {
        std::ifstream read_file(file_name, std::ios::binary | std::ios::in);

        if(!read_file.is_open()) {
                return 1;
        }

        uint8_t control_number; 
        std::string control_string;
        read_file.read(reinterpret_cast<char*>(&control_number), sizeof(uint8_t));
        std::getline(read_file, control_string, '\0');
        if(control_number != magic_number || control_string != magic_name) {
                std::cout << "Unsupported file format!" << std::endl;
                return 1;
        }

        read_file.read(reinterpret_cast<char*>(&file_header), sizeof(FishHeader));
        fheader = file_header;
        switch(file_header.file_type){
                case LINKABLE_FILE:
                        __readLinkable(read_file, symbol_table, section_table, mem_init, rela_tab);
                        break;
                case EXECUTABLE_FILE:
                        __readExecutable(read_file, seg_tab);
                        break;
        }

        return 0;
}

int32_t Fish32::readLinkable(SymTab& symbol_table, SecTab& section_table, MemTab& mem_init, RelaTab& rela_table) {
        std::ifstream read_file(file_name, std::ios::binary | std::ios::in);

        if(!read_file.is_open()) {
                return 1;
        }

        uint8_t control_number; 
        std::string control_string;
        read_file.read(reinterpret_cast<char*>(&control_number), sizeof(uint8_t));
        std::getline(read_file, control_string, '\0');
        if(control_number != magic_number || control_string != magic_name) {
                std::cout << "Unsupported file format!" << std::endl;
                return 1;
        }

        read_file.read(reinterpret_cast<char*>(&file_header), sizeof(FishHeader));
        if(file_header.file_type != LINKABLE_FILE) {
                std::cout << "Wrong file type!" << std::endl;
                return 1;
        }

        if(__readLinkable(read_file, symbol_table, section_table, mem_init, rela_table)) {
                return 1;
        }

        return 0;
}

int32_t Fish32::readExecutable(SegTab& mem_init) {
        std::ifstream read_file(file_name, std::ios::binary | std::ios::in);

        if(!read_file.is_open()) {
                return 1;
        }

        uint8_t control_number; 
        std::string control_string;
        read_file.read(reinterpret_cast<char*>(&control_number), sizeof(uint8_t));
        std::getline(read_file, control_string, '\0');
        if(control_number != magic_number || control_string != magic_name) {
                std::cout << "Unsupported file format!" << std::endl;
                return 1;
        }

        read_file.read(reinterpret_cast<char*>(&file_header), sizeof(FishHeader));
        if(file_header.file_type != EXECUTABLE_FILE) {
                std::cout << "Wrong file type!" << std::endl;
                return 1;
        }

        if(__readExecutable(read_file, mem_init)) {
                std::cout << "Failed reading file" << std::endl;
                return 1;
        }

        return 0;
}

int32_t Fish32::__writeExecutable(std::ofstream& write_file, const SecTab& section_table, const MemTab& mem_init) {
        std::vector<std::string> write_order;
        /* number of entries == number of sections!! */
        int64_t sections_start = static_cast<int64_t>(write_file.tellp()) + (section_table.size() * sizeof(PHeaderEntry));
        for(auto s : section_table) {
                SecEntry& section = s.second;
                PHeaderEntry ph_entry;
                ph_entry.addr = section.offset;
                ph_entry.offset = sections_start;
                ph_entry.entry_size = section.size;
                write_file.write(reinterpret_cast<const char*>(&ph_entry), sizeof(PHeaderEntry));
                write_order.push_back(section.section_name);
                sections_start += section.size;
        }

        for(auto next_name : write_order) {
                if(mem_init.count(next_name)) {
                        auto& ss = mem_init.at(next_name);
                        int64_t ss_size = section_table.at(next_name).size;
                        std::vector<uint8_t> buff(ss_size);
                        ss->read(reinterpret_cast<char*>(&buff[0]), ss_size);
                        write_file.write(reinterpret_cast<const char*>(&buff[0]), ss_size);
                }
        }
        return 0;
}

int32_t Fish32::__writeLinkable(std::ofstream& write_file, const SymTab& symbol_table, const SecTab& section_table, const MemTab& mem_init) {
        /* section headers immediately after the fish header */
        std::deque<std::string> write_order;
        int64_t sections_start = static_cast<int64_t>(write_file.tellp()) + (section_table.size() * (sizeof(SecEntry) - sizeof(std::string)));
        for(auto s: section_table) {
                SecEntry& section = s.second;
                section.offset = sections_start;
                write_file.write(reinterpret_cast<const char*>(&section), sizeof(SecEntry) - sizeof(std::string));
                write_order.push_back(section.section_name);
                sections_start += section.size;
        }

        for(auto next_name : write_order) {
                if(mem_init.count(next_name)) {
                        auto& ss = mem_init.at(next_name);
                        int64_t ss_size = ss->tellp();
                        std::vector<uint8_t> buff(ss_size);
                        ss->read(reinterpret_cast<char*>(&buff[0]), ss_size);
                        write_file.write(reinterpret_cast<const char*>(&buff[0]), ss_size);
                }
        }

        return 0;
}

int32_t Fish32::__readExecutable(std::ifstream& read_file, SegTab& mem_init) {
        std::vector<PHeaderEntry> pheaders;
        for(int32_t i = 0; i < file_header.sh_entries; i++) {
                PHeaderEntry ph_entry;
                read_file.read(reinterpret_cast<char*>(&ph_entry), sizeof(PHeaderEntry));
                pheaders.push_back(ph_entry);
        }
        
        for(auto& entry : pheaders) {
                read_file.seekg(entry.offset);
                auto ss = std::make_shared<std::stringstream>();
                std::vector<uint8_t> buff(entry.entry_size);
                read_file.read(reinterpret_cast<char*>(&buff[0]), entry.entry_size);
                ss->write(reinterpret_cast<const char*>(&buff[0]), entry.entry_size);
                mem_init[entry.addr] = ss;
        }

        return 0;
}

int32_t Fish32::__readLinkable(std::ifstream& read_file, SymTab& symbol_table, SecTab& section_table, MemTab& mem_init, RelaTab& rela_tab) {
        SecEntry sym_tab;
        SecEntry shstr_tab;
        SecEntry str_tab;
        std::vector<SecEntry> sec_entries;
        std::vector<SymEntry> sym_entries;
        std::vector<SecEntry> rela_sections;
        for(int i = 0; i < file_header.sh_entries; i++){
                SecEntry section;
                read_file.read(reinterpret_cast<char*>(&section), sizeof(SecEntry) - sizeof(std::string));
                
                if(section.type == STRTAB_SECTION) {
                     if(section.section_index == file_header.shstr_index) shstr_tab = section;
                     else str_tab = section;   
                }

                if(section.type == SYMTAB_SECTION) {
                        sym_tab = section;
                }

                sec_entries.push_back(section);
        }
        /* read section names */
        std::vector<std::string> section_names;
        read_file.seekg(shstr_tab.offset);
        for(int i = 0; i < file_header.sh_entries; i++) {
                std::string temp_string;
                std::getline(read_file, temp_string, '\0');
                section_names.push_back(temp_string);
        }

        for(auto& s : sec_entries) {
                s.section_name = section_names[s.name_entry];
                section_table.insert(std::pair(s.section_name, s));
        }
        /* read symbols!!! */
        read_file.seekg(sym_tab.offset);
        for(int i = 0; i< file_header.sy_entries; i++) {
                SymEntry symbol;
                read_file.read(reinterpret_cast<char*>(&symbol), sizeof(symbol) - 2 * sizeof(std::string));
                sym_entries.push_back(symbol);
        }

        std::vector<std::string> symbol_names;
        read_file.seekg(str_tab.offset);
        for (int i = 0; i < file_header.sy_entries; i++) {
                std::string temp_string;
                std::getline(read_file, temp_string, '\0');
                symbol_names.push_back(temp_string);
        }

        for(auto& sym : sym_entries) {
                sym.symbol_name = symbol_names[sym.name_entry];
                sym.section = std::find_if(sec_entries.begin(), 
                                           sec_entries.end(), 
                                           [&cs = sym.section_entry](SecEntry& ns) -> bool {return cs == ns.section_index; })->section_name;
                symbol_table.insert(std::pair(sym.symbol_name, sym));
        }

        /* read other sections into mem_init, linker will use this. somehow */
        for(auto& sec : sec_entries) {
                if(sec.size) {
                        read_file.seekg(sec.offset);
                        std::vector<uint8_t> buff(sec.size);
                        read_file.read(reinterpret_cast<char*>(&buff[0]), sec.size);
                        auto ss = std::make_shared<std::stringstream>();
                        mem_init[sec.section_name] = ss;
                        ss->write(reinterpret_cast<const char*>(&buff[0]), sec.size); 
                        if(sec.type == RELA_SECTION) {
                                rela_sections.push_back(sec);
                        }
                }
        }
        /* read rela sections, separately because i said so */
        for(auto& rela : rela_sections) {
                std::vector<RelocEntry> reloc_entries;
                RelocEntry temp_entry;
                int32_t entry_number = mem_init[rela.section_name]->tellp() / (sizeof(RelocEntry) - sizeof(std::string));
                for(int32_t i = 0; i < entry_number; i++) {
                        mem_init[rela.section_name]->read(reinterpret_cast<char*>(&temp_entry), sizeof(RelocEntry) - sizeof(std::string)); 
                        temp_entry.symbol_name = std::find_if(sym_entries.begin(), 
                                           sym_entries.end(), /* check for possible errors! */
                                           [&cs = temp_entry.symbol_index](SymEntry& ns) -> bool {return cs == ns.symbol_index; })->symbol_name;
                       reloc_entries.push_back(temp_entry);
                }
                mem_init[rela.section_name]->seekg(std::ios::beg);
                rela_tab.insert(std::pair(rela.section_name, reloc_entries));
        }
        return 0;
}
