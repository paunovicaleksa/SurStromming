#include "../common/Fish32.hpp"
#include "../common/common_types.hpp"
#include <sys/mman.h>

#ifndef EMULATOR_HPP
#define EMULATOR_HPP

class Emulator {
public:
        Emulator(std::string file_name): file(file_name) { CPU.GPR[PC] = 0x40000000; }
        ~Emulator() {
                if(memory != MAP_FAILED) {
                        munmap(memory, 1l << 32);
                }
        }

        int32_t emulate();
protected:
        void writeProcessorState();
        int32_t loadMemory();
        uint32_t fetchInstruction();
        uint32_t readMemory32(uint32_t address);
        void writeMemory32(uint32_t address, uint32_t value);
private:
        Fish32 file;
        uint8_t* memory = reinterpret_cast<uint8_t*>(MAP_FAILED);
        struct cpu {
                uint32_t GPR[16] = {0};
                uint32_t CSR[3] = {0};
        } CPU;
};

#endif