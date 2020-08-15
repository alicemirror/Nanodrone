# Nanodrone project makefile
# Version 1.0
# Compiles testlens and firstfly

all: testlens firstfly

# Added the -li2c linker flag to avoid compilation errors on the I2C protocol 
CCFLAGS = -std=c++0x -li2c

# OpenCV
CVFLAGS = `pkg-config --cflags opencv`
CVLIBS = `pkg-config --libs opencv`

# INCLUDE_CV = -I /usr/include -I /usr/include/opencv
OBJECTS = ArduCAM.o arducam_arch_raspberrypi.o \
			imageprocessor.o processormath.o \
			serialgps.o

# Build firsfly
firstfly : $(OBJECTS) firstfly.o 
	g++ $(CCFLAGS) -o firstfly $(OBJECTS) \
	firstfly.o -lwiringPi -Wall $(CVLIBS)

# Build testlens
testlens : $(OBJECTS) testlens.o 
	g++ $(CCFLAGS) -o testlens $(OBJECTS) \
	testlens.o -lwiringPi -Wall $(CVLIBS)
	
# No needed OpenCV flags (Arducam library)
ArduCAM.o : ArduCAM.cpp 
	g++ $(CCFLAGS) -c ArduCAM.cpp

# No needed OpenCV flags (Arducam library)
arducam_arch_raspberrypi.o : arducam_arch_raspberrypi.c 
	g++ $(CCFLAGS) -c arducam_arch_raspberrypi.c 
	
# Includes OpenCV flags
firstfly.o : firstfly.cpp
	g++ $(CCFLAGS) $(CVFLAGS) -c firstfly.cpp 

# OpenCV based image processor
imageprocessor.o : imageprocessor.cpp processormath.cpp
	g++ $(CCFLAGS) $(CVFLAGS) -c imageprocessor.cpp processormath.cpp
	
# Serial GPS manager
serialgps.o : serialgps.cpp
	g++ $(CCFLAGS) -c serialgps.cpp
 	
clean : 
	rm -f  testlens firstfly $(objects) *.o
