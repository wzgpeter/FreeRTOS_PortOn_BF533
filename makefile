# Any variable defined with a "?=" can be overridden by the command 
# line or environment.
#
#   To set via the environment: 
#         Start->Settings->Control Panel->Advanced->Environment Variables
#
#   To set via the command line:
#         make VARIABLE1=VALUE1 VARIABLE2=VALUE2 all
#
# Some particularly important, and variable, defines are:
#
#   ADI_DSP = The path to your VisualDSP++ tool installation
#   PROC = Defines your processor.  Supported procs are ADSP-BF537 and ADSP-BF538
#   SI_REVISION = Defines the silicon revision of the processor
#   CYGWIN_BINDIR = Path to your cygwin installation
#   HP_BINDIR = Path to necessary HP tools
#
# This makefile is designed for gmake.  For gmake information, go to:
#   http://www.gnu.org/software/make/manual/make.html
#
# To get cygwin, go to:
#   http://www.cygwin.com/
#
# Make sure to select the "make" package from the cygwin install to get gmake!
#

#
# Housekeeping that must come first
#
.SUFFIXES:                    # Delete the default suffixes
.SUFFIXES: .c .doj .h .asm    # Define our suffix list

ifeq ($(origin ADI_DSP), undefined)
ADI_DSP_DEFINED = "default"
else
ADI_DSP_DEFINED = "overridden"
endif

ifeq ($(origin CYGWIN_BINDIR), undefined)
CYGWIN_BINDIR_DEFINED = "default"
else
CYGWIN_BINDIR_DEFINED = "overridden"
endif

ifeq ($(origin HP_BINDIR), undefined)
HP_BINDIR_DEFINED = "default"
else
HP_BINDIR_DEFINED = "overridden"
endif

#
# Project build definitions and options go here
#
export ADI_DSP ?= c:/Program Files/Analog Devices/VisualDSP 4.5
export PROC ?= ADSP-BF533
export SI_REVISION ?= 0.3
export CYGWIN_BINDIR ?= c:/cygwin/bin
export HP_BINDIR ?= `$(CYGWIN_BINDIR)/pwd`/bin
export AUDIO_CHIPSET ?= AD1940_REV1
#
# Project build definition that indicates to the bootloader makefile,  
# whether the bootloader can be overwritten during a s-record download.
# Note that if the value is set to No and you need to flash through
# the emulator, then you should rebuild the bootloader in it's subdirectory
# to obtain a DXE that allows the bootloader to be overwritten.
# 
export OVERWRITE_BOOTLOADER ?= No


#
# Project build definition that indicates to the bootloader makefile,  
# to enable bus support.  This is needed for having CAN enabled
# in the bootloader.
# 
#export ENABLE_BOOTLOADER_BUS_SUPPORT ?= Yes


PROJECT_NAME = freeRTOS

export USER_FLAGS = -D AUDIO_CHIPSET=$(AUDIO_CHIPSET) -D S200 


#
# Export #defines to the bootloader that are used to control such things
# as the flash sectors that will be erased during a flash update and
# the area of flash to checksum
#
export APP_TO_BOOT_DEFINES  = -D SREC_APP_START_ADDRESS=0x00000000 -D SREC_START_CHECKSUM=0x00008000
## export APP_TO_BOOT_DEFINES += -D FLASH_SECTORS_BOOTLOADER="{0,1}" -D FLASH_SECTORS_DTCP_KEYS="{2}" -D FLASH_SECTORS_EQ_TABLE="{3,4}"
export APP_TO_BOOT_DEFINES += -D FLASH_SECTORS_APPLICATION="{0,1,2,3,4,5,6,7,8,9,10}" 

#
# Configure special memory locations for certain modules.  By default, 
# code (code), data (data1), and constants (constdata) will be placed in 
# external SDRAM.  If you define a special directory here, make sure 
# to add a rule for it below.
#
DRIVER_DIR_SRC = drivers
DRIVER_DIR_INC = drivers/include
DRIVER_SECTIONS = code=L1_code,data=L1_data

RTOS_DIR_SRC = rtos
RTOS_DIR_INC = rtos/include
RTOS_DIR_SRC_COM = rtos/Common/Minimal
RTOS_DIR_INC_COM = rtos/Common/include
RTOS_SECTIONS = code=L1_code,data=L1_data,constdata=L1_data

APP_DIR_SRC = app
APP_DIR_INC = app/include



