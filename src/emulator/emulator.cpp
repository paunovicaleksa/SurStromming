#include "../../inc/emulator/emulator.hpp"
#include <iomanip>

inline uint8_t getOpcode(uint32_t instruction) {
        uint8_t opcode = static_cast<uint8_t>(instruction >> 24);
        return opcode;
}

inline int32_t getRegs(uint32_t instruction, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3) {
        *reg1 = static_cast<uint8_t>((instruction >> 20) & 0x0f);
        *reg2 = static_cast<uint8_t>((instruction >> 16) & 0x0f);
        *reg3 = static_cast<uint8_t>((instruction >> 12) & 0x0f);
        return 0;
}

inline int16_t getDisplacement(uint32_t instruction) {
        int16_t displacement = (instruction & 0xfff) | ((instruction & 0x800)? 0xf000 : 0x0000);

        return displacement;
}

int32_t Emulator::emulate() {
        if(loadMemory()) {
                return 1;
        }

        bool halt = false;
        terminal = new Terminal();
        while(!halt) {
                uint32_t temp;
                uint32_t instruction = fetchInstruction();
                uint8_t opcode = getOpcode(instruction);
                uint8_t reg1, reg2, reg3;
                getRegs(instruction, &reg1, &reg2, &reg3);
                int16_t displacement = getDisplacement(instruction);

                switch(opcode) {
                        case HALT_PROC:
                                halt = true;
                                break;
                        case INT_PROC:
                                CPU.GPR[SP] -= 4;
                                writeMemory32(CPU.GPR[SP], CPU.CSR[STATUS]);
                                CPU.GPR[SP] -= 4;
                                writeMemory32(CPU.GPR[SP], CPU.GPR[PC]);
                                CPU.CSR[CAUSE] = 4;
                                CPU.CSR[STATUS] = CPU.CSR[STATUS] & (~0x1u);
                                CPU.GPR[PC] = CPU.CSR[HANDLER];
                                break;
                        case CALL_PROC:
                                CPU.GPR[SP] -= 4;
                                writeMemory32(CPU.GPR[SP], CPU.GPR[PC]);
                                CPU.GPR[PC] = CPU.GPR[reg1] + CPU.GPR[reg2] + displacement;
                                break;
                        case LONG_CALL_PROC:
                                CPU.GPR[SP] -= 4;
                                writeMemory32(CPU.GPR[SP], CPU.GPR[PC]);
                                CPU.GPR[PC] = readMemory32(CPU.GPR[reg1] + CPU.GPR[reg2] + displacement);
                                break;
                        case JUMP_PROC:
                                CPU.GPR[PC] = CPU.GPR[reg1] + displacement;
                                break;
                        case LONG_JUMP_PROC:
                                CPU.GPR[PC] = readMemory32(CPU.GPR[reg1] + displacement);
                                break;
                        case BEQ_PROC:
                                if(CPU.GPR[reg2] == CPU.GPR[reg3]) 
                                        CPU.GPR[PC] = CPU.GPR[reg1] + displacement;
                                break;
                        case BNE_PROC:
                                if(CPU.GPR[reg2] != CPU.GPR[reg3]) 
                                        CPU.GPR[PC] = CPU.GPR[reg1] + displacement;
                                break;
                        case BGT_PROC:
                                if(static_cast<int32_t>(CPU.GPR[reg2]) > static_cast<int32_t>(CPU.GPR[reg3]))
                                        CPU.GPR[PC] = CPU.GPR[reg1] + displacement;
                                break;
                        case LONG_BEQ_PROC:
                                if(CPU.GPR[reg2] == CPU.GPR[reg3]) 
                                        CPU.GPR[PC] = readMemory32(CPU.GPR[reg1] + displacement);
                                break;
                        case LONG_BNE_PROC:
                                if(CPU.GPR[reg2] != CPU.GPR[reg3]) 
                                        CPU.GPR[PC] = readMemory32(CPU.GPR[reg1] + displacement);
                                break;
                        case LONG_BGT_PROC:
                                if(static_cast<int32_t>(CPU.GPR[reg2]) > static_cast<int32_t>(CPU.GPR[reg3]))
                                        CPU.GPR[PC] = readMemory32(CPU.GPR[reg1] + displacement);
                                break;
                        case XCHG_PROC:
                                temp = CPU.GPR[reg2];
                                CPU.GPR[reg2] = CPU.GPR[reg3];
                                CPU.GPR[reg3] = temp;
                                break;
                        case ADD_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] + CPU.GPR[reg3];
                                break;                        
                        case SUB_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] - CPU.GPR[reg3];
                                break;
                        case MUL_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] * CPU.GPR[reg3];
                                break;
                        case DIV_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] / CPU.GPR[reg3];
                                break;
                        case NOT_PROC:
                                CPU.GPR[reg1] = ~CPU.GPR[reg2];
                                break;
                        case AND_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] & CPU.GPR[reg3];
                                break;
                        case OR_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] | CPU.GPR[reg3];
                                break;
                        case XOR_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] ^ CPU.GPR[reg3];
                                break;
                        case SHL_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] << CPU.GPR[reg3];
                                break;
                        case SHR_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] >> CPU.GPR[reg3];
                                break;
                        case ST_REG_IND_PROC:
                                writeMemory32(CPU.GPR[reg1] + CPU.GPR[reg2] + displacement, CPU.GPR[reg3]);
                                break;
                        case ST_MEM_IND_PROC:
                                temp = readMemory32(CPU.GPR[reg1] + CPU.GPR[reg2] + displacement);
                                writeMemory32(temp, CPU.GPR[reg3]);
                                break;
                        case PUSH_PROC:
                                if(reg1 != 0) CPU.GPR[reg1] += displacement;
                                writeMemory32(CPU.GPR[reg1], CPU.GPR[reg3]);
                                break;
                        case CSRRD_PROC:
                                CPU.GPR[reg1] = CPU.CSR[reg2];
                                break;
                        case LD_REG_DIR_PROC:
                                CPU.GPR[reg1] = CPU.GPR[reg2] + displacement;
                                break;
                        case LD_REG_IND_PROC:
                                CPU.GPR[reg1] = readMemory32(CPU.GPR[reg2] + CPU.GPR[reg3] + displacement);
                                break;
                        case POP_PROC:
                                CPU.GPR[reg1] = readMemory32(CPU.GPR[reg2]);
                                CPU.GPR[reg2] += displacement;
                                break;
                        case CSRWR_PROC:
                                CPU.CSR[reg1] = CPU.GPR[reg2];
                                break;
                        case CSRWR_CSR_PROC:
                                CPU.CSR[reg1] = CPU.CSR[reg2] | displacement;
                                break;
                        case CSRRD_MEM_PROC:
                                CPU.CSR[reg1] = readMemory32(CPU.GPR[reg2] + CPU.GPR[reg3] + displacement);
                                break;
                        case POP_CSR_PROC:
                                CPU.CSR[reg1] = readMemory32(CPU.GPR[reg2]);
                                CPU.GPR[reg2] += displacement;
                                break; 
                        default:
                                /* invalid instruction interrupt */
                                CPU.GPR[SP] -= 4;
                                writeMemory32(CPU.GPR[SP], CPU.CSR[STATUS]);
                                CPU.GPR[SP] -= 4;
                                writeMemory32(CPU.GPR[SP], CPU.GPR[PC]);
                                CPU.CSR[STATUS] = CPU.CSR[STATUS] & (~0x1u);
                                CPU.CSR[CAUSE] = 1;
                                CPU.GPR[PC] = CPU.CSR[HANDLER];
                                break;
                }
                CPU.GPR[R0] = 0;
                if(terminalInterrupt() && !(CPU.CSR[STATUS] & 0b110)) {
                        CPU.GPR[SP] -= 4;
                        writeMemory32(CPU.GPR[SP], CPU.CSR[STATUS]);
                        CPU.GPR[SP] -= 4;
                        writeMemory32(CPU.GPR[SP], CPU.GPR[PC]);
                        CPU.CSR[STATUS] = CPU.CSR[STATUS] & (~0x1u);
                        CPU.CSR[CAUSE] = 3;
                        CPU.GPR[PC] = CPU.CSR[HANDLER];
                }

                if(memory[TERM_OUT] != 0 ) {
                        terminal->putChar(memory[TERM_OUT]);
                        memory[TERM_OUT] = 0;
                }
        }
        delete terminal;
        std::cout << std::endl;
        writeProcessorState();
        return 0;
}

