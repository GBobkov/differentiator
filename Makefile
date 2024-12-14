ifeq ($(origin CC), default)
	CC = g++
endif
COMMONINC = -I include -I .
CFLAGS ?= -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code  -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -g
OUT_O_DIR := build

CSRC = main.cpp src/optimizator.cpp src/tree_for_diff.cpp src/diffor.cpp src/tree_dump.cpp src/read_data.cpp src/write_data.cpp
override CFLAGS += $(COMMONINC)

COBJ := $(addprefix $(OUT_O_DIR)/, $(CSRC:%.cpp=%.o)) 
DEPS := $(COBJ:%.o=%.d) 

.PHONY: all Release Debug  сlean doxygen
all: $(OUT_O_DIR)/main.exe
Release: $(OUT_O_DIR)/main.exe
Debug: $(OUT_O_DIR)/main.exe

сlean:
	rm $(COBJ) $(DEPS) 

doxygen:
	doxygen
	
$(OUT_O_DIR)/main.exe : $(COBJ)
	@$(CC) $^ -o $@ $(CFLAGS)

$(COBJ): $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $<  -o $@

$(DEPS): $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	@$(CC) -E $(CFLAGS) $< -MM -MT $(@:%.d=%.o) > $@
 
