%{
  #include <cmath>
  #include <cstdio>
  #include <iostream>
  #include <vector>
  #include <string>
  #include <deque>
  #include "../inc/asm/assembler.hpp"
  #include "../inc/asm/asm_types.hpp"
  using namespace std;

  extern int yylex();
  extern int yyparse();
  extern FILE *yyin;
  extern int line_num;
  std::deque<ParserArg> parser_args;
  extern Assembler* asem;
  void yyerror(const char *s);
  int32_t i = 0;
%}

%output "misc/parser.hpp"
%defines "misc/parser.cpp"

%union {
  unsigned int i_val;
  char *s_val;
  unsigned char r_val;
}

%token <i_val> INTEGER
%token <s_val> STRING //general string token used for names

// define the constant-string tokens:
%token COMMA
%token ENDL 
%token COLON
%token DOLLAR
%token SQUARE_LEFT
%token SQUARE_RIGHT
%token PLUS
%token <r_val> REGISTER
%token <r_val> CSR

//instruction tokens
%token LD ST
%token HALT
%token INT IRET
%token CALL RET
%token JMP
%token BEQ BNE BGT  
%token PUSH POP
%token XCHG
%token ADD SUB MUL DIV 
%token NOT AND OR XOR
%token SHL SHR
%token CSRRD CSRWR

//directive tokens
%token GLOBAL EXTERN 
%token SECTION WORD
%token SKIP END
%token ASCII EQU
%token <s_val>ASCII_STRING

%%

/* highest level rule */
prog:   
        |
        ENDLS body /* probably a bad idea to define this as the end of grammar */
        |
        body /* perhaps simply call this when the directive is END is found in the file? */
        |
        ENDLS
        ;
body:   
        elem
        |
        elem body
        ;

elem:
        instr {
                if(asem->getPass() == 1) asem->incCounter(4);
        }
        |
        direc 
        |
        symb {
                if(asem->getPass() == 1) {
                        if(asem->addSymbol(parser_args.front().symbol, LOCAL_BIND, true, UNDEFINED_TYPE)) {
                                YYERROR;
                        }
                }
                parser_args.clear();
        }
        ;
instr:
        halt
        |
        interrupt
        |
        call /* include return instruction here? */
        |
        jump
        |
        xchg
        |
        arithmetic
        |
        logic
        |
        move
        |
        store
        |
        load
        ;
direc:
        ASCII ASCII_STRING ENDLS {
                if(asem->initAscii($2)) { YYERROR; };
        }
        |
        END {
                asem->endPass();
                YYACCEPT;
        }
        |
        END ENDLS {
                asem->endPass();
                YYACCEPT;
        }
        |
        GLOBAL SYMS ENDLS {
                for (auto s : parser_args) {
                        if((asem->getPass() == 1 && asem->addSymbol(s.symbol, GLOBAL_BIND, false, UNDEFINED_TYPE)) ||
                        (asem->getPass() == 2 && asem->checkSymbol(s.symbol, GLOBAL_BIND, UNDEFINED_TYPE))) {
                                YYERROR;
                        }
                }       
                parser_args.clear();
        }
        |
        EXTERN SYMS ENDLS {
                for (auto s : parser_args) {
                        if((asem->getPass() == 1 && asem->addSymbol(s.symbol, EXTERN_BIND, false, UNDEFINED_TYPE)) ||
                        (asem->getPass() == 2 && asem->checkSymbol(s.symbol, EXTERN_BIND, UNDEFINED_TYPE))) {
                                YYERROR;
                        }
                }       
                parser_args.clear();
        }
        |
        SECTION STRING ENDLS {
                if(asem->addSection($2)){
                        YYERROR;
                }
                free($2);
        }
        |
        WORD WORDS ENDLS {
                if(asem->initWords(parser_args)) {
                        YYERROR;
                }

                parser_args.clear();
        }
        |
        SKIP INTEGER ENDLS {
                asem->initSkip($2);
        }
        ;