int32_t Emulator::loadMemory() {
        SegTab mem_init;
        if(file.readExecutable(mem_init)) {
                std::cout << "Reading file failed" << std::endl;; 
                return 1;
        }

        memory = reinterpret_cast<uint8_t*>(mmap(nullptr, 1l << 32, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0));
        if(memory == MAP_FAILED) {
                std::cout << "mmap failed" << std::endl;
                return 1;
        }

        for(auto& [address, segment] : mem_init) {
                segment->read(reinterpret_cast<char*>(&memory[address]), segment->tellp());
                segment->seekg(std::ios::beg);
        }

        return 0;
}

uint32_t Emulator::fetchInstruction() {
        uint32_t instruction;
        instruction = readMemory32(CPU.GPR[PC]); 
        CPU.GPR[PC] += 4;
        return instruction;
}

void Emulator::writeProcessorState() {
        std::cout << "-----------------------------------------------------------------" << std::endl;
        std::cout << "Emulated processor executed halt instruction" << std::endl;
        std::cout << "Emulated processor state:" << std::endl;

        for (int32_t i = 0; i < 16; ++i) {
                std::cout << "r" << std::dec << i << "=" << "0x" << std::setfill('0') << std::setw(8) << std::hex << CPU.GPR[i] << " ";
                if ((i + 1) % 4 == 0) {
                        std::cout << std::endl;
                }
        }
}


uint32_t Emulator::readMemory32(uint32_t address) {
        uint32_t big_endian;
        big_endian = static_cast<uint32_t>(memory[address]) | 
                      (static_cast<uint32_t>(memory[address + 1]) << 8) |
                      (static_cast<uint32_t>(memory[address + 2]) << 16) |
                      (static_cast<uint32_t>(memory[address + 3]) << 24);
        return big_endian;
}

void Emulator::writeMemory32(uint32_t address, uint32_t value) {
        uint32_t little_endian = ((value & 0xff) << 24) | ((value & 0xff00) << 8) | ((value & 0xff0000) >> 8) | ((value >> 24) & 0xff);
        memory[address] = static_cast<uint8_t>(value & 0xff);
        memory[address + 1] = static_cast<uint8_t>((value >> 8) & 0xff);
        memory[address + 2] = static_cast<uint8_t>((value >> 16) & 0xff);
        memory[address + 3] = static_cast<uint8_t>((value >> 24) & 0xff);
}

int32_t Emulator::terminalInterrupt() {
        char c;
        if(terminal->getChar(&c)) {
                memory[TERM_IN] = c;
                return 1;
        } 
        return 0;
}