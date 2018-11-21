################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CPUsManager.c \
../Communication.c \
../ConsoleHandler.c \
../ResourceManager.c \
../Scheduling.c \
../Settings.c \
../main.c 

OBJS += \
./CPUsManager.o \
./Communication.o \
./ConsoleHandler.o \
./ResourceManager.o \
./Scheduling.o \
./Settings.o \
./main.o 

C_DEPS += \
./CPUsManager.d \
./Communication.d \
./ConsoleHandler.d \
./ResourceManager.d \
./Scheduling.d \
./Settings.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documentos/tp-2018-2c-nuGetChar/kemmens" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