SYMS:
        STRING {
                parser_args.push_back({symbol : $1, literal : 0, arg_type : SYMBOL_ARG});
        }
        |
        SYMS COMMA STRING { 
                parser_args.push_back({symbol : $3, literal : 0, arg_type : SYMBOL_ARG});
        }
        ;
WORDS: 
        STRING {
                parser_args.push_back({symbol : $1, literal : 0, arg_type : SYMBOL_ARG});
        }
        |
        INTEGER {
                parser_args.push_back({symbol : "", literal : $1, arg_type : LITERAL_ARG});
        }
        |
        WORDS COMMA STRING {
                parser_args.push_back({symbol : $3, literal : 0, arg_type : SYMBOL_ARG});
        }
        |
        WORDS COMMA INTEGER {
                parser_args.push_back({symbol : "", literal : $3, arg_type : LITERAL_ARG});
        }
        ;
symb:
        STRING COLON ENDLS {
                if(asem->getPass() == 1) parser_args.push_back({symbol : $1, literal : 0, arg_type : SYMBOL_ARG});
                free($1);
        }
        |
        STRING COLON {
                if(asem->getPass() == 1) parser_args.push_back({symbol : $1, literal : 0, arg_type : SYMBOL_ARG});
                free($1);
        } 
        ;
/* instruction grammar */
halt:
        HALT ENDLS {
                if(asem->initInstruction({HALT_ASM, 0, 0, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        ;
interrupt:
        INT ENDLS {
                if(asem->initInstruction({INT_ASM, 0, 0, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        IRET ENDLS {
                if(asem->initInstruction({IRET_ASM, 0, 0, 0, {"", 0, REG_ARG}})) { YYERROR; }
                if(asem->getPass() == 1) asem->incCounter(4);
        }
        ;
call:
        CALL STRING ENDLS {
                if(asem->initInstruction({CALL_ASM, R15, 0, 0, {$2, 0, SYMBOL_ARG}})) { YYERROR; }
        }
        |
        CALL INTEGER ENDLS {
                if(asem->initInstruction({CALL_ASM, R15, 0, 0, {"", $2, LITERAL_ARG}})) { YYERROR; }
        }
        |
        RET ENDLS {
                if(asem->initInstruction({RET_ASM, R15, R14, 0, {"", 4, LITERAL_ARG}})) { YYERROR; }
        }
        ;
jump:  
        JMP INTEGER ENDLS {
                if(asem->initInstruction({JMP_ASM, R15, 0, 0, {"", $2, LITERAL_ARG}})) { YYERROR; }
        }
        |
        JMP STRING ENDLS {
                if(asem->initInstruction({JMP_ASM, R15, 0, 0, {$2, 0, SYMBOL_ARG}})) { YYERROR; }
        }
        |
        BEQ REGISTER COMMA REGISTER COMMA INTEGER ENDLS {
                if(asem->initInstruction({BEQ_ASM, R15, $2, $4, {"", $6, LITERAL_ARG}})) { YYERROR; }
        }
        |
        BEQ REGISTER COMMA REGISTER COMMA STRING ENDLS {
                if(asem->initInstruction({BEQ_ASM, R15, $2, $4, {$6, 0, SYMBOL_ARG}})) { YYERROR; }
        } 
        |
        BGT REGISTER COMMA REGISTER COMMA INTEGER ENDLS {
                if(asem->initInstruction({BGT_ASM, R15, $2, $4, {"", $6, LITERAL_ARG}})) { YYERROR; }
        }
        |
        BGT REGISTER COMMA REGISTER COMMA STRING ENDLS {
                if(asem->initInstruction({BGT_ASM, R15, $2, $4, {$6, 0, SYMBOL_ARG}})) { YYERROR; }
        }
        |
        BNE REGISTER COMMA REGISTER COMMA INTEGER ENDLS {
                if(asem->initInstruction({BNE_ASM, R15, $2, $4, {"", $6, LITERAL_ARG}})) { YYERROR; }
        }   
        |
        BNE REGISTER COMMA REGISTER COMMA STRING ENDLS {
                if(asem->initInstruction({BNE_ASM, R15, $2, $4, {$6, 0, SYMBOL_ARG}})) { YYERROR; }
        }
        ;
xchg:
        XCHG REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({XCHG_ASM, 0, $2, $4, {"", 0, REG_ARG}})) { YYERROR; }
        }
        ;
arithmetic:
        ADD REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({ADD_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        SUB REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({SUB_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        MUL REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({MUL_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        DIV REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({DIV_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        ; 
logic:
        AND REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({AND_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        OR REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({OR_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        XOR REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({XOR_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        NOT REGISTER ENDLS {
                if(asem->initInstruction({NOT_ASM, $2, $2, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        ;
move:
        SHL REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({SHL_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        SHR REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({SHR_ASM, $4, $4, $2, {"", 0, REG_ARG}})) { YYERROR; }
        }
        ;
store:
        ST REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({ST_REG_DIR_ASM, $4, $2, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        ST REGISTER COMMA ARG ENDLS {
                if(asem->initInstruction({ST_MEM_DIR_ASM, PC, 0, $2, parser_args.front()})) { YYERROR; }
                parser_args.pop_front();
        }
        |
        ST REGISTER COMMA SQUARE_LEFT REGISTER SQUARE_RIGHT ENDLS {
                if(asem->initInstruction({ST_REG_IND_ASM, $5, 0, $2, {"", 0, REG_ARG}})) { YYERROR; }
        } 
        |
        ST REGISTER COMMA SQUARE_LEFT REGISTER PLUS ARG SQUARE_RIGHT ENDLS {
                if(asem->initInstruction({ST_REG_IND_DISP_ASM, $5, 0, $2, parser_args.front()})) { YYERROR; }
                parser_args.pop_front();
        }
        |
        PUSH REGISTER ENDLS {
                if(asem->initInstruction({PUSH_ASM, 14, 0, $2, {"", static_cast<uint32_t>(-4), LITERAL_ARG}})) { YYERROR; }
        }
        ;
load:
        LD REGISTER COMMA REGISTER ENDLS {
                if(asem->initInstruction({LD_REG_DIR_ASM, $4, $2, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        LD ARG COMMA REGISTER ENDLS {
                auto& arg = parser_args.front();
                if(asem->initInstruction({LD_MEM_DIR_ASM, $4, 0, 0, arg})) { YYERROR; }
                if(asem->getPass() == 1) asem->incCounter(4);
                parser_args.clear();
        }
        |
        LD DOLLAR ARG COMMA REGISTER ENDLS {
                if(asem->initInstruction({LD_IMMED_ASM, $5, R15, 0, parser_args.front()})) { YYERROR; }
                parser_args.clear();
        }
        |
        LD SQUARE_LEFT REGISTER SQUARE_RIGHT COMMA REGISTER ENDLS {
                if(asem->initInstruction({LD_REG_IND_ASM, $6, $3, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        LD SQUARE_LEFT REGISTER PLUS ARG SQUARE_RIGHT COMMA REGISTER ENDLS {
                if(asem->initInstruction({LD_REG_IND_DISP_ASM, $8, $3, 0, parser_args.front()})) { YYERROR; }
                parser_args.clear();
        }
        |
        POP REGISTER ENDLS {
                if(asem->initInstruction({POP_ASM, $2, 14, 0, {"", 4, LITERAL_ARG}})) { YYERROR; }
        }
        |
        CSRRD CSR COMMA REGISTER ENDLS {
                if(asem->initInstruction({CSRRD_ASM, $4, $2, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        |
        CSRWR REGISTER COMMA CSR ENDLS {
                if(asem->initInstruction({CSRWR_ASM, $4, $2, 0, {"", 0, REG_ARG}})) { YYERROR; }
        }
        ;
ARG:
        INTEGER {
                parser_args.push_back({symbol : "", literal : $1, arg_type : LITERAL_ARG});
        }
        |
        STRING {
                parser_args.push_back({symbol : $1, literal : 0, arg_type : SYMBOL_ARG});
                free($1);
        }
        ;
ENDLS:
        ENDLS ENDL
        | 
        ENDL 
        ;

%%
 void yyerror (char const *s) {
        fprintf (stderr, "%s\n", s);
 }
