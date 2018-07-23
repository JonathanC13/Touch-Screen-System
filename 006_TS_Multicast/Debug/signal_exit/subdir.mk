################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../signal_exit/SignalHandler.cpp 

OBJS += \
./signal_exit/SignalHandler.o 

CPP_DEPS += \
./signal_exit/SignalHandler.d 


# Each subdirectory must supply rules for building sources it contributes
signal_exit/%.o: ../signal_exit/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


