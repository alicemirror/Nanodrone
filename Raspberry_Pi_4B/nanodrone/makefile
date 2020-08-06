# Nanodrone project makefile

all: testlens

# Added the -li2c linker flag to avoid compilation errors on the I2C protocol 
CCFLAGS = -std=c++0x -li2c

# OpenCV
# CVFLAGS = `pkg-config --cflags opencv`
# CVLIBS = `pkg-config --libs opencv`

# INCLUDE_CV = -I /usr/include -I /usr/include/opencv
OBJECTS = ArduCAM.o arducam_arch_raspberrypi.o

# Build testlens
testlens : $(OBJECTS) testlens.o 
	g++ $(CCFLAGS) -o testlens $(OBJECTS) \
	testlens.o -lwiringPi -Wall
	
# No needed OpenCV flags (Arducam library)
ArduCAM.o : ArduCAM.cpp 
	g++ $(CCFLAGS) -c ArduCAM.cpp

# No needed OpenCV flags (Arducam library)
arducam_arch_raspberrypi.o : arducam_arch_raspberrypi.c 
	g++ $(CCFLAGS) -c arducam_arch_raspberrypi.c 
	
# Includes OpenCV flags
testlens.o : testlens.cpp
	g++ $(CCFLAGS) -c testlens.cpp 
 	
clean : 
	rm -f  testlens $(objects) *.o
