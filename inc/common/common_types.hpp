#include <unistd.h>
#include <string>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

enum SymType: uint8_t {
        SECTION_TYPE,
        UNDEFINED_TYPE,
        ABS_TYPE
};

enum SymBind: uint8_t {
        LOCAL_BIND,
        GLOBAL_BIND,
        EXTERN_BIND
};

enum SecType: uint8_t {
        NULL_SECTION,
        PROGBITS_SECTION,
        RELA_SECTION,
        SYMTAB_SECTION,
        STRTAB_SECTION
};

typedef struct {
        uint32_t value;
        uint32_t size;//do i need this?
        SymType type;
        SymBind bind;
        uint32_t name_entry;
        uint32_t symbol_index; 
        uint32_t section_entry;
        std::string symbol_name; 
        std::string section; 
} SymEntry;

typedef struct {
        uint32_t size;
        SecType type;
        int64_t offset;
        /* link and info useful for rela sections and symtab */
        uint32_t section_index;
        uint32_t name_entry;
        uint32_t link;
        uint32_t info;
        std::string section_name;
} SecEntry;

typedef struct {
        uint32_t offset;
        uint32_t symbol_index;
        int32_t addend;
        std::string symbol_name;
} RelocEntry;

enum Registers: uint8_t {
        R0 = 0x0,
        R1 = 0x1,
        R2 = 0x2,
        R3 = 0x3,
        R4 = 0x4,
        R5 = 0x5,
        R6 = 0x6,
        R7 = 0x7,
        R8 = 0x8,
        R9 = 0x9,
        R10 = 0xa,
        R11 = 0xb,
        R12 = 0xc,
        R13 = 0xd,
        R14 = 0xe,
        R15 = 0xf,
        SP = R14,
        PC = R15,
        STATUS = 0x0,
        HANDLER = 0x1,
        CAUSE = 0x2
};

enum Opcodes: uint8_t {
        HALT_PROC = 0x00,
        INT_PROC = 0x10,
        CALL_PROC = 0x20,
        LONG_CALL_PROC = 0x21,
        JUMP_PROC = 0x30,
        LONG_JUMP_PROC = 0x38,
        BEQ_PROC = 0x31,
        BNE_PROC = 0x32,
        BGT_PROC = 0x33,
        LONG_BEQ_PROC = 0x39,
        LONG_BNE_PROC = 0x3a,
        LONG_BGT_PROC = 0x3b,
        XCHG_PROC = 0x40,
        ADD_PROC = 0x50,
        SUB_PROC = 0x51,
        MUL_PROC = 0x52,
        DIV_PROC = 0x53,
        NOT_PROC = 0x60,
        AND_PROC = 0x61,
        OR_PROC = 0x62,
        XOR_PROC = 0x63,
        SHL_PROC = 0x70,
        SHR_PROC = 0x71,
        ST_REG_IND_PROC = 0x80,
        ST_MEM_IND_PROC = 0x82,
        PUSH_PROC = 0x81,
        CSRRD_PROC = 0x90,
        LD_REG_DIR_PROC = 0x91,
        LD_REG_IND_PROC = 0x92,
        POP_PROC = 0x93,
        CSRWR_PROC = 0x94,
        CSRWR_CSR_PROC = 0x95,
        CSRRD_MEM_PROC = 0x96,
        POP_CSR_PROC = 0x97
};

enum FishType: uint8_t {
        EXECUTABLE_FILE,
        LINKABLE_FILE
};

typedef struct {
        int32_t sh_entries;
        int32_t sy_entries;
        uint32_t shstr_index;
        FishType file_type;
} FishHeader;

typedef struct {
        int64_t offset;
        uint32_t addr;
        uint32_t entry_size;
} PHeaderEntry;

/* useful type definitions */
using SymTab = std::unordered_map<std::string, SymEntry>;
using SecTab = std::unordered_map<std::string, SecEntry>;
using RelaTab = std::unordered_map<std::string, std::vector<RelocEntry>>;
using MemTab = std::unordered_map<std::string, std::shared_ptr<std::stringstream>>;
using SegTab = std::unordered_map<uint32_t, std::shared_ptr<std::stringstream>>;

#endif