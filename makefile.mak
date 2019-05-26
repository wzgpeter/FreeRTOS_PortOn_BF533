
export ADI_DSP ?= c:/Program Files/Analog Devices/VisualDSP 4.5
export PROC ?= ADSP-BF533
export SI_REVISION ?= 0.3
export CYGWIN_DIR ?= c:/cygwin/bin
export USER_DIR ?= $(CYGWIN_DIR)/pwd/bin
export AUDIO_CHIPSET ?= AD1940_REV1

PROJ_NAME = freeRTOS
PROJ_TYPE = fw

export CHIP_FLAGS = -D AUDIO_CHIPSET=$(AUDIO_CHIPSET) -D S200

#
# configure special memory location for special module
#
DRIVER_DIR_SRC = drivers
DRIVER_DIR_INC = drivers/include
DRIVER_SECTIONS = code=L1_code,data=L1_data

RTOS_DIR_SRC = rtos
RTOS_DIR_INC = rtos/include
RTOS_SECTIONS = code=L1_code,data=L1_data,constdata=L1_data

APP_DIR_SRC = app
APP_DIR_INC = app/include

OUTPUT_PATH = ./

#
# Got the whole project SRC and INC folder
#
SRC_DIRS = $(APP_DIR_SRC) $(RTOS_DIR_SRC) $(DRIVER_DIR_SRC)
INC_DIRS = -I$(APP_DIR_INC) -I$(RTOS_DIR_INC) -I$(DRIVER_DIR_INC)

#
# Setup flags for C compiler
#
export USR_CFLAGS = -g -structs-do-not-overlap -warn-protos -decls-strong -flags-compiler --diag_warning,implicit_func_decl -Wsuppress 0111,0188 #-Wremarks
export ADI_CFLAGS = -c -proc $(PROC) -si-revision $(SI_REVISION)
export CFLAGS = $(USR_CFLAGS) $(ADI_CFLAGS) $(CHIP_FLAGS)

#
# Setup flags for assembler compiler
#
export USR_ASMFLAGS = -g
export ADI_ASMFLAGS = -proc $(PROC) -si-revision $(SI_REVISION)
export ASMFLAGS = $(USR_ASMFLAGS) $(ADI_ASMFLAGS) $(CHIP_FLAGS)

#
# Setup Linker flags
#
export USR_LINKFLAGS = -flags-link -MDUSE_CACHE,-MDUSE_INSTRUCTION_CACHE,-MDUSE_DATA_A_CACHE,-MDUSE_DATA_B_CACHE,-MDUSE_SCRATCHPAD_TASK_STACK
export ADI_LINKFLAGS = -mem-bsz -si-revision $(SI_REVISION) -proc $(PROC) -flags-link -e,-od,.
export LINKFLAGS = $(USR_LINKFLAGS) $(ADI_LINKFLAGS) -flags-link -Map,$(MAP_FILE)

#
# Setup Loader flags
#
export USR_LOADFLAGS = 
export ADI_LOADFLAGS = -f hex -Width 16 -si-revision $(SI_REVISION) -proc $(PROC)
export LOADFLAGS = $(USR_LOADFLAGS) $(ADI_LOADFLAGS)

#
# Define Project Build Output File
#
MAP_FILE = $(PROJ_NAME)_$(PROJ_TYPE).map.xml
DXE_FILE = $(PROJ_NAME)_$(PROJ_TYPE).dxe
LDR_FILE = $(PROJ_NAME)_$(PROJ_TYPE).ldr
S_RECORD = $(PROJ_NAME)_$(PROJ_TYPE).s
LDF_FILE = $(PROC).ldf


#
# Define the Tool Path
#
export CC = "$(ADI_DSP)"/ccblkfn
export AS = "$(ADI_DSP)"/easmblkfn
export LD = "$(ADI_DSP)"/elfloader

export RM = "$(CYGWIN_DIR)"/rm
export SED = "$(CYGWIN_DIR)"/sed
export CAT = "$(CYGWIN_DIR)"/cat 
export ECHO = "$(CYGWIN_DIR)"/echo
export TOUCH = "$(CYGWIN_DIR)"/touch
export SHELL = "$(CYGWIN_DIR)"/sh

#
# Automatically search for source files in multi sub-directories
#
C_FILES = $(foreach dir,$(SRC_DIRS), $(wildcard $(dir)/*.c))
C_OBJTS = $(patsubst %.c,%.doj,$(C_FILES))

ASM_FILES = $(foreach dir,$(SRC_DIRS), $(wildcard $(dir)/*.asm))
ASM_OBJTS = $(patsubst %.asm,%.doj,$(ASM_FILES))

LIB_OBJTS = $(foreach dir,$(SRC_DIRS), $(wildcard $(dir)/*.dlb))

HDR_FIELS = $(foreach dir,$(INC_DIRS), $(wildcard $(dir)/*.h))
OBJS = $(C_OBJTS) $(ASM_OBJTS) $(LIB_OBJTS)


#-----------------------------------------------------------------------------------

%.doj : %.c	
	@$(ECHO) "Compiling $< -->> $@"
	$(CC) $(CFLAGS) $(INC_DIRS) $< -o $@

%.doj : %.asm
	@$(ECHO) "Assembling $< -->> $@"
	$(AS) $(ASMFLAGS) $(INC_DIRS) $< -o $@	

#-----------------------------------------------------------------------------------

#
# this is the start point of script
#
.PHONY: all
all:
	@$(ECHO) ""
	@$(ECHO) "  NOTE: this is FreeRTOS project "
	@$(ECHO) ""
	@$(ECHO) "  dsp_build - Build the FreeRTOS project "
	@$(ECHO) "  clean     - Clean out compiler objects "
	@$(ECHO) "  all       - Show this message here"
	@$(ECHO) ""


#-----------------------------------------------------------------------------------

.PHONY: dsp_build
dsp_build: $(DXE_FILE)
	
$(DXE_FILE) : $(OBJS) $(LDF_FILE)
	$(CC) $(OBJS) -T $(LDF_FILE) $(LINKFLAGS) -o $@

	


#	
#	this is the rule for running clean
#	
.PHONY: clean
clean:
	for dir in $(RECURSE_DIRS); do (cd $$dir && $(MAKE) clean); done
	-$(RM) -f $(C_OBJTS) $(ASM_OBJTS)
	-$(RM) -f $(PROJ_NAME)_$(PROJ_TYPE).dxe
	-$(RM) -f $(PROJ_NAME)_$(PROJ_TYPE).map.xml
	-$(RM) -f $(PROJ_NAME)_$(PROJ_TYPE).ldr
	-$(RM) -f $(PROJ_NAME)_$(PROJ_TYPE).s
	
	