#
# Define project directories.  These tell the makefile where to look
# for source to compile.  Any .c or .asm file in these dirs will be 
# built (Be careful: ".C" != ".c", same for ".asm").
#
SOURCE_DIRS   = $(APP_DIR_SRC) $(DRIVER_DIR_SRC) $(RTOS_DIR_SRC) $(RTOS_DIR_SRC_COM)
INCLUDE_DIRS  = -I$(APP_DIR_INC) -I$(DRIVER_DIR_INC) -I$(RTOS_DIR_INC) -I$(RTOS_DIR_INC_COM)


#
# Set up compiler flags.  Enable as many warnings as practically reasonable (especially
# implicit function declarations).
#
export HP_CFLAGS = -g -structs-do-not-overlap -warn-protos -decls-strong -flags-compiler --diag_warning,implicit_func_decl -Wsuppress 0111,0188 #-Wremarks
export ADI_CFLAGS = -c -proc $(PROC) -si-revision $(SI_REVISION) 
export CFLAGS = $(ADI_CFLAGS) $(HP_CFLAGS) $(USER_FLAGS)


#
# Set up assembler flags
#
export HP_ASMFLAGS = -g
export ADI_ASMFLAGS = -proc $(PROC) -si-revision $(SI_REVISION)
export ASMFLAGS = $(ADI_ASMFLAGS) $(HP_ASMFLAGS) $(USER_FLAGS) 

#
# Set up linker flags and directories
#
export HP_LINKFLAGS = -flags-link -MDUSE_CACHE,-MDUSE_INSTRUCTION_CACHE,-MDUSE_DATA_A_CACHE,-MDUSE_DATA_B_CACHE,-MDUSE_SCRATCHPAD_TASK_STACK
export ADI_LINKFLAGS = -mem-bsz -si-revision $(SI_REVISION) -proc $(PROC) -flags-link -e,-od,.
export LINKFLAGS = $(ADI_LINKFLAGS) $(HP_LINKFLAGS) -flags-link -Map,$(MAP_FILE)

#
# Set up loader flags
#
## export HP_LOADERFLAGS = -init $(EBIU_PRE_INIT_DXE)
export ADI_LOADERFLAGS = -f hex -Width 16 -si-revision $(SI_REVISION) -proc $(PROC)
export LOADERFLAGS = $(ADI_LOADERFLAGS) $(HP_LOADERFLAGS) 


#
# Set up Lint flags and directories
#
LINT_FLAGS = -zero ./bin/opt.lnt -D__ADSPLPBLACKFIN__ -D__ADSPBLACKFIN__ -D__EDG_VERSION__
# Add needed defines that are usually handled by the compiler

ifeq ($(PROC), ADSP-BF533)
LINT_FLAGS += -D__ADSPBF533__  
endif

LINT_FLAGS += -DAUDIO_CHIPSET=$(AUDIO_CHIPSET) -DFADE_SIDE_AS_REAR -DASL_ENABLED -DDEFAULT_DRIVE=RHD
LINT_FLAGS += $(INCLUDE_DIRS) -i"$(ADI_DSP)"/Blackfin/include

#
# Define the project build output files
#
MAP_FILE = $(PROJECT_NAME)_$(PROJECT_TYPE).map.xml
DXE_FILE = $(PROJECT_NAME)_$(PROJECT_TYPE).dxe
LDR_FILE = $(PROJECT_NAME)_$(PROJECT_TYPE).ldr
S_RECORD = $(PROJECT_NAME)_$(PROJECT_TYPE).s
LDF_FILE = $(PROC).ldf

# Project type names
PROJECT_TYPE_PCM     = stereo
PROJECT_TYPE_DECODER = decoder

#
# Define where to find various tools
#
export CC = "$(ADI_DSP)"/ccblkfn
export ASM = "$(ADI_DSP)"/easmblkfn
export LOADER = "$(ADI_DSP)"/elfloader

export RM = "$(CYGWIN_BINDIR)"/rm
export ECHO = "$(CYGWIN_BINDIR)"/echo
export SED = "$(CYGWIN_BINDIR)"/sed
export CAT = "$(CYGWIN_BINDIR)"/cat
export TOUCH = "$(CYGWIN_BINDIR)"/touch
export SHELL = "$(CYGWIN_BINDIR)"/sh

export HEX2SREC = "$(HP_BINDIR)"/ihex2srec
export SRECRELO = "$(HP_BINDIR)"/srecrelo
export VECT2SREC = "$(HP_BINDIR)"/vect2srec
export MAKEDEPEND = "$(HP_BINDIR)"/makedepend
export EQ2LDR = "$(HP_BINDIR)"/eq2ldr

export BIN_LINT = "$(HP_BINDIR)"/lint-nt

