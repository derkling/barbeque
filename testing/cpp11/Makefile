

all: cpp11_test_arm

# The BBQ provided header files for CPP11 features
BBQUE_HEADERS=../../../out/include

# The Android ARM cross-compiler to use
ARM_CC=arm-linux-androideabi-

# By default optimize code-size
#OPT=-g -O0
OPT=-Os

GC=-ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS=-DUNIX -std=c++0x -std=gnu++0x -I$(BBQUE_HEADERS) -Wall $(OPT) -DNDEBUG $(GC)
AFLAGS=\
	-D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ \
	-Wno-psabi -march=armv5te -mtune=xscale -msoft-float \
	-fno-exceptions -fno-rtti -mthumb \
	-Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 \
	-DANDROID  -Wa,--noexecstack -fexceptions -frtti  -O2 -DNDEBUG -g 

PHONY: all cpp11_test_arm cpp11_test_x86 androideabi clean

# ARM Version, targeting Android devices
cpp11_test_arm: cpp11_test.cc
	@echo "===== Building CPP11 features test"
	@mkdir objs  || true
	$(ARM_CC)g++ -o objs/cpp11_test_arm.o $(CFLAGS) $(AFLAGS) -c cpp11_test.cc
	$(ARM_CC)g++ -o objs/gnu_chrono.o $(CFLAGS) $(AFLAGS) -c cpp11/gnu_chrono.cc
	$(ARM_CC)g++ -o objs/gnu_mutex.o $(CFLAGS) $(AFLAGS) -c cpp11/gnu_mutex.cc
	$(ARM_CC)g++ -o objs/gnu_condition_variable.o $(CFLAGS) $(AFLAGS) -c cpp11/gnu_condition_variable.cc
	$(ARM_CC)g++ -o objs/gnu_thread.o $(CFLAGS) $(AFLAGS) -c cpp11/gnu_thread.cc
	$(ARM_CC)g++ -o objs/gnu_future.o $(CFLAGS) $(AFLAGS) -c cpp11/gnu_future.cc
	$(ARM_CC)g++ -o cpp11_test_arm $(CFLAGS) $(AFLAGS) \
		objs/cpp11_test_arm.o \
		objs/gnu_chrono.o objs/gnu_mutex.o objs/gnu_condition_variable.o \
		objs/gnu_thread.o objs/gnu_future.o

# X86 Version, targeting local host development and testing
cpp11_test_x86: cpp11_test.cc
	@echo "===== Building x86 version"
	g++ -o objs/cpp11_test_x86.o $(CFLAGS) -c cpp11_test.cc
	g++ -o cpp11_test_x86   $(CFLAGS) objs/cpp11_test_x86.o -lpthread

# Emulator installation, via ADB push
push: cpp11_test_arm
	adb shell mkdir /data/bbque
	adb push cpp11_test_arm /data/bbque/

# Emulator testing, via ADB bush
test: push
	adb shell /data/bbque/cpp11_test_arm

# Cleanuing-up
clean:
	rm -rf *.o cpp11_test_* objs

