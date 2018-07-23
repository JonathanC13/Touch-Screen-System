################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../locator/006_TS_Multicast.cpp \
../locator/emit.cpp \
../locator/multicast_send.cpp \
../locator/pfd2_test.cpp \
../locator/sensor.cpp 

OBJS += \
./locator/006_TS_Multicast.o \
./locator/emit.o \
./locator/multicast_send.o \
./locator/pfd2_test.o \
./locator/sensor.o 

CPP_DEPS += \
./locator/006_TS_Multicast.d \
./locator/emit.d \
./locator/multicast_send.d \
./locator/pfd2_test.d \
./locator/sensor.d 


# Each subdirectory must supply rules for building sources it contributes
locator/%.o: ../locator/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


