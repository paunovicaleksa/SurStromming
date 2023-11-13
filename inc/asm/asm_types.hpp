#include <unistd.h>
#include <string>

#ifndef ASM_TYPES_HPP
#define ASM_TYPES_HPP

/* parser structs? */
enum ArgType: uint8_t {
        SYMBOL_ARG,
        LITERAL_ARG,
        REG_ARG
};

typedef struct {
        std::string symbol;
        uint32_t literal;
        ArgType arg_type;
} ParserArg;

enum AsmInstruction: uint8_t {
        HALT_ASM,
        INT_ASM,
        IRET_ASM,
        CALL_ASM,
        RET_ASM,
        JMP_ASM,
        BEQ_ASM,
        BNE_ASM,
        BGT_ASM,
        PUSH_ASM,
        POP_ASM,
        XCHG_ASM,
        ADD_ASM,
        SUB_ASM,
        MUL_ASM,
        DIV_ASM,
        NOT_ASM,
        AND_ASM,
        OR_ASM,
        XOR_ASM,
        SHL_ASM,
        SHR_ASM,
        LD_IMMED_ASM,
        LD_MEM_DIR_ASM,
        LD_REG_DIR_ASM,
        LD_REG_IND_ASM,
        LD_REG_IND_DISP_ASM,
        ST_MEM_DIR_ASM,
        ST_REG_DIR_ASM,
        ST_REG_IND_ASM,
        ST_REG_IND_DISP_ASM,
        CSRRD_ASM,
        CSRWR_ASM
};


typedef struct {
        AsmInstruction asm_instruction;
        uint8_t reg1;
        uint8_t reg2;
        uint8_t reg3;
        ParserArg arg;
} InstrDesc;

#endif

