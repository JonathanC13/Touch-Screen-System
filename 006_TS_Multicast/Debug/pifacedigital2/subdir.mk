################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pifacedigital2/mcp23s17.c \
../pifacedigital2/pifacedigital.c 

CPP_SRCS += \
../pifacedigital2/logger.cpp 

OBJS += \
./pifacedigital2/logger.o \
./pifacedigital2/mcp23s17.o \
./pifacedigital2/pifacedigital.o 

C_DEPS += \
./pifacedigital2/mcp23s17.d \
./pifacedigital2/pifacedigital.d 

CPP_DEPS += \
./pifacedigital2/logger.d 


# Each subdirectory must supply rules for building sources it contributes
pifacedigital2/%.o: ../pifacedigital2/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

pifacedigital2/%.o: ../pifacedigital2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


