%{
        #include "parser.hpp"
        #include <iostream>
        #include <string>
        int line_num = 0;
        extern std::unordered_map<std::string, Registers> register_mappings;
%}
/*flex otpions*/
%option noyywrap
%option outfile="misc/lexer.cpp" 


/*generally useful regular expressions*/

WHS     [ \t\r]
STR     [^,\+\n\[\]\$\t\r: ]+
/* string should contain no special characters, excluding a "." for section names */

/*syntactically imporant regular expressions*/
COLON     ":"
COM   "#"[^\n\r]* 
COMMA   ","
DOLLAR "\$"
HEX     0x[0-9a-fA-F]+
DEC     [-]?[0-9]+
REG     (%r([0-9]|1[0-5]))|(%sp|%pc)
CSR     (%status|%handler|%cause)
SQUARE_LEFT "\["
SQUARE_RIGHT "\]"
PLUS    "\+"
/*instruction literals*/
HALT    "halt"
INT     "int"
IRET    "iret"
CALL    "call"
RET     "ret"
JMP     "jmp"
BEQ     "beq"
BNE     "bne"
BGT     "bgt"
PUSH    "push"
POP     "pop"
XCHG    "xchg"
ADD     "add"
SUB     "sub"
MUL     "mul"
DIV     "div"
NOT     "not"
AND     "and"
OR      "or"
XOR     "xor"
SHL     "shl"
SHR     "shr"
LD      "ld"
ST      "st"
CSRRD   "csrrd"
CSRWR   "csrwr"

/* directive literals */
GLOBAL "\.global"
EXTERN "\.extern"
SECTION "\.section"
WORD "\.word"
SKIP "\.skip"
ASCII "\.ascii"
EQU "\.equ"
END "\.end"



/* nemam pojma kako se koristi ovaj alat lol. */

%%
{HALT}    { return HALT; }
{INT}     { return INT; }
{IRET}    { return IRET; }
{CALL}    { return CALL; }
{RET}     { return RET; }
{JMP}     { return JMP; }
{BEQ}     { return BEQ; }
{BNE}     { return BNE; }
{BGT}     { return BGT; }
{PUSH}    { return PUSH; }
{POP}     { return POP; }
{XCHG}    { return XCHG; }
{ADD}     { return ADD; }
{SUB}     { return SUB; }
{MUL}     { return MUL; }
{DIV}     { return DIV; }
{NOT}     { return NOT; }
{AND}     { return AND; }
{OR}      { return OR; }
{XOR}     { return XOR; }
{SHL}     { return SHL; }
{SHR}     { return SHR; }
{LD}      { return LD; }
{ST}      { return ST; }
{CSRRD}   { return CSRRD; }
{CSRWR}   { return CSRWR; }      

{END}     { return END; }
{GLOBAL}  { return GLOBAL; }
{EXTERN}  { return EXTERN; }
{SECTION} { return SECTION; }
{WORD}    { return WORD; }
{SKIP}    { return SKIP; }
{ASCII}   { return ASCII; }
{EQU}     { return EQU; }

{DOLLAR} {
                return DOLLAR;
        }

{SQUARE_LEFT} {
                return SQUARE_LEFT;
        }

{SQUARE_RIGHT} {
                return SQUARE_RIGHT;
        }

{PLUS} {
                return PLUS;
        }

{REG}   {
                yylval.r_val = register_mappings[yytext];
                return REGISTER;
        }

{CSR}   {
                yylval.r_val = register_mappings[yytext];
                return CSR;
        }

{DEC}   {
                yylval.i_val = std::stol(yytext, nullptr);
                return INTEGER;
        }

{HEX}   {
                yylval.i_val = std::stol(yytext, nullptr, 16);
                return INTEGER;
        }


{COM}   { /* ignore comments */ }
{WHS}   { /* ignore *some* whitespaces */ }

{COLON} { return COLON; }
{COMMA}   { return COMMA; }
{STR}   {
                yylval.s_val = strdup(yytext);
                return STRING;
        }

[\n]    { return ENDL; }

%%
