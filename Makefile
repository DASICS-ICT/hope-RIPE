# Makefile for RIPE
# @author John Wilander & Nick Nikiforakis
# Modified for RISC-V by John Merrill

#Depending on how you test your system you may want to comment, or uncomment
#the following
CFLAGS=  -fno-stack-protector -z execstack -O0
CC=riscv64-unknown-elf-gcc

SRCS = ./source/ripe_attack_generator.c \
	   	./dasics/lib/udasics.c \
	    ./dasics/lib/dasics_entry.S		

INC_DIR += ./dasics/include/

INCLUDES  = $(addprefix -I, ${INC_DIR})

all: ripe_attack_generator
	ln -sf $(abspath .)/build/ripe_attack_generator $(RISCV_ROOTFS_HOME)/rootfsimg/build/ripe_attack_generator

clean:
	rm -rf build/ out/

ripe_attack_generator: ./source/ripe_attack_generator.c
	mkdir -p build/ out/
	$(CC)  $(CFLAGS) $(INCLUDES) \
		-T linkdasics.ld $(SRCS) -o ./build/ripe_attack_generator
