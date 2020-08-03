# Removed the unsopported modules but the ov5642camera
all:  ov5642_capture
# Added the -li2c linker flag to avoid compilation errors on the I2C protocol (Raspbian Buster version)
CCFLAGS = -std=c++0x -li2c
VPATH= ../../../ArduCAM
INCLUDE1 = -I../../../ArduCAM -I./
INCLUDE2 =-I./ -I../../../ArduCAM
objects = ArduCAM.o arducam_arch_raspberrypi.o

# Excluded the creation of the test programs not intented for this project
ov5642_capture : $(objects) arducam_ov5642_capture.o 
	g++ $(CCFLAGS) -o ov5642_capture $(objects) arducam_ov5642_capture.o -lwiringPi -Wall
	
ArduCAM.o : ArduCAM.cpp 
	g++ $(CCFLAGS) $(INCLUDE1) -c $(VPATH)/ArduCAM.cpp
arducam_arch_raspberrypi.o : arducam_arch_raspberrypi.c 
	g++ $(CCFLAGS) $(INCLUDE2) -c arducam_arch_raspberrypi.c 
	
arducam_ov5642_capture.o : arducam_ov5642_capture.cpp
	g++ $(CCFLAGS) $(INCLUDE2) -c arducam_ov5642_capture.cpp 
 	
clean : 
	rm -f  ov5642_capture $(objects) *.o
