################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CPUsManager.c \
../ConsoleHandler.c \
../bibliotecaSAFA.c \
../main.c 

OBJS += \
./CPUsManager.o \
./ConsoleHandler.o \
./bibliotecaSAFA.o \
./main.o 

C_DEPS += \
./CPUsManager.d \
./ConsoleHandler.d \
./bibliotecaSAFA.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documentos/tp-2018-2c-nuGetChar/kemmens" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


