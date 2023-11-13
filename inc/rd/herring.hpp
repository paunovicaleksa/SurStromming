#include "../common/Fish32.hpp"
#include <vector>

#ifndef HERRING_HPP
#define HERRING_HPP

class Herring {
public:
        Herring(std::string filename): fish(filename) {
                fish.readFile(fheader, sym_tab, sec_tab, mem_init, relas_tab, seg_tab); 
        }

        void writeSymbols();
        void writeSections();
        void writeHeader();
        void writeRela();
        void writeMem();

        static void writeHelp();
protected:
        void __writeExecMem();
        void __writeLinkMem();
private:
        Fish32 fish;
        FishHeader fheader;
        SymTab sym_tab;
        SecTab sec_tab;
        MemTab mem_init;
        RelaTab relas_tab;
        SegTab seg_tab;
};

#endif