#
# Automatically search for source files to build in the various
# subdirectories
#
C_FILES = $(foreach dir,$(SOURCE_DIRS), $(wildcard $(dir)/*.c))
C_OBJECTS = $(patsubst %.c,%.doj,$(C_FILES))
LINT_FILES = $(patsubst %.c,%.le,$(C_FILES))

ASM_FILES = $(foreach dir,$(SOURCE_DIRS), $(wildcard $(dir)/*.asm))
ASM_OBJECTS = $(patsubst %.asm,%.doj,$(ASM_FILES))

LIB_OBJECTS = $(foreach dir,$(SOURCE_DIRS), $(wildcard $(dir)/*.dlb))

HEADER_FILES = $(foreach dir,$(SOURCE_DIRS), $(wildcard $(dir)/*.h))
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS) $(LIB_OBJECTS)


#
# Since this help section comes first, it will be shown if no target 
# is specified.
#
.PHONY: all
all:
	@$(ECHO) ""
	@$(ECHO) "NOTE: Any DD/DTS project requires licensing!!!"
	@$(ECHO) ""
	@$(ECHO) "Available build TARGETs are:"
	@$(ECHO) ""
	@$(ECHO) "  dm_decoder   - Build the DM Amp DD/DTS project version"
	@$(ECHO) "  clean        - Clean out compiler objects"
	@$(ECHO) "  lint         - Run lint"
	@$(ECHO) "  lint_clean   - Remove all lint output files"
	@$(ECHO) "  all          - Show this message"
	@$(ECHO) ""
	@$(ECHO) ""
	@$(ECHO) "Additional build options include:"
	@$(ECHO) ""
	@$(ECHO) "  OPT:      This disables all optimizations normally present in the build,"
	@$(ECHO) "            specified in the makefile."
	@$(ECHO) "            Default set to 'Y'.  Set 'OPT=N' to disable optimization."
	@$(ECHO) ""
	@$(ECHO) "Your build settings are:"
	@$(ECHO) ""
	@$(ECHO) "  ADI Tools ("$(ADI_DSP_DEFINED)"):" 
	@$(ECHO) "    " $(ADI_DSP)
	@$(ECHO) "  Cygwin Tools ("$(CYGWIN_BINDIR_DEFINED)"):" 
	@$(ECHO) "    " $(CYGWIN_BINDIR)
	@$(ECHO) "  HP Tools ("$(HP_BINDIR_DEFINED)"):" 
	@$(ECHO) "    " $(HP_BINDIR)
	@$(ECHO) "  User Flags (defined in USER_FLAGS):"
	@$(ECHO) "    " $(USER_FLAGS)
	@$(ECHO) ""
	@$(ECHO) "These can be overridden by the environment or the command line"

#
# Set optimization flags
#
OPT ?= N
ifeq ($(OPT), N)
# Max Optimization flags
HP_CFLAGS += -Og 
else
# Optimization flags used for early development
HP_CFLAGS += -O1 -Ov100
endif


#
# List the targets
#



# DM DD/DTS  
dm_decoder: export APP_TO_BOOT_DEFINES += -D VEHICLE_TYPE=DM_DECODER
dm_decoder: USER_FLAGS += -D VEHICLE_TYPE=DM_DECODER -D SPDIF_MUTE_MUTES_HFM
dm_decoder: PROJECT_TYPE = $(PROJECT_TYPE_DECODER)
dm_decoder: subdirs $(DXE_FILE) $(S_RECORD)



#
# More specific implicit rules to put certain modules into 
# specific memory regions.
#
$(RTOS_DIR)/%.doj : $(RTOS_DIR)/%.c
	@$(ECHO) "Compiling $<"
	$(CC) $(CFLAGS) -section $(RTOS_SECTIONS) $(INCLUDE_DIRS) $< -o $@
$(DRIVER_DIR)/%.doj : $(DRIVER_DIR)/%.c
	@$(ECHO) "Compiling $<"
	$(CC) $(CFLAGS) -section $(DRIVER_SECTIONS) $(INCLUDE_DIRS) $< -o $@

#
# General implicit rules go here
#

%.doj : %.c
	@$(ECHO) "Compiling $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

%.doj : %.asm
	@$(ECHO) "Assembling $<"
	$(ASM) $(ASMFLAGS) $(INCLUDE_DIRS) $< -o $@

$(DXE_FILE) : $(OBJECTS) $(LDF_FILE) $(LIBS) $(RECURSE_DEPENDENCIES)
	$(CC) $(OBJECTS) $(LIBS) -T $(LDF_FILE) $(LINKFLAGS) -o $(DXE_FILE)

#
# This is the rule for ldr to s-record conversion
#
$(S_RECORD) : $(DXE_FILE) $(RECURSE_DEPENDENCIES)
#	@$(ECHO) "Combining the Bootloader with the EBIU pre-initialization"
#	$(LOADER) $(BOOTLOADER_DXE) -b flash -o boot_core.ldr $(LOADERFLAGS)
#	$(LOADER) $(BOOTLOADER_DXE) -romsplitter -maskaddr 29 -o split.ldr $(LOADERFLAGS)
#	$(HEX2SREC) split.ldr split.s -nt
#	$(HEX2SREC) boot_core.ldr boot_core.s -nt
#	$(CAT) boot_core.s split.s > boot_loader.s
	@$(ECHO) "Building application"
	$(LOADER) $(DXE_FILE) -b flash -o $(LDR_FILE) $(LOADERFLAGS)
	$(HEX2SREC) $(LDR_FILE) app_relo.s -nt
	$(SRECRELO) app_relo.s app.s 0x00000 0x5FFFB 0x00000 0xFF
	@$(ECHO) "Applying application Magic Number at address "$(SREC_MAGIC_NUMBER_ADDRESS)
	$(VECT2SREC) ./bin/app_magic_number.ldr app_magic_number.s -o $(SREC_MAGIC_NUMBER_ADDRESS) -nt
	$(CAT) app.s app_magic_number.s  > app_cat.s
#	@$(ECHO) "Generating Vehicle EQ ldr files from directory: $(EQF_DIR)"

#	$(CAT) boot_loader.s app_cat_magic_num.s > app_cat.s
	@$(ECHO) "Build Cleanup"
	$(SRECRELO) app_cat.s $(S_RECORD) 0x00000 0x7FFFF 0x00000 0xFF 
	$(RM) app.s app_relo.s app_cat.s app_magic_number.s
	@$(ECHO) "Build Successful - output file: $(S_RECORD)"
	

#
# This is the rule for recursion
#
.PHONY: subdirs $(RECURSE_DIRS)
subdirs: $(RECURSE_DIRS)
$(RECURSE_DIRS) :
	@$(ECHO) "Generating recursive program code for $@"
	$(MAKE) -C $@ all

#
# This is the rule for running lint
#
lint: $(LINT_FILES)
	perl ./bin/lint_stat.pl ./ $(LINT_FILES)

lint_clean :
	@$(ECHO) Removing Lint Files
	-$(RM) -f $(LINT_FILES)
	-$(RM) -f lint_stat.eo

%.le : %.c 
	@$(ECHO) Generating LINT output for file $@
	$(BIN_LINT) $(LINT_FLAGS) $< > $@

#
# This is the rule for recursively cleaning out the cruft
#
.PHONY: clean
clean :
	for dir in $(RECURSE_DIRS); do (cd $$dir && $(MAKE) clean); done
	-$(RM) -f $(C_OBJECTS) $(ASM_OBJECTS)
	-$(RM) -f $(PROJECT_NAME)_$(PROJECT_TYPE_PCM).dxe $(PROJECT_NAME)_$(PROJECT_TYPE_DECODER).dxe 
	-$(RM) -f $(PROJECT_NAME)_$(PROJECT_TYPE_PCM).map.xml $(PROJECT_NAME)_$(PROJECT_TYPE_DECODER).map.xml 
	-$(RM) -f $(PROJECT_NAME)_$(PROJECT_TYPE_PCM).ldr $(PROJECT_NAME)_$(PROJECT_TYPE_DECODER).ldr 
	-$(RM) -f $(PROJECT_NAME)_$(PROJECT_TYPE_PCM).s $(PROJECT_NAME)_$(PROJECT_TYPE_DECODER).s 
	-$(RM) -f *.xml
	-$(RM) -f makefile.dep
	-$(RM) -f *.bak
	-$(RM) -f *~
	-$(RM) -f *.ldr

#
# This is a rule for manually generating dependencies
#
.PHONY: depend
depend: makefile.dep

#
# This is the rule for making dependencies
#

makefile.dep : $(C_FILES) $(ASM_FILES) $(HEADER_FILES)
	$(TOUCH) makefile.dep
	$(MAKEDEPEND) -f makefile.dep -Y -o.doj -- $(CFLAGS) $(INCLUDE_DIRS) -- $(C_FILES) $(ASM_FILES) > /dev/null 2>&1

#
# Add each target above here so that dependencies will be magically generated
# by the makefile.dep rule above and then included
#

#
ifeq ($(MAKECMDGOALS),dm_decoder)
-include makefile.dep
endif
