#define project directories
SRC_DIR = src
ASM_DIR = $(SRC_DIR)/asm
READER_DIR = $(SRC_DIR)/rd
COM_DIR = $(SRC_DIR)/common
LINKER_DIR = $(SRC_DIR)/linker
EMULATOR_DIR = $(SRC_DIR)/emulator
INC_DIR = inc
ASM_INC_DIR = $(INC_DIR)/asm
COM_INC_DIR = $(INC_DIR)/common
READER_INC_DIR = $(INC_DIR)/rd
LINKER_INC_DIR = $(INC_DIR)/linker
EMULATOR_INC_DIR = $(INC_DIR)/emulator
MISC_DIR = misc
OBJ_DIR = obj

#define flex and bison input/output files, all of these are "constant" files
BISON_INPUT = $(MISC_DIR)/parser.y
BISON_OUTPUT = $(MISC_DIR)/parser.cpp

FLEX_INPUT = $(MISC_DIR)/lexer.l
FLEX_OUTPUT = $(MISC_DIR)/lexer.cpp

MISC_SRCS = $(BISON_OUTPUT) $(FLEX_OUTPUT)

#define source files
COM_SRCS = $(wildcard $(COM_DIR)/*.cpp)
COM_DEP = $(patsubst $(COM_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(COM_SRCS))

#asm source files
ASM_SRCS = $(wildcard $(ASM_DIR)/*.cpp)
ASM_DEP = $(patsubst $(ASM_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(ASM_SRCS))
MISC_DEP = $(patsubst $(MISC_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(MISC_SRCS))

#reader source files
READER_SRCS = $(wildcard $(READER_DIR)/*.cpp)
READER_DEP = $(patsubst $(READER_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(READER_SRCS))

#linker source files
LINKER_SRCS = $(wildcard $(LINKER_DIR)/*.cpp)
LINKER_DEP = $(patsubst $(LINKER_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(LINKER_SRCS)) 

#emulator source files
EMULATOR_SRCS = $(wildcard $(EMULATOR_DIR)/*.cpp)
EMULATOR_DEP = $(patsubst $(EMULATOR_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(EMULATOR_SRCS)) 

#object files
MISC_OBJ = $(patsubst $(MISC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(MISC_SRCS))
COM_OBJ = $(patsubst $(COM_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COM_SRCS))

ASM_OBJ = $(patsubst $(ASM_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(ASM_SRCS))
ASM_OBJ += $(MISC_OBJ)
ASM_OBJ += $(COM_OBJ)

READER_OBJ = $(patsubst $(READER_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(READER_SRCS))
READER_OBJ += $(COM_OBJ)

LINKER_OBJ = $(patsubst $(LINKER_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(LINKER_SRCS))
LINKER_OBJ += $(COM_OBJ)

EMULATOR_OBJ = $(patsubst $(EMULATOR_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(EMULATOR_SRCS))
EMULATOR_OBJ += $(COM_OBJ)

#compile with g++
CXX = g++ -std=c++17
CXXFLAGS = -I$(ASM_INC_DIR)
COM_FLAGS = -I$(COM_INC_DIR)
MISC_FLAGS = -I$(MISC_DIR)
READER_FLAGS = -I$(READER_INC_DIR)
LINKER_FLAGS = -I$(LINKER_INC_DIR)
EMULATOR_FLAGS = -I$(EMULATOR_INC_DIR)

all: assembler herring linker emulator

assembler: $(ASM_OBJ)
	$(CXX) -o $@ $^

herring: $(READER_OBJ)
	$(CXX) -o $@ $^

linker: $(LINKER_OBJ)
	$(CXX) -o $@ $^

emulator: $(EMULATOR_OBJ)
	$(CXX) -o $@ $^

$(OBJ_DIR)/%.o: $(ASM_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -o $@ -c $<

$(OBJ_DIR)/%.o: $(MISC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(MISC_FLAGS) -MMD -MP -o $@ -c $<

$(OBJ_DIR)/%.o: $(COM_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(COM_FLAGS) -MMD -MP -o $@ -c $<

$(OBJ_DIR)/%.o: $(READER_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(READER_FLAGS) -MMD -MP -o $@ -c $<

$(OBJ_DIR)/%.o: $(LINKER_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(LINKER_FLAGS) -MMD -MP -o $@ -c $<

$(OBJ_DIR)/%.o: $(EMULATOR_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(EMULATOR_FLAGS) -MMD -MP -o $@ -c $<

$(OBJ_DIR):
	mkdir $@

-include $(ASM_DEP)
-include $(MISC_DEP)
-include $(COM_DEP)
-include $(READER_DEP)
-include $(LINKER_DEP)
-include $(EMULATOR_DEP)

#create bison and flex cpp files
$(BISON_OUTPUT): $(BISON_INPUT)
	bison -d $^

$(FLEX_OUTPUT): $(FLEX_INPUT) 
	flex $^


clean: 
	rm -rf assembler emulator herring linker
	rm -rf $(OBJ_DIR)
	rm -f $(MISC_DIR)/*.hpp $(MISC_DIR)/*.cpp
	