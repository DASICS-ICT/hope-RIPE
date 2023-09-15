# Makefile for RIPE
# @author John Wilander & Nick Nikiforakis
# Modified for RISC-V by John Merrill

#Depending on how you test your system you may want to comment, or uncomment
#the following
CFLAGS 			= -fno-stack-protector -z execstack -O0 -g -DDASICS_CONFIG
CC_PREFIX_LINUX = riscv64-unknown-linux-gnu
CC 				= $(CC_PREFIX_LINUX)-gcc
OBJDUMP			= $(CC_PREFIX_LINUX)-objdump

CCVERSION 		= $(shell $(CC_PREFIX_LINUX)-gcc --version | grep ^$(CC_PREFIX_LINUX)-gcc | sed 's/^.* //g')


DIR_PWD    		= $(abspath .)
# dasics-dll-lib path
DIR_DASICS		 = $(DIR_PWD)/dasics-dynamic-lib
DIR_DASICS_BUILD = $(DIR_PWD)/dasics-dynamic-lib/build
DIR_SRC    		 = source
DIR_OUT    		 = out
DIR_BUILD  		 = build

SRCS 			 = $(DIR_SRC)/ripe_attack_generator.c

# The dasics include path
DASICS_INCLUDE 	= -I$(DIR_DASICS)/include -I$(DIR_DASICS)/dasics_dll/include -I$(DIR_DASICS)/memory/include -I$(DIR_DASICS)/ecall/include


all: ripe_attack_generator
# ln -sf $(abspath .)/$(DIR_BUILD)/ripe_attack_generator $(RISCV_ROOTFS_HOME)/rootfsimg/build/ripe_attack_generator
# ln -sf $(abspath .)/ripe_tester.sh $(RISCV_ROOTFS_HOME)/rootfsimg/build/ripe_tester.sh

$(DIR_DASICS_BUILD):
ifeq ($(wildcard $(DIR_DASICS)/*),)
	git submodule update --init $(DIR_DASICS)
endif
	make -C $(DIR_DASICS)

clean:
	rm -rf $(DIR_BUILD) $(DIR_OUT)
	make -C $(DIR_DASICS) clean

ripe_attack_generator: $(DIR_SRC)/ripe_attack_generator.c $(DIR_DASICS_BUILD)
	make -C $(DIR_DASICS)
	mkdir -p $(DIR_BUILD) $(DIR_OUT)
	$(CC) $(CFLAGS) $(DASICS_INCLUDE) \
		 $(SRCS) -o $(DIR_BUILD)/ripe_attack_generator $(DIR_DASICS)/build/dasics_lib.a -T$(DIR_DASICS)/ld.lds -e _set_utvec
	$(OBJDUMP) -d $(DIR_BUILD)/ripe_attack_generator > $(DIR_BUILD)/ripe_attack_generator.txt