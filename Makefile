#################################################################################
#                               param Makefile                                  #
#                                                                               #
#################################################################################

# 0 means off; 1 means on
#DEBUG required with VERBOSE
DEBUG = 1
VERBOSE = 0

# any custom library directories etc.
INCLUDE_DIR =

# compiler options
CXX      = g++
OPT      = 
COPT     = -c
OUTOPT   = -o 
LINK_OPT = -static
LINK_OUT = -o 

LINKER   ?= ${CXX}
CXXFLAGS ?= -I$(INCLUDE_DIR)
CXXFLAGS += --Wall -Werror -Wno-unknown-pragmas $(OPT) -MMD

# Optimizations
CXXFLAGS += -fcprop-registers \
            -fdefer-pop \
            -fguess-branch-probability \
            -fif-conversion2\
            -fif-conversion\
            -fipa-pure-const\
            -fipa-reference\
            -fmerge-constants\
            -ftree-ccp\
            -ftree-ch\
            -ftree-copyrename \
            -ftree-dominator-opts \
            -ftree-fre\
            -ftree-sra\
            -ftree-ter\
            -ftree-dce\
            -ftree-dse\
            -funit-at-a-time\
            -finline-functions\
            -floop-optimize\
            -falign-functions \
            -falign-loops \
            -funroll-loops

ifeq ($(DEBUG),1)
  DBG = -ggdb
  CXXFLAGS += -D DEBUG
else
  DBG =
endif

ifeq ($(VERBOSE),1)
  CXXFLAGS += -D VERBOSE
endif

TYPES = param_types.h param_funcs.h
CONFIG_OBJ = Config.cpp Config.h
VALID_OBJ = ConfigValidator.cpp ConfigValidator.h
CONSTRUCT_S_OBJ = ConstructSkeleton.cpp ConstructSkeleton.h
ASSIMILATE_S_OBJ = AssimilateSkeleton.cpp AssimilateSkeleton.h
SKELETON_OBJ = Skeleton.cpp Skeleton.h
CELL_OBJ = Cell.cpp Cell.h
NAN_ASS = utilities/nanassert.cpp utilities/nanassert.h
SKIN_OBJ = Skin.cpp Skin.h
INST_OBJ = Instruction.cpp Instruction.h
BODY_OBJ = Body.cpp Body.h
COMP_OBJ = Compressor.cpp Compressor.h

OBJS = Config.o ConfigValidator.o AssimilateSkeleton.o ConstructSkeleton.o Skeleton.o Cell.o Skin.o Body.o \
		 Compressor.o Instruction.o utilities/nanassert.o param.o

## build rules
all: param

param : $(OBJS)
	${LINKER} ${PIN_LDFLAGS} $(LINK_DEBUG) ${LINK_OPT} ${LINK_OUT} param $(OBJS) $(DBG)

Config.o : $(CONFIG_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

ConfigValidator.o : $(VALID_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

AssimilateSkeleton.o : $(ASSIMILATE_S_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

ConstructSkeleton.o : $(CONSTRUCT_S_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

Skeleton.o : $(SKELETON_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

Cell.o : $(CELL_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

Compressor.o : $(COMP_OBJ) $(CELL_OBJ) $(TYPES) SequiterClasses.h
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

Skin.o : $(SKIN_OBJ) $(INST_OBJ) $(SKELETON_OBJ) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

Body.o : $(BODY_OBJ) $(SKIN_OBJ) $(TYPES) OperandList.h
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

Instruction.o : $(INST_OBJ) $(TYPES) OperandList.h
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

utilities/nanassert.o.o : $(NAN_ASS) $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

param.o : param.cpp $(TYPES)
	${CXX} ${COPT} $(CXXFLAGS) $(DBG) ${OUTOPT}$@ $< 

force:
	touch *.cpp *.h; make

clean:
	rm -f *.d *.o param utilities/nanassert.o
